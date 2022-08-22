from random import shuffle
from PyPDF2 import PdfFileReader

reader = PdfFileReader("lasm.pdf")
number_of_pages = len(reader.pages)

cmd_byte_enum = "enum LASMCommandByte : BYTE\n{\n"
cmd_macro_calls = ""

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
        # cmd_name_2 = parts[1].strip()

    text = text[paren_index + 1 : ]

    # get command byte
    paren_index = text.find(")")
    cmd_byte = text[0 : paren_index].strip()
    text = text[paren_index + 1 : ]

    cmd_byte_enum = cmd_byte_enum + "\t" + cmd_name + " = " + cmd_byte + ",\n"
    # if cmd_name_2 != None:
    #     cmd_byte_enum = cmd_byte_enum + "\t" + cmd_name_2 + " = " + cmd_byte + ",\n"

    # see if it has parameters
    text = text.split(cmd_byte)[1]

    reply_index = text.find("Reply")
    availability_index = text.find("Availability")

    text_between = ""
    if reply_index != -1:
        text_between = text[0 : reply_index]
    else:
        text_between = text[0 : availability_index]
    
    text_between = text_between.strip()
    
    params = []

    prev_char = ' '
    line_started_with_num = False
    start_index = 0
    current_index = 0

    for c in text_between:
        if prev_char == '\n':
            line_started_with_num = c.isnumeric()

        should_add_param = False

        is_at_end = current_index == len(text_between) - 1
        should_add_param = should_add_param or is_at_end
        should_add_param = should_add_param or (prev_char != "\n" and (c == "B" and text_between[current_index : current_index + 3] == "Bit"))
        should_add_param = should_add_param or (not line_started_with_num and (prev_char.islower() or prev_char == ')' or prev_char.isnumeric()) and c.isupper())
        if should_add_param:
            param = ""
            if is_at_end:
                param = text_between[start_index : current_index + 1]
            else:
                param = text_between[start_index : current_index]
            params.append(param)
            start_index = current_index

        prev_char = c
        current_index = current_index + 1

    if len(text_between.strip()) != 0:
        print(cmd_name)
        for param in params:
            print("PARAM: " + param.replace("\n", ""))

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

    # if reply == "NO_REPLY":
    #     print(reply + "\t\t" + cmd_name)
    # else:
    #     calculated_reply = hex(~int(cmd_byte, base=16) & 0xf7).upper()
    #     reply = reply.upper()
    #     print(str(calculated_reply == reply) + "\t\t" + cmd_name + "\t\t" + calculated_reply + "\t\t" + reply)

    cmd_macro_calls = cmd_macro_calls + "Cmd(" + cmd_name + ", " + availability + ");\n"
    # if cmd_name_2 != None:
    #    cmd_macro_calls = cmd_macro_calls + "Cmd(" + cmd_name_2 + ", " + availability + ");\n"

cmd_byte_enum = cmd_byte_enum + "}\n"

# print(cmd_byte_enum)
# print()
# print(cmd_macro_calls)
