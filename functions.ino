void callback_iobroker(String strTopic, String strPayload) {
  if (strTopic == "myhome/controller/Reset") {
      if(strPayload == "true"){
        client.publish("myhome/controller/Reset", "false");
        wdt_enable(WDTO_15MS);
        for(;;){}
      }
  }
  //////////////////////////
  else if (strTopic == "myhome/controller/PWM_1") {
      int pwm = PWM(strPayload);
      analogWrite(PWM_1, pwm);
      client.publish("myhome/controller/PWM_1", IntToChar(255 - pwm));
  }
  //////////////////////////
  else if (strTopic == "myhome/controller/PWM_2") {
      int pwm = PWM(strPayload);
      analogWrite(PWM_2, pwm);
      client.publish("myhome/controller/PWM_2", IntToChar(255 - pwm));
  }
  //////////////////////////
  else if (strTopic == "myhome/controller/SSR_1") {
      digitalWrite(SSR_1, SrtToLvl(strPayload));
      client.publish("myhome/controller/SSR_1", pub(strPayload));
  }
  //////////////////////////
  else if (strTopic == "myhome/controller/SSR_2") {
      digitalWrite(SSR_2, SrtToLvl(strPayload));
      client.publish("myhome/controller/SSR_2", pub(strPayload));
  }
  //////////////////////////
  else if (strTopic == "myhome/controller/RELAY_1") {
      digitalWrite(RELAY_1, SrtToLvl(strPayload));
      client.publish("myhome/controller/RELAY_1", pub(strPayload));
  }
  //////////////////////////
  else if (strTopic == "myhome/controller/RELAY_2") {
      digitalWrite(RELAY_2, SrtToLvl(strPayload));
      client.publish("myhome/controller/RELAY_2", pub(strPayload));
  }
  //////////////////////////
}

void AnalogRead(){
  unsigned int Ain_1 = analogRead(AIN_1);
  unsigned int Ain_2 = analogRead(AIN_2);
  unsigned int Ain_3 = analogRead (AIN_3);
  unsigned int Ain_4 = analogRead (AIN_4);

  if(Ain_1 == 1){Ain_1 = 0;}
  if(Ain_2 == 1){Ain_2 = 0;}
  if(Ain_3 == 1){Ain_3 = 0;}
  if(Ain_4 == 1){Ain_4 = 0;}
  if (((old_Ain_1 != Ain_1 || old_Ain_2 != Ain_2 || old_Ain_3 != Ain_3 || old_Ain_4 != Ain_4) && millis() - prevMillis2 > 1000) || firststart){
    prevMillis2 = millis();
    old_Ain_1 = Ain_1;
    old_Ain_2 = Ain_2;
    old_Ain_3 = Ain_3;
    old_Ain_4 = Ain_4;
    client.publish("myhome/controller/AIN_1", IntToChar(Ain_1));
    client.publish("myhome/controller/AIN_2", IntToChar(Ain_2));
    client.publish("myhome/controller/AIN_3", IntToChar(Ain_3));
    client.publish("myhome/controller/AIN_4", IntToChar(Ain_4));
  }
}

void DinRead(){
  int Din_1 = digitalRead(DIN_1);
  int Din_2 = digitalRead(DIN_2);
  if (((old_din_1 != Din_1 || old_din_2 != Din_2) && millis() - prevMillis3 > 500) || firststart){
    prevMillis3 = millis();
    old_din_1 = Din_1;
    old_din_2 = Din_2;
    client.publish("myhome/controller/DIN_1", BoolToChar(Din_1));
    client.publish("myhome/controller/DIN_2", BoolToChar(Din_2));
    firststart = false;
  }
}


void ReadButton (){
  if (firststart){
    for(int i = 0; i <= 15; i++){
      btn_old[i] = mcp.digitalRead(bt[i]);
      delay(100);
      Switch(i);
    }
  } else {
    for(int i = 0; i <= 15; i++){
      btn[i] = mcp.digitalRead(bt[i]);
      if (btn[i] != btn_old[i]){
        //if (millis() - prevMillisPoll > 100){
          //prevMillisPoll = millis();
          btn_old[i] = btn[i];
          Switch(i);
       //}
      }
    }
  }
}

void Switch(int i){
  switch (i){
    case 0:
        client.publish("myhome/controller/IN_1", state(btn_old[i]));
      break;
    case 1:
        client.publish("myhome/controller/IN_2", state(btn_old[i]));
      break;
    case 2:
        client.publish("myhome/controller/IN_3", state(btn_old[i]));
      break;
    case 3:
        client.publish("myhome/controller/IN_4", state(btn_old[i]));
      break;
    case 4:
        client.publish("myhome/controller/IN_5", state(btn_old[i]));
      break;
    case 5:
        client.publish("myhome/controller/IN_6", state(btn_old[i]));
      break;
    case 6:
        client.publish("myhome/controller/IN_7", state(btn_old[i]));
      break;
    case 7:
        client.publish("myhome/controller/IN_8", state(btn_old[i]));
      break;
    case 8:
        client.publish("myhome/controller/IN_9", state(btn_old[i]));
      break;
    case 9:
        client.publish("myhome/controller/IN_10", state(btn_old[i]));
      break;
    case 10:
        client.publish("myhome/controller/IN_11", state(btn_old[i]));
      break;
    case 11:
        client.publish("myhome/controller/IN_12", state(btn_old[i]));
      break;
    case 12:
        client.publish("myhome/controller/IN_13", state(btn_old[i]));
      break;
    case 13:
        client.publish("myhome/controller/IN_14", state(btn_old[i]));
      break;
    case 14:
        client.publish("myhome/controller/IN_15", state(btn_old[i]));
      break;
    case 15:
        client.publish("myhome/controller/IN_16", state(btn_old[i]));
      break;
    //default:
  }
}

const char* state(int num){
    if (num > 0){
      return "false";
    } else {
      return "true";
    }
}

bool SrtToLvl(String st){
    if (st == "false" || st == "0" || st == "off"){
      return 0;
    } else {
      return 1;
    }
}
const char* pub(String st){
    if (st == "false" || st == "0" || st == "off"){
      return "false";
    } else{
      return "true";
    }
}

const char* BoolToChar (bool r) {
    return r ? "true" : "false";
}

const char* IntToChar (unsigned int v) {
  sprintf(buf, "%d", v);
  return buf;
}

int PWM(String p){
    pwm = p.toInt();
    if (pwm > 254){
      pwm = 255;
    } else if (pwm < 1){
      pwm = 0;
    }
    pwm = 255 - pwm;
    return pwm;
}
