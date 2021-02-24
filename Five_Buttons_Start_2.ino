#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run 'make menuconfig' to and enable it
#endif

BluetoothSerial SerialBT;

int OUTPUT_PIN = 16;
int BUTTON_1_PIN = 17;
int BUTTON_2_PIN = 18;
int BUTTON_3_PIN = 19;
int BUTTON_4_PIN = 21;
int BUTTON_5_PIN = 22;

int BUTTON_1_STATE_OLD;
int BUTTON_2_STATE_OLD;
int BUTTON_3_STATE_OLD;
int BUTTON_4_STATE_OLD;
int BUTTON_5_STATE_OLD;

int BUTTON_1_STATE;
int BUTTON_2_STATE;
int BUTTON_3_STATE;
int BUTTON_4_STATE;
int BUTTON_5_STATE;


void setup() {
  // put your setup code here, to run once:
  // Button pin setup
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);
  pinMode(BUTTON_3_PIN, INPUT);
  pinMode(BUTTON_4_PIN, INPUT);
  pinMode(BUTTON_5_PIN, INPUT);

  // Initialize old button states to LOW
  BUTTON_1_STATE_OLD = LOW;
  BUTTON_2_STATE_OLD = LOW;
  BUTTON_3_STATE_OLD = LOW;
  BUTTON_4_STATE_OLD = LOW;
  BUTTON_5_STATE_OLD = LOW;

  // Begin bluetooth and debugging monitor
  Serial.begin(115200);
  SerialBT.begin("ESP32SecondBoard");
  digitalWrite(OUTPUT_PIN, HIGH);
  Serial.print("Device started, you can now pair with bluetooth!\n");
}

int get_message() {
  int ret = 0;
  if(BUTTON_1_STATE == HIGH) {
    ret += 1;
  }
  if(BUTTON_2_STATE == HIGH) {
    ret += 2;
  }
  if(BUTTON_3_STATE == HIGH) {
    ret += 4;
  }
  if(BUTTON_4_STATE == HIGH) {
    ret += 8;
  }
  if(BUTTON_5_STATE == HIGH) {
    ret += 16;
  }
  return ret;
}

void loop() {
  // put your main code here, to run repeatedly:
  BUTTON_1_STATE = digitalRead(BUTTON_1_PIN);
  BUTTON_2_STATE = digitalRead(BUTTON_2_PIN);
  BUTTON_3_STATE = digitalRead(BUTTON_3_PIN);
  BUTTON_4_STATE = digitalRead(BUTTON_4_PIN);
  BUTTON_5_STATE = digitalRead(BUTTON_5_PIN);
  bool change = false;
  delay(100);
  if(BUTTON_1_STATE != BUTTON_1_STATE_OLD) {
    /*
    if(BUTTON_1_STATE == HIGH) {
      SerialBT.print("Button 1 pressed");
    }
    else {
      SerialBT.print("Button 1 released");
    }*/
    change = true;
  }
  
  if(BUTTON_2_STATE != BUTTON_2_STATE_OLD) {
    /*
    if(BUTTON_2_STATE == HIGH) {
      SerialBT.print("Button 2 pressed");
    }
    else {
      SerialBT.print("Button 2 released");
    }*/
    change = true;
  }
  if(BUTTON_3_STATE != BUTTON_3_STATE_OLD) {
    /*
    if(BUTTON_3_STATE == HIGH) {
      SerialBT.print("Button 3 pressed");
    }
    else {
      SerialBT.print("Button 3 released");
    }
    */
    change = true;
  }
  if(BUTTON_4_STATE != BUTTON_4_STATE_OLD) {
    /*
    if(BUTTON_4_STATE == HIGH) {
      SerialBT.print("Button 4 pressed");
    }
    else {
      SerialBT.print("Button 4 released");
    }
    */
    change = true;
  }
  if(BUTTON_5_STATE != BUTTON_5_STATE_OLD) {
    /*
    if(BUTTON_5_STATE == HIGH) {
      SerialBT.print("Button 5 pressed");
    }
    else {
      SerialBT.print("Button 5 released");
    }
    */
    change = true;
  }

  if(change) {
    SerialBT.print(get_message());
  }
  BUTTON_1_STATE_OLD = BUTTON_1_STATE;
  BUTTON_2_STATE_OLD = BUTTON_2_STATE;
  BUTTON_3_STATE_OLD = BUTTON_3_STATE;
  BUTTON_4_STATE_OLD = BUTTON_4_STATE;
  BUTTON_5_STATE_OLD = BUTTON_5_STATE;
}
