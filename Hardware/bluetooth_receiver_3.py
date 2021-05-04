import bluetooth
import time
import select
import RPi.GPIO as GPIO
import dht11
import subprocess
import sys

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.cleanup()

temp = 20
hum = 60
bt_mac_addr = ""

def read():
    f = open('/home/pi/Hardware/temp_humidity.txt', 'r+')
    line = f.readline()
    temp = float(line)
    line = f.readline()
    hum = float(line)
    f.close()
    ret = []
    ret.append(temp)
    ret.append(hum)
    return ret

f = open('/home/pi/Hardware/temp_humidity.txt', 'w+')
f.write("-1\n-1\n")
f.close()
f = open('/home/pi/Hardware/btr3.errorlog', 'w+')
f.close()

def get_mac_address_log():
    f = open('/home/pi/my_bluetooth.log', 'r+')
    lines = f.readlines()
    for line in lines:
        if(line[0:6] == "Device"):
            bt_mac_addr = line[len(line)-19:len(line)-2]
            f2 = open('/home/pi/Hardware/phone_mac.txt', 'w')
            f2.write(bt_mac_addr)
            f2.close()
    f.close()
    print("Read address %s from log" % bt_mac_addr)
    return bt_mac_addr

def get_mac_address_save():
    f = open('/home/pi/Hardware/phone_mac.txt', 'r+')
    bt_mac_addr = f.readline()
    f.close()
    print("Read address %s from save." % bt_mac_addr)
    return bt_mac_addr

def bt_connect(read_log):
    try:
        server_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        port = 1
        server_sock.bind(("", 1))
        server_sock.listen(1)
        client_sock,address = server_sock.accept()
        client_sock.setblocking(0)
        while True:
            client_sock.settimeout(10)
            mystr = ""
            try:
                recdata = client_sock.recv(1024)
                mystr = recdata.decode('utf-8')
            except:
                mystr = "z"
            try:
                if(mystr != "z"):
                    print("Received \"%s\" through Bluetooth"%mystr)
                if(mystr == "Q"):
                    print("Exiting")
                    break
                if(mystr[0] == 'c'):
                    color = mystr.split()
                    f = open('/home/pi/Hardware/color.txt', 'w+')
                    f.write(color[1] + "\n" + color[2] + "\n" + color[3])
                    f.close()
                    subprocess.call(["python3", "/home/pi/Hardware/image_to_stream.py"])
                if(mystr[0] == 'm'):
                    map_arr = mystr.split()
                    f = open('/home/pi/Hardware/map.txt', 'w+')
                    for x in map_arr:
                        if(x != "m"):
                            f.write(x)
                            f.write("\n")
                    f.close()
                if(mystr[0] == 'h'):
                    heat_arr = mystr.split()
                    f = open('/home/pi/Hardware/heat_hum_limits.txt', 'w+')
                    f.write(heat_arr[1] + "\n" + heat_arr[2])
                    f.close()
            except:
                f = open('/home/pi/Hardware/btr3.errorlog', 'a+')
                f.write("Error: unable to handle string. Received %s" % mystr)
                f.close()
            val = read()
            if(val[0] != -1 and val[1] != -1):
                client_sock.send("%t")
                client_sock.send(str(val[0]))
                client_sock.send("%h")
                client_sock.send(str(val[1]))
                client_sock.send("\n")
    except bluetooth.btcommon.BluetoothError as err:
        print(err, file=sys.stderr)
        read_log = not read_log
        time.sleep(1)
        pass

reading_log = False
while True:
    bt_connect(reading_log)
    reading_log = not reading_log
client_sock.close()
server_sock.close()
