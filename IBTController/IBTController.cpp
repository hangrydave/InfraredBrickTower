// example imgui code from here: https://github.com/ocornut/imgui/tree/master/examples/example_win32_directx12

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <thread>

#include "imfilebrowser.h"

#include "LASM.h"
#include "TowerController.h"
#include "WinUsbTowerInterface.h"
#include "VLL.h"
#include "PBrick.h"

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue;
};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = NULL;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
static ID3D12Fence* g_fence = NULL;
static HANDLE                       g_fenceEvent = NULL;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = NULL;
static HANDLE                       g_hSwapChainWaitableObject = NULL;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool programIsDone = false;

static ImGuiViewport* mainViewport;
static ImGui::FileBrowser fileDialog;

static unsigned long towerLengthWritten = 0;

static LASM::CommandData lasmCommand;
struct RCXRemoteData
{
    bool motorAFwd,
        motorBFwd,
        motorCFwd,
        motorABwd,
        motorBBwd,
        motorCBwd = false;

    int message1,
        message2,
        message3 = 0;

    int program1,
        program2,
        program3,
        program4,
        program5 = 0;

    int stop = 0;
    int sound = 0;

    std::string* downloadFilePath;

    WORD request = 0;
} static rcxRemoteData;

struct VLLData
{
    int beep1Immediate, 
        beep2Immediate, 
        beep3Immediate, 
        beep4Immediate, 
        beep5Immediate = 0;
    BYTE beep1ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_1_IMMEDIATE };
    BYTE beep2ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_2_IMMEDIATE };
    BYTE beep3ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_3_IMMEDIATE };
    BYTE beep4ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_4_IMMEDIATE };
    BYTE beep5ImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_5_IMMEDIATE };

    bool forwardImmediate = false;
    bool backwardImmediate = false;
    BYTE fwdImmediateBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_IMMEDIATE };
    BYTE bwdImmediateBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_IMMEDIATE };

    int beep1Program,
        beep2Program,
        beep3Program,
        beep4Program,
        beep5Program = 0;
    BYTE beep1ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_1_PROGRAM };
    BYTE beep2ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_2_PROGRAM };
    BYTE beep3ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_3_PROGRAM };
    BYTE beep4ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_4_PROGRAM };
    BYTE beep5ProgramBytes[VLL_PACKET_LENGTH]{ VLL_BEEP_5_PROGRAM };

    int forwardHalf,
        forwardOne,
        forwardTwo,
        forwardFive = 0;
    BYTE forwardHalfBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_HALF };
    BYTE forwardOneByte[VLL_PACKET_LENGTH]{ VLL_FORWARD_ONE };
    BYTE forwardTwoBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_TWO };
    BYTE forwardFiveBytes[VLL_PACKET_LENGTH]{ VLL_FORWARD_FIVE };

    int backwardHalf,
        backwardOne,
        backwardTwo,
        backwardFive = 0;
    BYTE backwardHalfBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_HALF };
    BYTE backwardOneByte[VLL_PACKET_LENGTH]{ VLL_BACKWARD_ONE };
    BYTE backwardTwoBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_TWO };
    BYTE backwardFiveBytes[VLL_PACKET_LENGTH]{ VLL_BACKWARD_FIVE };

    int waitLight,
        seekLight,
        code,
        keepAlive = 0;
    BYTE waitLightBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
    BYTE seekLightBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
    BYTE codeBytes[VLL_PACKET_LENGTH]{ VLL_STOP };
    BYTE keepAliveBytes[VLL_PACKET_LENGTH]{ VLL_STOP };

    int run = 0;
    BYTE runBytes[VLL_PACKET_LENGTH]{ VLL_RUN };

    int stop = 0;
    BYTE stopBytes[VLL_PACKET_LENGTH]{ VLL_STOP };

    int deleteProgram = 0;
    BYTE deleteBytes[VLL_PACKET_LENGTH]{ VLL_DELETE_PROGRAM };
} static vllData;

void SendVLL(BYTE* data, Tower::RequestData* towerData)
{
    // todo: store current comm mode
    Tower::SetCommMode(Tower::CommMode::VLL, towerData);
    Tower::WriteData(data, VLL_PACKET_LENGTH, towerLengthWritten, towerData);
}

