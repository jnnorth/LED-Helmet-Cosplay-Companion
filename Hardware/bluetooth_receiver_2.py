# Write your code here :-)
import bluetooth
import sys
from time import time


bd_addr = 'ac:67:b2:37:4c:86'
port = 1

try:
    s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    print("Connecting...")
    s.connect((bd_addr, port))
    print("Connection successful")
    original_stdout = sys.stdout
    while True:
        mystr = ""
        received = s.recv(1024)
        mystr = received.decode('utf-8')
        print(mystr)
        ms_time = int(time() * 1000) % 100
        while 30 < ms_time and ms_time < 70:
            ms_time = int(time() * 1000) % 100
        f = open('/home/pi/Hardware/blue_intput_2_2.txt', 'w')
        f.write(mystr + '\n')
        f.close()
        sys.stdout = original_stdout
except bluetooth.btcommon.BluetoothError as err:
    print(err)
    pass
