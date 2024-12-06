void setup_encoders() {
  //we must initialize rotary encoder
  RE_1->begin();
  RE_1->setup(RE_1_ISR);
  RE_1->setBoundaries(RE_1_MIN, RE_1_MAX, false);  //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
  numberRE1Selector.attachEncoder(RE_1);
  numberRE1Selector.setRange(RE_1_MIN, RE_1_MAX, RE_1_STEP, false, 0);
  numberRE1Selector.setValue(default_RE_DC);

  RE_2->begin();
  RE_2->setup(RE_2_ISR);
  RE_2->setBoundaries(RE_2_MIN, RE_2_MAX, false);  //Frequency limit Hz
  numberRE2Selector.attachEncoder(RE_2);
  numberRE2Selector.setRange(RE_2_MIN, RE_2_MAX, RE_2_STEP, false, 0);
  numberRE2Selector.setValue(default_RE_Skip);

  RE_1->disableAcceleration();  //acceleration is now enabled by default
  RE_2->setAcceleration(200);
}

void setup_lcd() {

  while(!u8x8.begin());

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 0, "Hello :)");
}