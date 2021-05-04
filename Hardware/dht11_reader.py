import time
import RPi.GPIO as GPIO
import dht11

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.cleanup()

temp = 20
hum = 60

instance = dht11.DHT11(pin=19)
def read():
    result = instance.read()
    if result.is_valid():
        f = open('/home/pi/Hardware/temp_humidity.txt', 'w')
        f.write(str(result.temperature))
        f.write("\n")
        f.write(str(result.humidity))
        f.close()
        print("Temp: ", result.temperature, ", humidity: ", result.humidity, "%")
    else:
        pass

f = open("/home/pi/Hardware/temp_humidity.txt", 'w')
f.write("20\n60")
f.close()

while True:
    read()
    time.sleep(10)
