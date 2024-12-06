// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet send status:\t");

  String s;
  if (status == ESP_NOW_SEND_SUCCESS) {
    s = "Delivery: ok";
  } else {
    s = "Delivery: fail";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  //Serial.print("Bytes received: ");
  //Serial.println(len);

  //Send the reply back
  outgoingMessage.message = counter;
  esp_now_send(broadcastAddress, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage));

  if(incoming_on_time != incomingMessage.on_time || incoming_frequency != incomingMessage.frequency){
    incoming_on_time = incomingMessage.on_time;
    incoming_frequency = incomingMessage.frequency;
    update_data();
    outgoingMessage.message = counter++;
  }
  
}

void update_data() {
  //New data is received
  timerStop(Skip_Timer);                //Pause the timer which skips AC cycles
  detachInterrupt(zcd.PIN);             //De-attach ZCD interrupt to prevent excution of ISR
  digitalWrite(ENABLE_PIN, ENABLE_LOW); //Disable the gate drivers

  DUTY_CYCLE_US = incoming_on_time;
  SKIP_TIME_US = incoming_frequency * time_half_wave_us + error_margin_us;

  if (DUTY_CYCLE_US != 0) {
    timerStart(Skip_Timer);              //Start the timer which skips AC cycles
    attachInterrupt(zcd.PIN, Zcd_ISR, TRIGGER);  //AC signal is RISING
  }

  //Write the new values to timers
  timerAlarmWrite(Duty_Cycle_Timer, DUTY_CYCLE_US, false);    //Gate Drivers are disabled after this time
  timerAlarmWrite(Skip_Timer, SKIP_TIME_US, false);   //After this time is finished ZCD interrupt is enabled again
}