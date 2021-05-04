To run the code, install the `Hardware` folder onto the raspberry pi. Add the following lines to the `.bashrc` file in the home folder:
```
sudo bash my_bluetooth.sh
cd Hardware
python3 dht11_reader.py &
python3 bluetooth_receiver_3 .py &
./main &
```
Next, the Bluetooth must be set up. First, a file must be created to store the passcode used to initialize the bluetooth connection. Use `sudo vim /root/bluetooth.cfg`, or your preferred text editor, to create this file. The following example sets the passcode to '12345':
```
* 12345
```
In the home folder, create a bash file titled `my_bluetooth.sh` with the following contents:
```
sleep 6
sudo hciconfig hci0 up
sudo hciconfig hci0 piscan
sudo hciconfig hci0 sspmode 0
sudo bt-agent -c NoInputNoOutput -p /root/bluetooth.cfg > my_bluetooth.log &
```
The next step in the setup is to [install the hzeller library](https://github.com/hzeller/rpi-rgb-led-matrix)<br>
If the `Hardware` and  `rpi-rgb-led-matrix` directories are not in the home directory, the makefile must be updated to accurately define the path to the RGB_DISTRIBUTION.<br>
Finally, in the terminal, run the following commands:
```
$ sudo apt-get install bluez
$ sudo pip3 install pybluez
```
