import React from 'react';
import { bluetoothComponent } from './BluetoothButton';


class MACForm extends React.Component<{}, {value:string}> {
    constructor(props:any) {
      super(props);
      this.state = {value: ''};
      this.handleChange = this.handleChange.bind(this);
      this.handleSubmit = this.handleSubmit.bind(this);
    }
  
    handleChange(event:any) {
      this.setState({value: event.target.value});
    }
  
    handleSubmit(event:any) {
      bluetoothComponent.setMAC(this.state.value);
      event.preventDefault();
    }
  
    render() {
      return (
        <form id="submitForm" onSubmit={this.handleSubmit}>
          <label>
            Enter MAC Address of Raspberry Pi: 
                <input type="text" value={this.state.value} onChange={this.handleChange}/>
          </label>
          <input type="submit" value="Submit"/>
        </form> 
      );
    }
  };
  
  export default MACForm;