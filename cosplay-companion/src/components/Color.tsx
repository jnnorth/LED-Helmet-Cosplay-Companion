import React from 'react';
import { IonButton } from '@ionic/react';
import { bluetoothComponent } from '../components/BluetoothButton';
import './Color.css';


function hexToRGBString(hex:string) {
  let res = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex)!;
  if (hex.length != 7){
    throw "Only six-digit hex colors are allowed.";
  }

  let rgb = [ 
    parseInt(res[1], 16),
    parseInt(res[2], 16),
    parseInt(res[3], 16)
  ];

  let type = "c ";
  let returnString = type + String(rgb[0]) + " " + String(rgb[1]) + " " + String(rgb[2]);
  
  sendButtonMapping(returnString);
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
    console.log("Bluetooth connection has not been established!")
    return data
  }
}

class Color extends React.Component<{}, {input:string}> {
    constructor(props:any) {
      super(props);
      this.state = {input: '#ffffff'};
      this.handleChange = this.handleChange.bind(this);
      this.handleSubmit = this.handleSubmit.bind(this);
    }

    handleChange(event:any) {
      //console.log(this.state.input);
      this.setState({input: event.target.value});
    }
  
    handleSubmit(event:any) {
      event.preventDefault();
    }

  

    render() {
      return (
           
        <div className="picker" id="picker">
          <label>
            <h1>Select a color: </h1>
                <input type="color" id="Color" value={this.state.input} onChange={this.handleChange}/>
          </label>
          <p>
            <IonButton color="secondary" size="large" onClick={() => hexToRGBString(this.state.input)}> Confirm Choice</IonButton>
        </p></div>

      );
    }

  };
  
  export default Color;
