import React from 'react';
import { IonButton } from '@ionic/react';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';

export let bluetoothComponent = {
  macAddress: 'B8:27:EB:C8:32:3B',
  sendText: "Test",

  testPiConnect: function() {
    if (BluetoothSerial.isEnabled()) {
      BluetoothSerial.connect('B8:27:EB:C8:32:3B')
      BluetoothSerial.subscribe("\n")
      let myTest:string = "This is a test string!"
      BluetoothSerial.write(myTest).then( () => alert("Test string: " + myTest + " has been sent to Raspberry Pi."), () => alert("Failed to send test message!") )
      BluetoothSerial.list().then(devices => alert("Paired devices: " + devices))
      BluetoothSerial.disconnect().then( () => alert("Test Finished, Bluetooth has been disconnected."))
    }
    else {
      alert("Connection failed. Make sure Bluetooth is enabled on both devices.")
    }
  },

  setMAC: function(MAC:string) {
    bluetoothComponent.macAddress = MAC;
    console.log("Given MAC Address: " + bluetoothComponent.macAddress)
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

interface ContainerProps {
    title: string;
  }
  
  const BluetoothButton: React.FC<ContainerProps> = ({ title }) => {   
    return (
      <div className="container" id="container">
        <strong>{title}</strong>
            <p>
                <IonButton color="primary" size="large" onClick={bluetoothComponent.testPiConnect}>Connect to Device</IonButton>
            </p>
      </div>
    );

  };
  
  export default BluetoothButton;