#include <SPI.h>           // Ethernet shield
#include <Ethernet2.h>     // Ethernet shield
#include <PubSubClient.h>  // MQTT 
#include <Adafruit_MCP23017.h>
#include <avr/wdt.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht.h>
#include <TextFinder.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

#define RELAY_1      9  //Релейный выход 1
#define RELAY_2      4  //Релейный выход 2
#define PWM_1        6  //ШИМ Выход 1
#define PWM_2        5  //ШИМ Выход 1
#define SSR_1        16 //Выход твердотельное реле 1
#define SSR_2        17 //Выход твердотельное реле 2
#define MCP_INTA     3  //MCP_INT
#define DIN_1        8  //Дискретный вход GPIO
#define DHT22_PIN    8 // Датчик DHT22
#define DIN_2        7  //Дискретный вход GPIO
#define ONE_WIRE_BUS 7 // Датчик DS18B20
#define AIN_1        A1 //Аналоговый U 0-10В вход 1
#define AIN_2        A0 //Аналоговый U 0-10В вход 2
#define AIN_3        A6 //Аналоговый U 0-10В / I 0-20мА вход 3
#define AIN_4        A7 //Аналоговый U 0-10В / I 0-20мА вход 4

#define ID_CONNECT "controller"
#define PRE_TOPIC "myhome/controller/"

Adafruit_MCP23017 mcp;
dht DHT;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS_sensors(&oneWire);
DeviceAddress addr_T1 = { 0x28, 0xFF, 0xE3, 0x66, 0x53, 0x15, 0x02, 0x75 };

int count = 0;
bool btn[16]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0};
bool btn_old[16]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0};
const byte bt[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
unsigned long prevMillis = 0; //для reconnect
unsigned long prevMillisPoll = 0;
unsigned long prevMillis2 = 0;
unsigned long prevMillis3 = 0;
//unsigned long prevMillis4 = 0;
bool firststart = true;
bool old_din_1;
bool old_din_2;
int old_Ain[4] = {0, 0, 0, 0};
//static char buf [50];
int pwm;
float Temp1 = 0;
float Hout = 0;
float Tout = 0;
const byte ID = 0x91;
char buffer[100];
const byte averageFactor = 5;


byte mac[] = { 0xC0, 0x44, 0x71, 0x11, 0xEE, 0x65 }; //MAC адрес контроллера
byte mqtt_serv[] = {192, 168, 88, 190}; //IP MQTT брокера

const char htmlx0[] PROGMEM = "<html><title>Controller IO setup Page</title><body marginwidth=\"0\" marginheight=\"0\" ";
const char htmlx1[] PROGMEM = "leftmargin=\"0\" \"><table bgcolor=\"#999999\" border";
const char htmlx2[] PROGMEM = "=\"0\" width=\"100%\" cellpadding=\"1\" ";
const char htmlx3[] PROGMEM = "\"><tr><td>&nbsp Controller IO setup Page</td></tr></table><br>";
const char* const string_table0[] PROGMEM = {htmlx0, htmlx1, htmlx2, htmlx3};

const char htmla0[] PROGMEM = "<script>function hex2num (s_hex) {eval(\"var n_num=0X\" + s_hex);return n_num;}";
const char htmla1[] PROGMEM = "</script><table><form><input type=\"hidden\" name=\"SBM\" value=\"1\"><tr><td>MAC:&nbsp&nbsp&nbsp";
const char htmla2[] PROGMEM = "<input id=\"T1\" type=\"text\" size=\"1\" maxlength=\"2\" name=\"DT1\" value=\"";
const char htmla3[] PROGMEM = "\">.<input id=\"T3\" type=\"text\" size=\"1\" maxlength=\"2\" name=\"DT2\" value=\"";
const char htmla4[] PROGMEM = "\">.<input id=\"T5\" type=\"text\" size=\"1\" maxlength=\"2\" name=\"DT3\" value=\"";
const char htmla5[] PROGMEM = "\">.<input id=\"T7\" type=\"text\" size=\"1\" maxlength=\"2\" name=\"DT4\" value=\"";
const char htmla6[] PROGMEM = "\">.<input id=\"T9\" type=\"text\" size=\"1\" maxlength=\"2\" name=\"DT5\" value=\"";
const char htmla7[] PROGMEM = "\">.<input id=\"T11\" type=\"text\" size=\"1\" maxlength=\"2\" name=\"DT6\" value=\"";
const char* const string_table1[] PROGMEM = {htmla0, htmla1, htmla2, htmla3, htmla4, htmla5, htmla6, htmla7};

const char htmlb0[] PROGMEM = "\"><input id=\"T2\" type=\"hidden\" name=\"DT1\"><input id=\"T4\" type=\"hidden\" name=\"DT2";
const char htmlb1[] PROGMEM = "\"><input id=\"T6\" type=\"hidden\" name=\"DT3\"><input id=\"T8\" type=\"hidden\" name=\"DT4";
const char htmlb2[] PROGMEM = "\"><input id=\"T10\" type=\"hidden\" name=\"DT5\"><input id=\"T12\" type=\"hidden\" name=\"D";
const char htmlb3[] PROGMEM = "T6\"></td></tr><tr><td>MQTT: <input type=\"text\" size=\"1\" maxlength=\"3\" name=\"DT7\" value=\"";
const char htmlb4[] PROGMEM = "\">.<input type=\"text\" size=\"1\" maxlength=\"3\" name=\"DT8\" value=\"";
const char htmlb5[] PROGMEM = "\">.<input type=\"text\" size=\"1\" maxlength=\"3\" name=\"DT9\" value=\"";
const char htmlb6[] PROGMEM = "\">.<input type=\"text\" size=\"1\" maxlength=\"3\" name=\"DT10\" value=\"";
const char* const string_table2[] PROGMEM = {htmlb0, htmlb1, htmlb2, htmlb3, htmlb4, htmlb5, htmlb6};

const char htmlc0[] PROGMEM = "\"></td></tr><tr><td><br></td></tr><tr><td><input id=\"button1\"type=\"submit\" value=\"SAVE\" ";
const char htmlc1[] PROGMEM = "></td></tr></form></table></body></html>";
const char* const string_table3[] PROGMEM = {htmlc0, htmlc1};

const char htmld0[] PROGMEM = "Onclick=\"document.getElementById('T2').value ";
const char htmld1[] PROGMEM = "= hex2num(document.getElementById('T1').value);";
const char htmld2[] PROGMEM = "document.getElementById('T4').value = hex2num(document.getElementById('T3').value);";
const char htmld3[] PROGMEM = "document.getElementById('T6').value = hex2num(document.getElementById('T5').value);";
const char htmld4[] PROGMEM = "document.getElementById('T8').value = hex2num(document.getElementById('T7').value);";
const char htmld5[] PROGMEM = "document.getElementById('T10').value = hex2num(document.getElementById('T9').value);";
const char htmld6[] PROGMEM = "document.getElementById('T12').value = hex2num(document.getElementById('T11').value);\"";
const char* const string_table4[] PROGMEM = {htmld0, htmld1, htmld2, htmld3, htmld4, htmld5, htmld6};

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  callback_iobroker(strTopic, strPayload);
}

