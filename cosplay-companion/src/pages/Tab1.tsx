import React from 'react';
import { IonContent, IonHeader, IonPage, IonTitle, IonToolbar } from '@ionic/react';
import BluetoothButton from '../components/BluetoothButton';
import './Tab1.css';

const Tab1: React.FC = () => {
  return (
    <IonPage>
      <IonHeader>
        <IonToolbar>
          <IonTitle>Bluetooth</IonTitle>
        </IonToolbar>
      </IonHeader>
      <IonContent fullscreen>
        <IonHeader collapse="condense">
          <IonToolbar>
            <IonTitle size="large">Bluetooth</IonTitle>
          </IonToolbar>
        </IonHeader>
        <BluetoothButton title="Connect to Raspberry Pi via Bluetooth"/>
      </IonContent>
    </IonPage>
  );
};

export default Tab1;
