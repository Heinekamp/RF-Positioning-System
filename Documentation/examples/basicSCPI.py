import pyvisa
import time

rm = pyvisa.ResourceManager()
EthInstrument = rm.open_resource('TCPIP0::192.168.0.50::5025::SOCKET')
EthInstrument.write_termination = '\r\n'
EthInstrument.read_termination = '\n'

# Set a higher timeout, e.g., 10 seconds
EthInstrument.timeout = 10000  # Timeout in milliseconds

print(EthInstrument.query("*IDN?")) # Query and Print the IDN of the instrument

EthInstrument.write("CONFigure:MOTor:SPEED 5000") # Set the motor speed to 5000
EthInstrument.write("CONFigure:MOTor:ACCeleration 5000") # Set the motor acceleration to 5000

EthInstrument.write("CONTRol:ROTation:RELative2 360") # Rotate the motor 360 degrees

while (EthInstrument.query("*OPC?") != "1"):
        time.sleep(0.1) # Wait for the operation to complete

EthInstrument.write("CONFigure:MOTor:ACCeleration 1000") # Set the motor acceleration to 5000

for i in range(36):
    EthInstrument.write("CONTRol:ROTation:RELative2 10") # Rotate the motor 10 degrees
    print(EthInstrument.query("CONTRol:ROTation:ABSolute2?")) # Query the absolute position of the motor
    while (EthInstrument.query("*OPC?") != "1"):
        time.sleep(0.1) # Wait for the operation to complete
    time.sleep(1)

EthInstrument.write("CONFigure:MOTor:SPEED 20000") # Set the motor speed to 20000
EthInstrument.write("CONFigure:MOTor:ACCeleration 5000") # Set the motor acceleration to 5000
EthInstrument.write("CONTRol:ROTation:RELative2 360") # Rotate the motor 360 degrees

EthInstrument.write("CONTRol:ROTation:DIRection2 CCW") # Set the motor direction to counter-clockwise
EthInstrument.write("CONFigure:MOTor:SPEED 5000") # Set the motor speed to 5000
EthInstrument.write("CONTRol:ROTation:RELative2 360") # Rotate the motor 360 degrees

EthInstrument.write("CONTRol:ROTation:DIRection2 CW") # Set the motor direction to clockwise
EthInstrument.write("CONTRol:ROTation:RELative2 360") # Rotate the motor 360 degrees

for i in range(360):
    EthInstrument.write(f"CONTRol:ROTation:ABSolute2 {i}")
    print(EthInstrument.query("CONTRol:ROTation:ABSolute2?")) # Query the absolute position of the motor
    while (EthInstrument.query("*OPC?") != "1"):
        time.sleep(0.1) # Wait for the operation to complete