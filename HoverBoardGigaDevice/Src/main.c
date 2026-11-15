#ifdef BUTTON
  uint32_t iTimePushed = millis();
  #ifdef REMOTE_ADC
    extern int16_t iConfigMode;
  #endif
  while (BUTTON_PUSHED == digitalRead(BUTTON))
  {
    fwdgt_counter_reload();
    #ifdef REMOTE_ADC
      if (millis() - iTimePushed > 2000)
      {
        iConfigMode = 1;
        BuzzerSet(8, 2);
      }
    #endif
  }
  Delay(10);
#endif
