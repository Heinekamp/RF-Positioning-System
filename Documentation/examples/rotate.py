import pyvisa
import time

rm = pyvisa.ResourceManager()
EthInstrument = rm.open_resource('TCPIP0::192.168.0.50::5025::SOCKET')
EthInstrument.write_termination = '\r\n'
EthInstrument.read_termination = '\n'

print(EthInstrument.query("*IDN?")) # Query and Print the IDN of the instrument

EthInstrument.write("CONFigure:MOTor:SPEED 2000") # Set the motor speed to 5000
EthInstrument.write("CONFigure:MOTor:ACCeleration 5000") # Set the motor acceleration to 5000
EthInstrument.write("CONTRol:ROTation:DIRection2 CW") # Set the motor direction to clockwise

for i in range(10):
    EthInstrument.write("CONTRol:ROTation:RELative2 360") # Rotate the motor 360 degrees
    time.sleep(12) # Wait for 20 seconds to allow the motor to complete the rotation
#     EthInstrument.write("CONTRol:ROTation:RELative1 360") # Rotate the motor 360 degrees
#     time.sleep(4) # Wait for 20 seconds to allow the motor to complete the rotation
    EthInstrument.write("CONTRol:ROTation:DIRection2 CCW") # Set the motor direction to counterclockwise
    EthInstrument.write("CONTRol:ROTation:RELative2 360") # Rotate the motor 360 degrees
    time.sleep(12) # Wait for 20 seconds to allow the motor to complete the rotation
    EthInstrument.write("CONTRol:ROTation:DIRection2 CW") # Set the motor direction to clockwise

# EthInstrument.write("CONTRol:ROTation:DIRection1 CCW") # Set the motor direction to counterclockwise

# for i in range(5):
#     EthInstrument.write("CONTRol:ROTation:RELative1 360") # Rotate the motor 360 degrees
#     time.sleep(5) # Wait for 20 seconds to allow the motor to complete the rotation

# EthInstrument.write("CONTRol:ROTation:DIRection1 CW") # Set the motor direction to counterclockwise

    #start: 59.45
    #end: 59.5
    #start: 59.5
    #end: 59.55

    #end in other direction
    #start: 59.55
    #end: 59.7

# for i in range(2): 
#     for i in range(360):
#         EthInstrument.write("CONTRol:ROTation:RELative2 1") # Rotate the motor 10 degrees
#         time.sleep(1)
#     time.sleep(5)