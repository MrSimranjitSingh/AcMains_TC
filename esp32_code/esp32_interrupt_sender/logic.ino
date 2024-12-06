void rotary_loop() {

  if (RE_1->encoderChanged() || RE_2->encoderChanged()) {
    update_data();
  }

  bool re1 = RE_1->isEncoderButtonDown();
  bool re2 = RE_2->isEncoderButtonDown();

  if (re1) {
    //numberRE1Selector.setValue(default_RE_DC);
    //numberRE2Selector.setValue(default_RE_Skip);
    //update_data();
    RE_1->enable();
    RE_2->enable();
  }

  if (re2) {
    //numberRE1Selector.setValue(RE_1_MAX);
    //numberRE2Selector.setValue(RE_2_MIN - RE_2_STEP);
    //update_data();
    RE_1->disable();
    RE_2->disable();
  }
}

void update_data() {
  u8x8.clear();
  u8x8.setCursor(0,0);

  //Full 60Hz signal > time = 16.66ms
  //For 50% 60Hz signal > time = 8.33ms (50% duty cycle)
  //For 30% duty cycle > time = 4.998ms

  float time_ms = (1.0 / AC_MAINS_FREQ) * 1000.0;              //Time in milliseconds for AC Mains frequency
  time_ms = time_ms * (numberRE1Selector.getValue() / 100.0);  //Time in milliseconds with duty cycle

  String s = "DC: " + String(numberRE1Selector.getValue(), 0) + "% / " + String(time_ms, 1) + "ms";
  const char* foo = s.c_str();
  u8x8.drawString(0, 0, foo);

  float freq = (2 * AC_MAINS_FREQ) / (numberRE2Selector.getValue() + 1);
  s = "Freq: " + String(freq, 2) + " Hz";
  foo = s.c_str();
  u8x8.drawString(0, 2, foo);

  // Set values to send
  outgoingMessage.on_time = time_ms * 1000.0;
  outgoingMessage.frequency = numberRE2Selector.getValue();
}


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet send status:\t");

  /*String s;
  if(status == ESP_NOW_SEND_SUCCESS){
    s = "Dlvr: ok   ";
  }else{
    s = "Dlvr: fail";
  }
  
  const char* foo = s.c_str();
  u8x8.drawString(0, 6, foo);*/
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  incoming_message = incomingMessage.message;

  //char buf[10];
  //sprintf(buf, "%3d", incoming_message);
  //8x8.drawString(0, 7, buf);

  //String s = "Recv: " + String(buf);
  //const char* foo = s.c_str();
  //u8x8.drawString(0, 7, foo);
}