import React, { useState } from 'react';
import { IonContent, IonHeader, IonPage, IonTitle,
IonCard, IonCardHeader, IonCardTitle, IonCardContent,
IonButton, IonModal, IonItem, IonLabel, IonSelect, IonSelectOption,
IonGrid, IonRow, IonCol, IonList } from '@ionic/react';
import './Tab2.css';
import { bluetoothComponent } from '../components/BluetoothButton';
import { SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS } from 'constants';

let mapStr:string = "m "
let buttonToMap:string | undefined = ""
let buttonMapped:boolean = false
let modMapped:boolean = false
let animMapped:boolean = false

let animNumStr:string = ""
let modMapStr:string | undefined = ""

function updateMapStr(data:string | undefined, type:string)
{ 
  if (type == "button")
  {
    if (data == "button1") buttonToMap = "1"
    if (data == "button2") buttonToMap = "2"
    if (data == "button3") buttonToMap = "3"
    if (data == "button4") buttonToMap = "4"
    buttonMapped = true
  }

  if (type == "anim")
  { 
    let animNum:string = "0"
    // Images
    if (data == "Neutral") animNum = "0"
    if (data == "Cheerful") animNum = "1"
    if (data == "Starry-eyed") animNum = "2"
    if (data == "Excited") animNum = "3"
    if (data == "Happy shock") animNum = "4"
    if (data == "Sad shock") animNum = "5"
    if (data == "Teary") animNum = "6"
    if (data == "Bluescreen") animNum = "7"
    if (data == "Unimpressed") animNum = "8"
    if (data == "Angry") animNum = "9"
    if (data == "Dead") animNum = "10"
    if (data == "Table flip") animNum = "11"
    if (data == "Party Parrot") animNum = "12"
    if (data == "Rickroll") animNum = "13"
    if (data == "The Game") animNum = "14"
    if (data == "This is fine") animNum = "15"

    animNumStr = animNum
    animMapped = true
  }

  if (type == "mod")
  { 
    modMapStr = data
  }

  return data
}

function getButtonCombo(button:string | undefined, mod:string | undefined)
{
  let modMapping:string = ""
// Button Combinations
  if (button == "1" && mod == "noMod") modMapping = "0"
  if (button == "1" && mod == "mod1") modMapping = "1"
  if (button == "1" && mod == "mod2") modMapping = "2"
  if (button == "1" && mod == "mod12") modMapping = "3"
  if (button == "2" && mod == "noMod") modMapping = "4"
  if (button == "2" && mod == "mod1") modMapping = "5"
  if (button == "2" && mod == "mod2") modMapping = "6"
  if (button == "2" && mod == "mod12") modMapping = "7"
  if (button == "3" && mod == "noMod") modMapping = "8"
  if (button == "3" && mod == "mod1") modMapping = "9"
  if (button == "3" && mod == "mod2") modMapping = "10"
  if (button == "3" && mod == "mod12") modMapping = "11"
  if (button == "4" && mod == "noMod") modMapping = "12"
  if (button == "4" && mod == "mod1") modMapping = "13"
  if (button == "4" && mod == "mod2") modMapping = "14"
  if (button == "4" && mod == "mod12") modMapping = "15"

  modMapped = true
  return modMapping
}

function submitStr()
{
  let buttonCombo:string = getButtonCombo(buttonToMap, modMapStr)
  if (buttonMapped && modMapped && animMapped)
  {
    buttonMapped = false
    modMapped = false
    animMapped = false

    mapStr = mapStr + buttonCombo + " " + animNumStr
    modMapStr = ""
    animNumStr = ""

    sendButtonMapping(mapStr)
    mapStr = "m "
    buttonToMap = ""
  }

  return false
}

function sendButtonMapping(data:string)
{
  console.log(data)
  if (bluetoothComponent.bluetooth_started)
  {
    bluetoothComponent.send_to_pi(data)
    return data
  }
  else
  {
    alert("Bluetooth connection has not been established!")
    console.log("Bluetooth connection has not been established!")
    return data
  }
}

function setTempAndHumd(temp:string | undefined, humd:string | undefined)
{
  console.log(temp + " and " + humd)
  sendTempAndHumd("h " + temp + " " + humd)
  return false
}

function sendTempAndHumd(toSend:string)
{
  console.log("toSend: " + toSend)
  if (bluetoothComponent.bluetooth_started)
  {
    bluetoothComponent.send_to_pi(toSend)
  }
  else
  {
    alert("Bluetooth connection has not been established!")
  }
}

