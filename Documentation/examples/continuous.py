import pyvisa
import time

rm = pyvisa.ResourceManager()
EthInstrument = rm.open_resource('TCPIP0::192.168.0.50::5025::SOCKET')
EthInstrument.write_termination = '\r\n'
EthInstrument.read_termination = '\n'

# Set a higher timeout, e.g., 10 seconds
EthInstrument.timeout = 100000  # Timeout in milliseconds

print(EthInstrument.query("*IDN?")) # Query and Print the IDN of the instrument

EthInstrument.write("CONTRol:ROTation:STEP2") # Rotate the motor 360 degrees


while True:
    EthInstrument.write("CONTRol:ROTation:STEP2") # Rotate the motor 360 degrees
    while (EthInstrument.query("*OPC?") != "1"):
        time.sleep(0.1) # Wait for the operation to complete