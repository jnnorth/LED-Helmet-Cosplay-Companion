import React from 'react';
import { IonButton } from '@ionic/react';
import './BluetoothButton.css';
import { BluetoothLE } from '@ionic-native/bluetooth-le';

const scanBluetooth = async () => {
    let scanParams = {
        "services": [],
        "allowDuplicates": true,
        "scanMode": BluetoothLE.SCAN_MODE_LOW_LATENCY,
        "matchMode": BluetoothLE.MATCH_MODE_AGGRESSIVE,
        "matchNum": BluetoothLE.MATCH_NUM_MAX_ADVERTISEMENT,
        "callbackType": BluetoothLE.CALLBACK_TYPE_ALL_MATCHES,
      }
    BluetoothLE.initialize()
    let devices = BluetoothLE.startScan(scanParams)
    console.log("This is a test")
    console.log(devices)
}

interface ContainerProps {
    title: string;
  }
  
  const BluetoothButton: React.FC<ContainerProps> = ({ title }) => {
    return (
      <div className="container">
        <strong>{title}</strong>
            <p>
                <IonButton color="primary" size= "large" onClick= {scanBluetooth}>Scan for devices</IonButton>
            </p>
      </div>
    );
  };
  
  export default BluetoothButton;