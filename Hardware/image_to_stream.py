import glob
import subprocess

def generate_stream(img_file):
    cmd = "/home/pi/Hardware/utils/led-image-streamer " + img_file + " -O " + img_file + ".stream --led-no-hardware-pulse"
    subprocess.call(cmd, shell=True)
    print(cmd)

arr = glob.glob("/home/pi/Hardware/Images/Faces/*.png")
for img in arr:
    generate_stream(img)