void RunTowerThread()
{
    WinUsbTowerInterface* usbTowerInterface;
    bool gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
    if (!gotInterface)
    {
        printf("Error getting WinUSB interface!\n");
        programIsDone = true;
        return;
    }

    Tower::RequestData* towerData = new Tower::RequestData(usbTowerInterface);
    while (!programIsDone)
    {
        // RCX
        {
            // remote
            rcxRemoteData.request = 0;

            if (rcxRemoteData.message1-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PB_MESSAGE_1;
            if (rcxRemoteData.message2-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PB_MESSAGE_2;
            if (rcxRemoteData.message3-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PB_MESSAGE_3;

            if (rcxRemoteData.motorAFwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_A_FORWARDS;
            if (rcxRemoteData.motorABwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_A_BACKWARDS;

            if (rcxRemoteData.motorBFwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_B_FORWARDS;
            if (rcxRemoteData.motorBBwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_B_BACKWARDS;

            if (rcxRemoteData.motorCFwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_C_FORWARDS;
            if (rcxRemoteData.motorCBwd)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::MOTOR_C_BACKWARDS;

            if (rcxRemoteData.program1-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_1;
            if (rcxRemoteData.program2-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_2;
            if (rcxRemoteData.program3-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_3;
            if (rcxRemoteData.program4-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_4;
            if (rcxRemoteData.program5-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::PROGRAM_5;

            if (rcxRemoteData.stop-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::STOP_PROGRAM_AND_MOTORS;
            if (rcxRemoteData.sound-- > 0)
                rcxRemoteData.request |= (WORD)LASM::RemoteCommandRequest::REMOTE_SOUND;

            Tower::SetCommMode(Tower::CommMode::IR, towerData);
            if (rcxRemoteData.request != 0)
            {
                LASM::Cmd_RemoteCommand(rcxRemoteData.request, lasmCommand);
                LASM::SendCommand(&lasmCommand, towerData, 0);
            }

            if (rcxRemoteData.downloadFilePath != nullptr)
            {
                RCX::DownloadProgram(rcxRemoteData.downloadFilePath->c_str(), 0, towerData);
                rcxRemoteData.downloadFilePath = nullptr;
            }
        }

        // VLL
        {
            // immediate
            // motor
            if (vllData.forwardImmediate)
                SendVLL(vllData.fwdImmediateBytes, towerData);
            if (vllData.backwardImmediate)
                SendVLL(vllData.bwdImmediateBytes, towerData);

            // sound
            if (vllData.beep1Immediate-- > 0)
                SendVLL(vllData.beep1ImmediateBytes, towerData);
            if (vllData.beep2Immediate-- > 0)
                SendVLL(vllData.beep2ImmediateBytes, towerData);
            if (vllData.beep3Immediate-- > 0)
                SendVLL(vllData.beep3ImmediateBytes, towerData);
            if (vllData.beep4Immediate-- > 0)
                SendVLL(vllData.beep4ImmediateBytes, towerData);
            if (vllData.beep5Immediate-- > 0)
                SendVLL(vllData.beep5ImmediateBytes, towerData);

            // program
            // sound
            if (vllData.beep1Program-- > 0)
                SendVLL(vllData.beep1ProgramBytes, towerData);
            if (vllData.beep2Program-- > 0)
                SendVLL(vllData.beep2ProgramBytes, towerData);
            if (vllData.beep3Program-- > 0)
                SendVLL(vllData.beep3ProgramBytes, towerData);
            if (vllData.beep4Program-- > 0)
                SendVLL(vllData.beep4ProgramBytes, towerData);
            if (vllData.beep5Program-- > 0)
                SendVLL(vllData.beep5ProgramBytes, towerData);

            // motor
            if (vllData.forwardHalf-- > 0)
                SendVLL(vllData.forwardHalfBytes, towerData);
            if (vllData.forwardOne-- > 0)
                SendVLL(vllData.forwardOneByte, towerData);
            if (vllData.forwardTwo-- > 0)
                SendVLL(vllData.forwardTwoBytes, towerData);
            if (vllData.forwardFive-- > 0)
                SendVLL(vllData.forwardFiveBytes, towerData);
            if (vllData.backwardHalf-- > 0)
                SendVLL(vllData.backwardHalfBytes, towerData);
            if (vllData.backwardOne-- > 0)
                SendVLL(vllData.backwardOneByte, towerData);
            if (vllData.backwardTwo-- > 0)
                SendVLL(vllData.backwardTwoBytes, towerData);
            if (vllData.backwardFive-- > 0)
                SendVLL(vllData.backwardFiveBytes, towerData);

            // preset programs
            if (vllData.waitLight-- > 0)
                SendVLL(vllData.waitLightBytes, towerData);
            if (vllData.seekLight-- > 0)
                SendVLL(vllData.seekLightBytes, towerData);
            if (vllData.code-- > 0)
                SendVLL(vllData.codeBytes, towerData);
            if (vllData.keepAlive-- > 0)
                SendVLL(vllData.keepAliveBytes, towerData);

            // etc functionality
            if (vllData.run-- > 0)
                SendVLL(vllData.runBytes, towerData);
            if (vllData.stop-- > 0)
                SendVLL(vllData.stopBytes, towerData);
            if (vllData.deleteProgram-- > 0)
                SendVLL(vllData.deleteBytes, towerData);
        }
    }

    delete towerData;
    delete usbTowerInterface;
}

void BuildMicroScoutRemote()
{
    ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 50, mainViewport->WorkPos.y + 150), ImGuiCond_FirstUseEver);

    // VLL window
    ImGui::Begin("MicroScout Remote");

#define VLL_IMMEDIATE_MODE 0
#define VLL_PROGRAM_MODE 1
    static int commandMode = 0;
    ImGui::Text("Command mode");
    ImGui::RadioButton("immediate", &commandMode, VLL_IMMEDIATE_MODE); ImGui::SameLine();
    ImGui::RadioButton("program", &commandMode, VLL_PROGRAM_MODE);

    if (commandMode == VLL_IMMEDIATE_MODE)
    {
        // sounds
        ImGui::Separator();
        if (ImGui::Button("1"))
            vllData.beep1Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("2"))
            vllData.beep2Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("3"))
            vllData.beep3Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("4"))
            vllData.beep4Immediate = 1;
        ImGui::SameLine();
        if (ImGui::Button("5"))
            vllData.beep5Immediate = 1;

        ImGui::SameLine();
        ImGui::Text("Sound");

        // motors
        ImGui::Separator();
        ImGui::Checkbox("motor forwards", &vllData.forwardImmediate);
        if (vllData.forwardImmediate)
            vllData.backwardImmediate = false;

        ImGui::SameLine();
        ImGui::Checkbox("motor backwards", &vllData.backwardImmediate);
        if (vllData.backwardImmediate)
            vllData.forwardImmediate = false;
    }
    else
    {
        // sounds
        ImGui::Separator();
        ImGui::Text("Sound");
        if (ImGui::Button("1"))
            vllData.beep1Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("2"))
            vllData.beep2Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("3"))
            vllData.beep3Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("4"))
            vllData.beep4Program = 1;
        ImGui::SameLine();
        if (ImGui::Button("5"))
            vllData.beep5Program = 1;

        // motors
        // forward
        ImGui::Separator();
        ImGui::Text("Motor");
        if (ImGui::Button("fwd 0.5 sec"))
            vllData.forwardHalf = 1;
        ImGui::SameLine();
        if (ImGui::Button("fwd 1 sec"))
            vllData.forwardOne = 1;
        ImGui::SameLine();
        if (ImGui::Button("fwd 2 sec"))
            vllData.forwardTwo = 1;
        ImGui::SameLine();
        if (ImGui::Button("fwd 5 sec"))
            vllData.forwardFive = 1;

        // backward
        if (ImGui::Button("bwd 0.5 sec"))
            vllData.backwardHalf = 1;
        ImGui::SameLine();
        if (ImGui::Button("bwd 1 sec"))
            vllData.backwardOne = 1;
        ImGui::SameLine();
        if (ImGui::Button("bwd 2 sec"))
            vllData.backwardTwo = 1;
        ImGui::SameLine();
        if (ImGui::Button("bwd 5 sec"))
            vllData.backwardFive = 1;

        // program presets
        // wait seek code keep
        ImGui::Separator();
        ImGui::Text("Program presets");
        if (ImGui::Button("wait light"))
            vllData.waitLight = 1;
        ImGui::SameLine();
        if (ImGui::Button("seek light"))
            vllData.seekLight = 1;
        ImGui::SameLine();
        if (ImGui::Button("code"))
            vllData.code = 1;
        ImGui::SameLine();
        if (ImGui::Button("keep alive"))
            vllData.keepAlive = 1;

        // run/delete/stop
        ImGui::Separator();
        ImGui::Text("Management");
        if (ImGui::Button("run"))
            vllData.run = 1;
        ImGui::SameLine();
        if (ImGui::Button("stop"))
            vllData.stop = 1;
        ImGui::SameLine();
        if (ImGui::Button("delete program"))
            vllData.deleteProgram = 1;
    }

    ImGui::End();
}

void BuildRCXRemote()
{
    ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x + 50, mainViewport->WorkPos.y + 450), ImGuiCond_FirstUseEver);

    // RCX remote
    ImGui::Begin("RCX Remote");

    // messages
    ImGui::Text("Message");
    if (ImGui::Button("Msg 1"))
        rcxRemoteData.message1 = true;
    ImGui::SameLine();
    if (ImGui::Button("Msg 2"))
        rcxRemoteData.message2 = true;
    ImGui::SameLine();
    if (ImGui::Button("Msg 3"))
        rcxRemoteData.message3 = true;

    // motors
    ImGui::Separator();
    ImGui::Text("Motors");

    if (ImGui::BeginTable("motorControlTable", 3))
    {
        ImVec2 buttonSize;
        buttonSize.x = -1;
        buttonSize.y = 0;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Checkbox("A Fwd", &rcxRemoteData.motorAFwd);
        if (rcxRemoteData.motorAFwd) rcxRemoteData.motorABwd = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("B Fwd", &rcxRemoteData.motorBFwd);
        if (rcxRemoteData.motorBFwd) rcxRemoteData.motorBBwd = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::Checkbox("C Fwd", &rcxRemoteData.motorCFwd);
        if (rcxRemoteData.motorCFwd) rcxRemoteData.motorCBwd = false;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Checkbox("A Bwd", &rcxRemoteData.motorABwd);
        if (rcxRemoteData.motorABwd) rcxRemoteData.motorAFwd = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("B Bwd", &rcxRemoteData.motorBBwd);
        if (rcxRemoteData.motorBBwd) rcxRemoteData.motorBFwd = false;
        ImGui::TableSetColumnIndex(2);
        ImGui::Checkbox("C Bwd", &rcxRemoteData.motorCBwd);
        if (rcxRemoteData.motorCBwd) rcxRemoteData.motorCFwd = false;
    }
    ImGui::EndTable();

    // programs
    ImGui::Separator();
    ImGui::Text("Program");
    if (ImGui::Button("Prgm 1"))
        rcxRemoteData.program1 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 2"))
        rcxRemoteData.program2 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 3"))
        rcxRemoteData.program3 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 4"))
        rcxRemoteData.program4 = 1;
    ImGui::SameLine();
    if (ImGui::Button("Prgm 5"))
        rcxRemoteData.program5 = 1;

    ImGui::Separator();
    if (ImGui::Button("Stop"))
        rcxRemoteData.stop = 1;
    ImGui::SameLine();
    if (ImGui::Button("Sound"))
        rcxRemoteData.sound = 1;

    ImGui::Separator();
    if (ImGui::Button("Download Program"))
        fileDialog.Open();

    ImGui::End();

    fileDialog.Display();
    if (fileDialog.HasSelected())
    {
        rcxRemoteData.downloadFilePath = new std::string(fileDialog.GetSelected().string());
        fileDialog.ClearSelected();
    }
}

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(
        wc.lpszClassName, 
        _T("IBT"), 
        WS_OVERLAPPEDWINDOW, 
        100, 100, 
        640, 800, 
        NULL, 
        NULL, 
        wc.hInstance, 
        NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Run the tower transmission things on a separate thread
    std::thread towerThread(RunTowerThread);

    mainViewport = ImGui::GetMainViewport();

    fileDialog.SetTitle("Select an RCX program");
    fileDialog.SetTypeFilters({ ".rcx" });

    // Main loop
    while (!programIsDone)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                programIsDone = true;
        }
        if (programIsDone)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        BuildMicroScoutRemote();
        BuildRCXRemote();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        ImGui::Render();

        FrameContext* frameCtx = WaitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtx->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pd3dCommandList->Reset(frameCtx->CommandAllocator, NULL);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, NULL);
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_pd3dCommandQueue->Signal(g_fence, fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frameCtx->FenceValue = fenceValue;
    }

    WaitForLastSubmittedFrame();

    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    towerThread.join();
    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = NULL;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != NULL)
    {
        ID3D12InfoQueue* pInfoQueue = NULL;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
        g_pd3dCommandList->Close() != S_OK)
        return false;

    if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_fenceEvent == NULL)
        return false;

    {
        IDXGIFactory4* dxgiFactory = NULL;
        IDXGISwapChain1* swapChain1 = NULL;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, NULL); g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_hSwapChainWaitableObject != NULL) { CloseHandle(g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = NULL; }
    if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = NULL; }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = NULL; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = NULL; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = NULL; }
    if (g_fence) { g_fence->Release(); g_fence = NULL; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = NULL;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = NULL;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = NULL; }
}

void WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources()
{
    UINT nextFrameIndex = g_frameIndex + 1;
    g_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, NULL };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->FenceValue = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        waitableObjects[1] = g_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(result) && "Failed to resize swapchain.");
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}