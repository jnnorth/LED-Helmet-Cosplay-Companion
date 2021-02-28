import React from 'react';
import { IonButton } from '@ionic/react';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';


let bluetoothComponent = {
  macAddress: 'AA:BB:CC:DD:EE:FF', // dummy MAC address, need to manually enter MAC address of Raspberry Pi
  sendText: "Test",

  onClick: function() {
    let isClicked = "true"
    bluetoothComponent.sendText = 'hello world'
    window.localStorage.setItem("sendText", bluetoothComponent.sendText)
    window.localStorage.setItem("isClicked", isClicked)
    window.location.reload(false)
    bluetoothComponent.init()
  },

  init: function() {
    console.log("Initializing bluetooth connection...")
    bluetoothComponent.bindEvents()
  },

  bindEvents: function() {
    document.addEventListener('findDevice', bluetoothComponent.connectBluetooth, false)
  },

  connectBluetooth: function() {
    BluetoothSerial.connect(bluetoothComponent.macAddress)
    if (BluetoothSerial.isEnabled()) {
      bluetoothComponent.onConnect()
    }
    else {
      bluetoothComponent.onDisconnect()
    }
  },

  onConnect: function() {
    alert("Connected to device with MAC address: " + bluetoothComponent.macAddress + ".")
    BluetoothSerial.subscribe('\n')
    let data:string = 'This is a test string'
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
      <div className="container">
        <strong>{title}</strong>
            <p>
                <IonButton color="primary" size="large" onClick={bluetoothComponent.onClick}>Connect to Device</IonButton>
            </p>    
            <div>
              {
                bluetoothComponent.testRender()
              }
            </div>
      </div>
    );
  };
  
  export default BluetoothButton;