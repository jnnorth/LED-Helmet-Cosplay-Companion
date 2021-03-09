import React, {ReactDOM} from 'react';
import { IonContent, IonHeader, IonPage, IonTitle, IonToolbar } from '@ionic/react';
import BluetoothButton from '../components/BluetoothButton';
import MACForm from '../components/MACForm'
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
        <MACForm></MACForm>
      </IonContent>
    </IonPage>
  );
};

export default Tab1;