const Tab2: React.FC = () => {

  const [showModal1, setShowModal1] = useState(false);
  const [showModal2, setShowModal2] = useState(false);

  const [buttonChoice, setButtonChoice] = useState<string>();
  const [modChoice, setModChoice] = useState<string>();
  const [animChoice, setAnimChoice] = useState<string>();

  const [temperature, setTemp] = useState<string>();
  const [humidity, setHumd] = useState<string>();

  function resetForm()
  {
    setTemp("")
    setHumd("")
  }

  return (
    <IonPage>

      <IonContent>
        <div id="myPage">


          <IonGrid>
            <IonRow>
              <IonCol>
                <IonCard style={{ width: '150px'}}>
                  <IonCardHeader>
                    <IonCardTitle>Temperature</IonCardTitle>
                  </IonCardHeader>
                  <IonCardContent>
                    XX°C
                  </IonCardContent>
                </IonCard>
              </IonCol>

              <IonCol>
                <IonCard style={{ width: '150px' }}>
                  <IonCardHeader>
                    <IonCardTitle>Humidity</IonCardTitle>
                  </IonCardHeader>
                  <IonCardContent>
                    YY%
                  </IonCardContent>
                </IonCard>
              </IonCol>
            </IonRow>
          </IonGrid>


          <IonModal isOpen={showModal1}>
            <form>
            <p> <b>Enter new temperature and humidity preferences</b> </p>
            <p> Fans will turn on when conditions inside the helmet exceed these values. </p>
              <IonList>
                <IonItem>
                  <label>
                    <b>Temperature:</b>
                    <input value={temperature} id="temp" type = "text" placeholder = "currently AA" onChange={e => setTemp(e.target.value)} />
                  </label>
                </IonItem>
                <IonItem>
                  <label>
                    <b>Humidity:</b>
                    <input value={humidity} id="humd" type = "text" placeholder = "currently BB" onChange ={e => setHumd(e.target.value)} />
                  </label>
                </IonItem>
              </IonList>
            </form>
            <IonButton onClick={() => {setTempAndHumd(temperature, humidity); setShowModal1(false); resetForm(); } }>Ok</IonButton>
          </IonModal>
          <IonButton onClick={() => setShowModal1(true)} color="success" shape="round" expand="block">
            Change temperature & humidity preferences
          </IonButton>


          <IonGrid>
            <IonRow>
              <IonCol id="myHeader">  </IonCol>
              <IonCol id="myHeader"> No mod </IonCol>
              <IonCol id="myHeader"> Mod 1 </IonCol>
              <IonCol id="myHeader"> Mod 2 </IonCol>
              <IonCol id="myHeader"> Mods 1 & 2 </IonCol>
            </IonRow>

            <IonRow>
              <IonCol id="myHeader"> Button 1 </IonCol>
              <IonCol id="myCol"> Neutral </IonCol>
              <IonCol id="myCol"> Cheerful </IonCol>
              <IonCol id="myCol"> Starry-eyed </IonCol>
              <IonCol id="myCol"> Excited </IonCol>
            </IonRow>

            <IonRow>
              <IonCol id="myHeader"> Button 2 </IonCol>
              <IonCol id="myCol"> Happy shock </IonCol>
              <IonCol id="myCol"> Sad shock </IonCol>
              <IonCol id="myCol"> Teary </IonCol>
              <IonCol id="myCol"> Bluescreen </IonCol>
            </IonRow>

            <IonRow>
              <IonCol id="myHeader"> Button 3 </IonCol>
              <IonCol id="myCol"> Unimpressed </IonCol>
              <IonCol id="myCol"> Angry </IonCol>
              <IonCol id="myCol"> Dead </IonCol>
              <IonCol id="myCol"> Table flip </IonCol>
            </IonRow>

            <IonRow>
              <IonCol id="myHeader"> Button 4 </IonCol>
              <IonCol id="myCol"> Party Parrot </IonCol>
              <IonCol id="myCol"> Rickroll </IonCol>
              <IonCol id="myCol"> The Game </IonCol>
              <IonCol id="myCol"> This is fine </IonCol>
            </IonRow>
          </IonGrid>


          <IonModal isOpen={showModal2}>
            <IonList>
              <IonItem>
                <IonLabel>Button</IonLabel>
                <IonSelect value={buttonChoice} placeholder="Select One" onIonChange={e => updateMapStr(e.detail.value, "button")}>
                  <IonSelectOption value="button1">1</IonSelectOption>
                  <IonSelectOption value="button2">2</IonSelectOption>
                  <IonSelectOption value="button3">3</IonSelectOption>
                  <IonSelectOption value="button4">4</IonSelectOption>
                </IonSelect>
              </IonItem>

              <IonItem>
                <IonLabel>Modifier</IonLabel>
                <IonSelect value={modChoice} placeholder="Select One" onIonChange={e => updateMapStr(e.detail.value, "mod")}>
                  <IonSelectOption value="noMod">none</IonSelectOption>
                  <IonSelectOption value="mod1">1</IonSelectOption>
                  <IonSelectOption value="mod2">2</IonSelectOption>
                  <IonSelectOption value="mod12">1 & 2</IonSelectOption>
                </IonSelect>
              </IonItem>

              <IonItem>
                <IonLabel>Animation</IonLabel>
                <IonSelect value={animChoice} placeholder="Select One" onIonChange={e => updateMapStr(e.detail.value, "anim")}>
                  <IonSelectOption>Neutral</IonSelectOption>
                  <IonSelectOption>Cheerful</IonSelectOption>
                  <IonSelectOption>Starry-eyed</IonSelectOption>
                  <IonSelectOption>Excited</IonSelectOption>
                  <IonSelectOption>Happy shock</IonSelectOption>
                  <IonSelectOption>Sad shock</IonSelectOption>
                  <IonSelectOption>Teary</IonSelectOption>
                  <IonSelectOption>Bluescreen</IonSelectOption>
                  <IonSelectOption>Unimpressed</IonSelectOption>
                  <IonSelectOption>Angry</IonSelectOption>
                  <IonSelectOption>Dead</IonSelectOption>
                  <IonSelectOption>Table flip</IonSelectOption>
                  <IonSelectOption>Party Parrot</IonSelectOption>
                  <IonSelectOption>Rickroll</IonSelectOption>
                  <IonSelectOption>The Game</IonSelectOption>
                  <IonSelectOption>This is fine</IonSelectOption>
                </IonSelect>
              </IonItem>

            </IonList>
            <IonButton onClick={() => setShowModal2( submitStr() )}>Ok</IonButton>
          </IonModal>
          <IonButton onClick={() => setShowModal2(true)} color="success" shape="round" expand="block">
            Change animation mapping
          </IonButton>




        </div>
      </IonContent>
    </IonPage>
  );
};

export default Tab2;