EthernetClient ethClient;
EthernetServer http_server(80);
PubSubClient mqtt(ethClient);

void reconnect() {
  count++;
  wdt_reset();
  if (mqtt.connect(ID_CONNECT)) {
    count = 0;
    wdt_reset();
    mqtt.publish("myhome/controller/connection", "true");
    PubTopic();
    mqtt.subscribe("myhome/controller/#");
  }
  /*
  for (int i = 0; i <= 15; i++) { //непонятно для чего добавлял, пока закоментил
    btn[i] = mcp.digitalRead(bt[i]);
    if (btn[i] == 1) {
      count = 0;
    }
  }
  */
  if (count > 50) {
    wdt_enable(WDTO_15MS);
    for (;;) {}
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
  delay(1000);
  for (int i = 0; i <= 15; i++) {
    mcp.pinMode(i, INPUT);
  }
  httpSetup();
  mqttSetup();
  delay(1000);
  wdt_enable(WDTO_8S);
}

void loop() {
  wdt_reset();
  //checkHttp();
  mqtt.loop();
  if (!mqtt.connected()) {
    if (millis() - prevMillis > 10000) {
      prevMillis = millis();
      if (Ethernet.begin(mac) == 0) {
        Reset();
      } else {
        reconnect();
      }
    }
  } else {
    ReadButton();
    AnalogRead();
    //DinRead(); // Читаем Дискретные входы !Камент так как на этих портах датчики температуры.
    if (millis() - prevMillis > 5000) {
      prevMillis = millis();
      ReadDHT();
      ReadDS18();
      mqtt.publish("myhome/controller/SRAM", IntToChar(availableMemory()));
    }
  }
}

int availableMemory(){
        int size = 2048;
        byte *buf;
        while ((buf = (byte *) malloc(--size)) == NULL);
        free(buf);
        return size;
      }

void mqttSetup() {
  int idcheck = EEPROM.read(0);
  if (idcheck == ID) {
    for (int i = 0; i < 4; i++) {
      mqtt_serv[i] = EEPROM.read(i + 7);
    }
  }
  mqtt.setServer(mqtt_serv, 1883);
  mqtt.setCallback(callback);
}

void httpSetup() {
  int idcheck = EEPROM.read(0);
  if (idcheck == ID) {
    for (int i = 0; i < 6; i++) {
      mac[i] = EEPROM.read(i + 1);
    }
  }
  Ethernet.begin(mac);
}

void PubTopic () {
  char s[16];
  sprintf(s, "%d.%d.%d.%d", Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
  mqtt.publish("myhome/controller/ip", s);
  mqtt.publish("myhome/controller/RELAY_1", "false");
  mqtt.publish("myhome/controller/RELAY_2", "false");
  mqtt.publish("myhome/controller/PWM_1", "0");
  mqtt.publish("myhome/controller/PWM_2", "0");
  mqtt.publish("myhome/controller/SSR_1", "false");
  mqtt.publish("myhome/controller/SSR_2", "false");
  mqtt.publish("myhome/controller/Reset", "false");
}

void checkHttp() {
  EthernetClient http = http_server.available();
  if (http) {
    TextFinder  finder(http );
    while (http.connected()) {
      if (http.available()) {
        if ( finder.find("GET /") ) {
          if (finder.findUntil("setup", "\n\r")) {
            if (finder.findUntil("SBM", "\n\r")) {
              byte SET = finder.getValue();
              while (finder.findUntil("DT", "\n\r")) {
                int val = finder.getValue();
                if (val >= 1 && val <= 6) {
                  mac[val - 1] = finder.getValue();
                }
                if (val >= 7 && val <= 10) {
                  mqtt_serv[val - 7] = finder.getValue();
                }
              }
              for (int i = 0 ; i < 6; i++) {
                EEPROM.write(i + 1, mac[i]);
              }
              for (int i = 0 ; i < 4; i++) {
                EEPROM.write(i + 7, mqtt_serv[i]);
              }
              EEPROM.write(0, ID);
              http.println("HTTP/1.1 200 OK");
              http.println("Content-Type: text/html");
              http.println();
              for (int i = 0; i < 4; i++) {
                strcpy_P(buffer, (char*)pgm_read_word(&(string_table0[i])));
                http.print( buffer );
              }
              http.println();
              http.print("Saved!");
              http.println();
              http.print("Restart");
              for (int i = 1; i < 10; i++) {
                http.print(".");
                delay(500);
              }
              http.println("OK");
              Reset(); // ребутим с новыми параметрами
            }
            http.println("HTTP/1.1 200 OK");
            http.println("Content-Type: text/html");
            http.println();
            for (int i = 0; i < 4; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table0[i])));
              http.print( buffer );
            }
            for (int i = 0; i < 3; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[i])));
              http.print( buffer );
            }
            http.print(mac[0], HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[3])));
            http.print( buffer );
            http.print(mac[1], HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[4])));
            http.print( buffer );
            http.print(mac[2], HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[5])));
            http.print( buffer );
            http.print(mac[3], HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[6])));
            http.print( buffer );
            http.print(mac[4], HEX);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table1[7])));
            http.print( buffer );
            http.print(mac[5], HEX);
            for (int i = 0; i < 4; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[i])));
              http.print( buffer );
            }
            http.print(mqtt_serv[0], DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[4])));
            http.print( buffer );
            http.print(mqtt_serv[1], DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[5])));
            http.print( buffer );
            http.print(mqtt_serv[2], DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table2[6])));
            http.print( buffer );
            http.print(mqtt_serv[3], DEC);
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[0])));
            http.print( buffer );
            for (int i = 0; i < 7; i++) {
              strcpy_P(buffer, (char*)pgm_read_word(&(string_table4[i])));
              http.print( buffer );
            }
            strcpy_P(buffer, (char*)pgm_read_word(&(string_table3[1])));
            http.print( buffer );
            break;
          }
        }
        http.println("HTTP/1.1 200 OK");
        http.println("Content-Type: text/html");
        http.println();
        http.print("IOT controller [");
        http.print(ID_CONNECT);
        http.print("]: go to <a href=\"/setup\"> setup</a>");
        break;
      }
    }
    delay(1);
    http.stop();
  } else {
    return;
  }
}

void Reset() {
  for (;;) {}
}
