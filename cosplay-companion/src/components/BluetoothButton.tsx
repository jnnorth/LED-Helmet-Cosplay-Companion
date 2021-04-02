import React from 'react';
import { IonButton } from '@ionic/react';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';
import { Plugins } from '@capacitor/core'
const { Toast } = Plugins

export let bluetoothComponent = {
  macAddress: 'DC:A6:32:84:37:41',
    sendText: "Test",
    bluetooth_started: false,

    pi_connect: function () {
        BluetoothSerial.disconnect().then(() => {
            bluetoothComponent.testPiConnect()
        }, () => {
            bluetoothComponent.testPiConnect()
        })
    },

    testPiConnect: function () {
        BluetoothSerial.disconnect()
        BluetoothSerial.isEnabled().then(() => {
            BluetoothSerial.list().then(devices => {
                let correct_mac_address : string = ""
                correct_mac_address = bluetoothComponent.get_correct_mac(devices)
                if (correct_mac_address != "None") {
                    //alert("Attempting to connect with " + correct_mac_address + "...")
                    BluetoothSerial.connect(correct_mac_address).subscribe(() => {
                        // Successfully connected for write
                        BluetoothSerial.subscribe('\n').subscribe(success => {
                            // Subscribe success
                            // Now connected for read and write
                            // When a string is received, it is sent to the handle_read function
                            bluetoothComponent.handle_read(success)
                        }, error => {
                            alert("Error: " + error)
                        })
                        bluetoothComponent.bluetooth_started = true
                        let myTest: string = "This is a test string!"
                        BluetoothSerial.write(myTest).then(() => {
                            // Write Success
                            alert("Test string: " + myTest + " has been sent to Raspberry Pi.")
                        }, () => {
                            // Write Fail
                            alert("Failed to send test message!")
                        })
                    }, () => {
                        // Connection failed or ended
                        bluetoothComponent.bluetooth_started = false
                        alert("Connection failed.")
                        BluetoothSerial.disconnect()
                    })
                }
                else {
                    // No helmet paired
                    bluetoothComponent.bluetooth_started = false
                    alert("Please pair with a helmet first.")
                }
            }, () => {
                // Failure on BluetoothSerial.list()
                alert("Failed to list paired devices.")
            })
        }, () => {
                // Bluetooth not enabled
                bluetoothComponent.bluetooth_started = false
                BluetoothSerial.disconnect()
                alert("Please enable bluetooth.")
        })
    },

    // Debugging purposes only
    alertDevices: function (devices: pairedlist[]) {
        devices.forEach(device => {
            alert("Name: " + device.name + "\nAddress: " + device.address + "\nClass: " + device.class + "\nId: " + device.id)
        })
    },

    // This method sends the string to the Raspberry Pi
    send_to_pi: function (data: string) {
        BluetoothSerial.write(data).then(() => {
            alert("Sent " + data + " to pi")
        }, () => {
            bluetoothComponent.bluetooth_started = false
            alert("Failed to send data")
        })
    },

    // disconnects bluetooth
    disconnectBluetooth: function () {
        bluetoothComponent.bluetooth_started = false
        BluetoothSerial.disconnect()
    },

    // Scans all paired bluetooth devices and returns the MAC address of the device with the name "Team-21-Helmet"
    get_correct_mac: function (devices: pairedlist[]) {
        let ret: string = "None"
        devices.forEach(device => {
            if (device.name == "Team-21-Helmet") {
                ret = device.address
            }
        })
        return ret
    },

  setMAC: function(MAC:string) {
    bluetoothComponent.macAddress = MAC;
    console.log("Given MAC Address: " + bluetoothComponent.macAddress)
    },

    //TODO: properly handle heat and humidity data
    handle_read: function (heat_humidity: string) {
        // The heat_humidity string is formatted in the following way:
        // %t<temp:float>%h<humidity:float>
        // For example, if the temp were 23.2 degrees C and the humidity were 36.1%, the string would read
        // %t23.2%h36.1
        alert(heat_humidity)
    },

  onClick: function() {
    let isClicked = "true"
    bluetoothComponent.sendText = 'hello world'
    window.localStorage.setItem("sendText", bluetoothComponent.sendText)
    window.localStorage.setItem("isClicked", isClicked)
    //window.location.reload()
    bluetoothComponent.init()
  },

  init: function() {
    console.log("Initializing bluetooth connection...")
    alert(BluetoothSerial.discoverUnpaired())
    bluetoothComponent.bindEvents()
  },

  bindEvents: function() {
    document.addEventListener('findDevice', bluetoothComponent.connectBluetooth, false)
    bluetoothComponent.connectBluetooth()
  },

  connectBluetooth: function() {
    BluetoothSerial.connect(bluetoothComponent.macAddress)
    if (BluetoothSerial.isEnabled()) {
      bluetoothComponent.onConnect()
    }
    else
    {
      bluetoothComponent.connectFailed()
    }
  },

  onConnect: function() {
    BluetoothSerial.subscribe('\n')
    let data:string = 'This is a test string'
    alert("Connected to device with MAC address: " + bluetoothComponent.macAddress)
    bluetoothComponent.sendData(data)
  },

  onDisconnect: function() {
    alert("Device has been disconnected")
  },

  sendData: function(data:string) {
    BluetoothSerial.write(data)
    bluetoothComponent.sendText = data
  },

  listDevices: function() {
    BluetoothSerial.list()
  },

  connectFailed: function() {
    alert("Connection to device failed!")
  },

  onError: function(errorMsg:string) {
    console.log('Error: ' + errorMsg + '\n')
  },

  testRender: function() {

    let isClicked = window.localStorage.getItem("isClicked")
    let testText = window.localStorage.getItem("sendText")

    console.log("isClicked: " + isClicked)
    console.log("bluetoothComponent.sendText: " + bluetoothComponent.sendText)

    if (isClicked === "true")
    {
      return( <div className="sentDataTest"> Test: {testText}
      </div> )
    }
    else
    {
      return ( <div>Test: No text available yet</div> )
    }
    
  }
};

interface pairedlist {
    "class": number,
    "id": string,
    "address": string,
    "name": string
}

interface ContainerProps {
    title: string;
  }
  
  const BluetoothButton: React.FC<ContainerProps> = ({ title }) => {   
    return (
      <div className="container" id="container">
        <strong>{title}</strong>
            <p>
                <IonButton color="primary" size="large" onClick={bluetoothComponent.pi_connect}>Connect to Device</IonButton>
            </p>
      </div>
    );

  };
  
  export default BluetoothButton;
