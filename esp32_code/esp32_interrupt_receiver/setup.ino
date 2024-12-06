void setup_timers() {
  if (DUTY_CYCLE_US != 0) {
    attachInterrupt(zcd.PIN, Zcd_ISR, TRIGGER);  //AC signal is RISING
  }

  Duty_Cycle_Timer = timerBegin(0, 80, true);             // use ESP32 Timer 0, pre-scale 80 (for 80MHz freq), count up.
  timerAttachInterrupt(Duty_Cycle_Timer, &Duty_Cycle_ISR, true);      // attach the function to call when timer interrupt fires. Edge.
  Skip_Timer = timerBegin(1, 80, true);                   // use ESP32 Timer 1, pre-scale 80 (for 80MHz freq), count up.
  timerAttachInterrupt(Skip_Timer, &Skip_ISR, true);  // attach the function to call when timer interrupt fires. Edge.
  Zcd_Delay_Timer = timerBegin(2, 80, true);
  timerAttachInterrupt(Zcd_Delay_Timer, &Zcd_Delay_ISR, true);

  timerAlarmWrite(Zcd_Delay_Timer, DELAY_TIME_US, false);
  timerAlarmWrite(Duty_Cycle_Timer, DUTY_CYCLE_US, false);
  timerAlarmWrite(Skip_Timer, SKIP_TIME_US, false);
}


void setup_espnow() {
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}