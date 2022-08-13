
def process(data):
  newData = ""
  for c in data:
    if not c.isspace():
      newData = newData + c
    
  newNewData = "0x51, 0x"
  counter = 1
  for c in newData:
    newNewData = newNewData + c
    if counter % 2 == 0 and counter != 48:
      newNewData = newNewData + ", 0x"
    counter = counter + 1
  
  return newNewData

lines = """Beep1Immediate	51 0a   15 0a 0b 14 15 0a   15 0a   15 0a   15 0a   15 0a 0b 14 15 0a 15 0a   0b0b00
Beep2Immediate	51 0a   15 0a 15 0a 0b 14   15 0a   15 0a   15 0a   15 0a 0b 14 15 0a 0b 14   0b0b00
Beep3Immediate	51 0a   15 0a 15 0a 15 0a   15 0a   15 0a   15 0a   15 0a 0b 14 0b 14 15 0a   0b0b00
Beep4Immediate	51 0a   0b 14 0b 14 0b 14   15 0a   15 0a   15 0a   15 0a 0b 14 0b 14 0b 14   0b0b00
Beep5Immediate	51 0a   0b 14 15 0a 0b 14   15 0a   15 0a   15 0a   0b 14 15 0a 15 0a 15 0a   0b0b00
ForwardImmediate	51 0a   0b 14 0b 14 0b 14   15 0a   15 0a   15 0a   15 0a 15 0a 15 0a 15 0a   0b0b00
BackwardImmediate	51 0a   0b 14 0b 14 15 0a   15 0a   15 0a   15 0a   15 0a 15 0a 15 0a 0b 14   0b0b00
Stop		51 0a   15 0a 0b 14 0b 14   15 0a   15 0a   15 0a   0b 14 15 0a 0b 14 15 0a   0b0b00
Run		51 0a   0b 14 15 0a 15 0a   15 0a   0b 14   15 0a   15 0a 15 0a 15 0a 0b 14   0b0b00
Delete	51 0a   15 0a 0b 14 0b 14   15 0a   0b 14   15 0a   15 0a 15 0a 0b 14 15 0a   0b0b00
Beep1	51 0a   15 0a 15 0a 15 0a   15 0a   15 0a   0b 14   0b 14 15 0a 15 0a 15 0a   0b0b00
Beep2	51 0a   0b 14 0b 14 0b 14   15 0a   15 0a   0b 14   0b 14 15 0a 15 0a 0b 14   0b0b00
Beep3	51 0a   0b 14 0b 14 15 0a   15 0a   15 0a   0b 14   0b 14 15 0a 0b 14 15 0a   0b0b00
Beep4	51 0a   0b 14 15 0a 0b 14   15 0a   15 0a   0b 14   0b 14 15 0a 0b 14 0b 14   0b0b00
Beep5	51 0a   15 0a 0b 14 0b 14   15 0a   15 0a   0b 14   0b 14 0b 14 15 0a 15 0a   0b0b00
ForwardHalf	51 0a   15 0a 0b 14 15 0a   15 0a   15 0a   0b 14   15 0a 15 0a 15 0a 15 0a   0b0b00
ForwardOne	51 0a   15 0a 15 0a 0b 14   15 0a   15 0a   0b 14   15 0a 15 0a 15 0a 0b 14   0b0b00
ForwardTwo	51 0a   15 0a 15 0a 15 0a   15 0a   15 0a   0b 14   15 0a 15 0a 0b 14 15 0a   0b0b00
ForwardFive	51 0a   0b 14 0b 14 0b 14   15 0a   15 0a   0b 14   15 0a 15 0a 0b 14 0b 14   0b0b00
BackwardHalf	51 0a   0b 14 15 0a 0b 14   15 0a   15 0a   0b 14   15 0a 0b 14 15 0a 15 0a   0b0b00
BackwardOne	51 0a   0b 14 15 0a 15 0a   15 0a   15 0a   0b 14   15 0a 0b 14 15 0a 0b 14   0b0b00
BackwardTwo	51 0a   15 0a 0b 14 0b 14   15 0a   15 0a   0b 14   15 0a 0b 14 0b 14 15 0a   0b0b00
BackwardFive	51 0a   15 0a 0b 14 15 0a   15 0a   15 0a   0b 14   15 0a 0b 14 0b 14 0b 14   0b0b00
WaitLight	51 0a   15 0a 0b 14 15 0a   15 0a   15 0a   0b 14   0b 14 0b 14 15 0a 0b 14   0b0b00
SeekLight	51 0a   15 0a 15 0a 0b 14   15 0a   15 0a   0b 14   0b 14 0b 14 0b 14 15 0a   0b0b00
Code		51 0a   15 0a 15 0a 15 0a   15 0a   15 0a   0b 14   0b 14 0b 14 0b 14 0b 14   0b0b00
KeepAlive	51 0a   0b 14 15 0a 0b 14   15 0a   0b 14   15 0a   15 0a 15 0a 15 0a 15 0a   0b0b00"""

constants = []
functionCalls = []

for line in lines.splitlines():
  parts = line.split("51", 1)
  constants.append("#define " + parts[0].strip().upper() + " { " + process(parts[1]) + " }")
  functionCalls.append("GenerateVLLFunction(" + parts[0].strip() + ", " + parts[0].strip().upper() + ")")

for s in constants:
  print(s);

print()

for s in functionCalls:
  print(s)
