from PyPDF2 import PdfFileReader

reader = PdfFileReader("lasm.pdf")
number_of_pages = len(reader.pages)

for i in range(11, 97):
    if i == 70: # this page just elaborates on the command on the previous page, ignore
        continue

    page = reader.pages[i]
    text = page.extract_text()
    text = text.replace(" ", "") # don't need spaces, they only hurt me

    # get command name
    text = text.split("DonotcopyPage", 1)[1]
    paren_index = text.find("(")
    cmd_name = text[0 : paren_index].strip()
    cmd_name_2 = None
    comma_index = cmd_name.find(",")
    if comma_index != -1:
        parts = cmd_name.split(",")
        cmd_name = parts[0].strip()
        cmd_name_2 = parts[1].strip()

    text = text[paren_index + 1 : ]

    # get command byte
    paren_index = text.find(")")
    cmd_byte = text[0 : paren_index].strip()
    text = text[paren_index + 1 : ]

    # get reply
    reply = "NO_REPLY"
    if text.find("Reply") != -1 and text.find("Thereisnoreplytothiscommand") == -1:
        text = text.replace("Replystructure:", "Reply:").split("Reply:")[1]
        zero_index = text.find("0")
        text = text[zero_index - 1 : ].strip()
        parts = text.split("\n", 1)
        reply = parts[0][0 : 4]
    
    parts = text.split("LASMsyntax:")
    availability_text = parts[0].split("Availability:")[1]
    text = parts[1]
    availability = "BOTH"
    if availability_text == "Direct":
        availability = "DIRECT"
    elif availability_text == "Program":
        availability = "PROGRAM"

    print("Cmd(" + cmd_name + ", " + cmd_byte + ", " + reply + ", " + availability + ");")
    if cmd_name_2 != None:
        print("Cmd(" + cmd_name_2 + ", " + cmd_byte + ", " + reply + ", " + availability + ");")
