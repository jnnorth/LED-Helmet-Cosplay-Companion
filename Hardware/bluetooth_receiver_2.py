import bluetooth
import sys
from time import time
sys.path.append('/home/pi/.local/lib/python3.7/site-packages')


bd_addr = 'ac:67:b2:37:4c:86'

def bt_connect(port):
    try:
        s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        s.connect((bd_addr, port))
        while 30 < int(time() * 1000) % 100 < 70:
            x = 0 # do nothing, must have some code here to avoid indentation error
        f = open('/home/pi/Hardware/blue_input_2.txt', 'w+')
        f.write("200")
        f.close()
        print("Connected to glove 2.")
        original_stdout = sys.stdout
        while True:
            mystr = ""
            received = s.recv(1024)
            mystr = received.decode('utf-8')
            ms_time = int(time() * 1000) % 100
            while 30 < ms_time < 70:
                ms_time = int(time() * 1000) % 100
            f = open('/home/pi/Hardware/blue_input_2.txt', 'w')
            f.write(mystr + '\n')
            f.close()
            sys.stdout = original_stdout
    except bluetooth.btcommon.BluetoothError as err:
        print(err)
        pass

while True:
    for port_num in range(1, 3):
        bt_connect(port_num)

while 30 < int((time() * 1000) % 100) < 70:
    x = 0
print("writing failure text")
f = open('/home/pi/Hardware/blue_input_1.txt', 'w')
f.write("-1")
f.close()
