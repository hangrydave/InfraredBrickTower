#include <LASM.h>
#include <TowerController.h>
#include <VLL.h>
#include <PBrick.h>

#if defined(WIN64)
#include <WinUsbTowerInterface.h>
#elif defined(__linux)
#include <LinuxUSBTowerInterface.h>
#endif

int main()
{
    HostTowerCommInterface* usbTowerInterface;

    bool gotInterface = false;
#if defined(WIN64)
    gotInterface = OpenWinUsbTowerInterface(usbTowerInterface);
#elif defined(__linux)
    gotInterface = OpenLinuxUSBTowerInterface(usbTowerInterface);
#endif

    if (!gotInterface)
    {
        printf("Error getting USB interface!\n");
        return 1;
    }

    auto towerData = new Tower::RequestData(usbTowerInterface);
    Tower::SetCommMode(Tower::CommMode::IR, towerData);
}