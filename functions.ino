void callback_iobroker(String topic, String payload) {
  if (topic.equals("myhome/controller/Reset")) {
    if (payload == "true") {
      mqtt.publish("myhome/controller/Reset", "false");
      wdt_enable(WDTO_15MS);
      for (;;) {}
    }
  }
  //////////////////////////
  else if (topic.equals("myhome/controller/PWM_1")) {
    int pwm = PWM(payload);
    analogWrite(PWM_1, pwm);
    mqtt.publish("myhome/controller/PWM_1", IntToChar(255 - pwm));
  }
  //////////////////////////
  else if (topic.equals("myhome/controller/PWM_2")) {
    int pwm = PWM(payload);
    analogWrite(PWM_2, pwm);
    mqtt.publish("myhome/controller/PWM_2", IntToChar(255 - pwm));
  }
  //////////////////////////
  else if (topic.equals("myhome/controller/SSR_1")) {
    digitalWrite(SSR_1, SrtToLvl(payload));
    mqtt.publish("myhome/controller/SSR_1", pub(payload));
  }
  //////////////////////////
  else if (topic.equals("myhome/controller/SSR_2")) {
    digitalWrite(SSR_2, SrtToLvl(payload));
    mqtt.publish("myhome/controller/SSR_2", pub(payload));
  }
  //////////////////////////
  else if (topic.equals("myhome/controller/RELAY_1")) {
    digitalWrite(RELAY_1, SrtToLvl(payload));
    mqtt.publish("myhome/controller/RELAY_1", pub(payload));
  }
  //////////////////////////
  else if (topic.equals("myhome/controller/RELAY_2")) {
    digitalWrite(RELAY_2, SrtToLvl(payload));
    mqtt.publish("myhome/controller/RELAY_2", pub(payload));
  }
  //////////////////////////
}

void AnalogRead() { //http://arduino.ru/forum/programmirovanie/usrednenie-znachenii-s-potentsiometra
  int Ain[4] = {analogRead(AIN_1), analogRead(AIN_2), analogRead (AIN_3), analogRead (AIN_4)};
  if (Ain[0] == 1 || (Ain[0] == 2 && old_Ain[0] == 0)) {
    Ain[0] = 0;
  }
  if (Ain[1] == 1 || (Ain[1] == 2 && old_Ain[1] == 0)) {
    Ain[1] = 0;
  }
  if (Ain[2] == 1 || (Ain[2] == 2 && old_Ain[2] == 0)) {
    Ain[2] = 0;
  }
  if (Ain[3] == 1 || (Ain[3] == 2 && old_Ain[3] == 0)) {
    Ain[3] = 0;
  }
  if (Ain[0] - 2 == old_Ain[0]) {
    Ain[0] = old_Ain[0];
  }
  if (Ain[1] - 2 == old_Ain[1]) {
    Ain[1] = old_Ain[1];
  }
  if (Ain[2] - 2 == old_Ain[2]) {
    Ain[2] = old_Ain[2];
  }
  if (Ain[3] - 2 == old_Ain[3]) {
    Ain[4] = old_Ain[3];
  }

  if (((old_Ain[0] != Ain[0] || old_Ain[1] != Ain[1] || old_Ain[2] != Ain[2] || old_Ain[3] != Ain[3]) && millis() - prevMillis2 > 1000) || firststart) {
    prevMillis2 = millis();
    old_Ain[0] = Ain[0];
    old_Ain[1] = Ain[1];
    old_Ain[2] = Ain[2];
    old_Ain[3] = Ain[3];
    for (int i = 0; i < 4; i++) {
      String t = "myhome/controller/AIN_";
      t += i + 1;
      const char *q = t.c_str();
      mqtt.publish(q, IntToChar(Ain[i]));
    }
  }
  
  firststart = false;
}

/*
  void DinRead() {
  int Din_1 = digitalRead(DIN_1);
  int Din_2 = digitalRead(DIN_2);
  if (((old_din_1 != Din_1 || old_din_2 != Din_2) && millis() - prevMillis3 > 500) || firststart) {
    prevMillis3 = millis();
    old_din_1 = Din_1;
    old_din_2 = Din_2;
    mqtt.publish("myhome/controller/DIN_1", BoolToChar(Din_1));
    mqtt.publish("myhome/controller/DIN_2", BoolToChar(Din_2));
    firststart = false;
  }
  }
*/

void ReadDS18() {
  DS_sensors.requestTemperatures();
  Temp1 = DS_sensors.getTempC(addr_T1);
  if (Temp1 > 0) {
    mqtt.publish("myhome/controller/Temperature_1", FloatToChar(Temp1));
  }
}

void ReadDHT() {
  if (DHT.read22(DHT22_PIN) == DHTLIB_OK) {
    Hout = DHT.humidity;
    Tout = DHT.temperature;
    if (Hout != 0 || Tout != 0) {
      mqtt.publish("myhome/controller/DHT_humidity", FloatToChar(Hout));
      mqtt.publish("myhome/controller/DHT_temperature", FloatToChar(Tout));
    }
  }
}

void ReadButton () {
  if (firststart) {
    for (int i = 0; i <= 15; i++) {
      btn_old[i] = mcp.digitalRead(bt[i]);
      delay(100);
      Switch(i);
    }
  } else {
    for (int i = 0; i <= 15; i++) {
      btn[i] = mcp.digitalRead(bt[i]);
      if (btn[i] != btn_old[i]) {
        //if (millis() - prevMillisPoll > 100){
        //prevMillisPoll = millis();
        btn_old[i] = btn[i];
        Switch(i);
        //}
      }
    }
  }
}

void Switch(int i) {
  String t = "myhome/controller/IN_";
  t += i + 1;
  const char *q = t.c_str();
  mqtt.publish(q, state(btn_old[i]));
}

const char* state(int num) {
  if (num > 0) {
    return "false";
  } else {
    return "true";
  }
}

bool SrtToLvl(String st) {
  if (st == "false" || st == "0" || st == "off") {
    return 0;
  } else {
    return 1;
  }
}
const char* pub(String st) {
  if (st == "false" || st == "0" || st == "off") {
    return "false";
  } else {
    return "true";
  }
}

const char* BoolToChar (bool r) {
  return r ? "true" : "false";
}

const char* IntToChar (unsigned int v) {
  sprintf(buffer, "%d", v);
  return buffer;
}

const char* FloatToChar (float f) {
  sprintf(buffer, "%d.%02d", (int)f, (int)(f * 100) % 100);
  return buffer;
}

int PWM(String p) {
  pwm = p.toInt();
  if (pwm > 254) {
    pwm = 255;
  } else if (pwm < 1) {
    pwm = 0;
  }
  pwm = 255 - pwm;
  return pwm;
}
