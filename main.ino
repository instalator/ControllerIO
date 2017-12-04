#include <SPI.h>           // Ethernet shield
#include <Ethernet2.h>     // Ethernet shield
#include <PubSubClient.h>  // MQTT 
#include <Adafruit_MCP23017.h>
#include <avr/wdt.h>
//#include <EEPROM.h>

#define RELAY_1  9 //Релейный выход 1
#define RELAY_2  4 //Релейный выход 2
#define PWM_1    6 //ШИМ Выход 1
#define PWM_2    5 //ШИМ Выход 1
#define SSR_1    A2 //Выход твердотельное реле 1
#define SSR_2    A3 //Выход твердотельное реле 2

#define MCP_INTA 3
#define DIN_1    8 //Дискретный вход GPIO
#define DIN_2    7 //Дискретный вход GPIO
#define AIN_1    A1 //Аналоговый U 0-10В вход 1
#define AIN_2    A0 //Аналоговый U 0-10В вход 2
#define AIN_3    A6 //Аналоговый U 0-10В / I 0-20мА вход 3
#define AIN_4    A7 //Аналоговый U 0-10В / I 0-20мА вход 4

#define ID_CONNECT "CONTROLLER"

Adafruit_MCP23017 mcp;
int count = 0;
bool btn[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool btn_old[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool firststart = true;
unsigned long prevMillis = 0; //для reconnect
unsigned long prevMillisPoll = 0;
unsigned long prevMillis2 = 0;
unsigned long prevMillis3 = 0;

static char buf [100];
int pwm;
const byte bt[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};


byte mac[] = { 0xC0, 0x44, 0x70, 0x11, 0xEE, 0x65 }; //MAC адрес контроллера
IPAddress server(192, 168, 1, 190); //IP MQTT брокера
IPAddress ip(192, 168, 1, 61);      //IP контроллера

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String strTopic = String(topic);
    String strPayload = String((char*)payload);
    callback_iobroker(strTopic, strPayload);
}

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void reconnect() {
    count++;
    wdt_reset();
    if (client.connect(ID_CONNECT)) {
      count = 0;
      wdt_reset();
      client.publish("myhome/controller/connection", "true");
      PubTopic();
      client.subscribe("myhome/controller/#");
    }
    for(int i = 0; i <= 15; i++){
      btn[i] = mcp.digitalRead(bt[i]);
      if (btn[i] == 1){
        count = 0;
      }
    }
    if (count > 50){
      wdt_enable(WDTO_15MS);
        for(;;){}
    }
}

void setup() {
  MCUSR = 0;
  wdt_disable();
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(PWM_1, OUTPUT);
  pinMode(PWM_2, OUTPUT);
  pinMode(SSR_1, OUTPUT);
  pinMode(SSR_2, OUTPUT);
  pinMode(MCP_INTA, INPUT);
  pinMode(DIN_1, INPUT);
  pinMode(DIN_2, INPUT);
  pinMode(AIN_1, INPUT);
  pinMode(AIN_2, INPUT);

  analogWrite(PWM_1, 255);
  analogWrite(PWM_2, 255);
  
  mcp.begin();
  delay(100);
  for(int i = 0; i <= 15; i++){
    mcp.pinMode(i, INPUT);
  }
  Ethernet.begin(mac, ip);
  delay(100);
  wdt_enable(WDTO_8S);
  //Serial.print("START");

}

void loop() {
   wdt_reset();
   client.loop();
   if (!client.connected()){
     if (millis() - prevMillis > 10000){
        prevMillis = millis();
        reconnect();
     }
   }   
   ReadButton();
   AnalogRead();
   DinRead();
}

void PubTopic (){
    char s[16];
    sprintf(s, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    client.publish("myhome/controller/ip", s);
    client.publish("myhome/controller/AIN_1", "0");
    client.publish("myhome/controller/AIN_2", "0");
    client.publish("myhome/controller/AIN_3", "0");
    client.publish("myhome/controller/AIN_4", "0");
    client.publish("myhome/controller/DIN_1", "false");
    client.publish("myhome/controller/DIN_2", "false");
    client.publish("myhome/controller/RELAY_1", "false");
    client.publish("myhome/controller/RELAY_2", "false");
    client.publish("myhome/controller/PWM_1", "0");
    client.publish("myhome/controller/PWM_2", "0");
    client.publish("myhome/controller/SSR_1", "false");
    client.publish("myhome/controller/SSR_2", "false");
    client.publish("myhome/controller/Reset", "false");
}
