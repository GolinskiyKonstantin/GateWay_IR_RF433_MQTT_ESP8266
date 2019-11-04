/*
    После заливки прошивки первый раз включаем модуль и ждем примерно минуту, так как настройки отсудствуют модуль перейдет в режим точки доступа.
    Ищем  точку ( сеть ) Wi-Fi с указаным именем и паролем в настройках кода по умолчанию подключаемся к точке.
    В браузере заходим по адресу 192.168.4.1 вводим настройки и перегружаем модуль, после етого он будет работать согласно настройкам.
*/
/** Для возможности прошивки по сетевому порту OTA,
 *  необходимо установить последнюю версию python 
 *  Скачать по ссылке: https://www.python.org/downloads/
 *  Для коректной работы OTA надо чтоб в имени устройства отсутствовали такие "_" знаки. Пример ArduinoOTA.setHostname("gydota_esp8266_01") - так нельзя
    ArduinoOTA.setHostname("gydota-esp8266-01") - надо так﻿
**/

#include <pgmspace.h>   // Используем для PROGMEM - говорит компилятору «разместить информацию во flash-памяти», а не в SRAM, 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>
#include <SimpleTimer.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>  // Нужна тоже для ОТА
#include <ArduinoOTA.h> // Библиотека для OTA-прошивки

#include <RCSwitch.h>             // https://github.com/sui77/rc-switch/
#include <IRremoteESP8266.h>      // https://github.com/markszabo/IRremoteESP8266
#include <IRrecv.h>
#include <RCSwitch.h>
#include <IRutils.h>


#define IRpin                   13  // D7     // Порт куда подключен ИК приемник
#define RF433pin                4   // D2     // Порт куда подключен Радиомодуль 433
#define LedWiFiConect           15  // D8     // Светодиод соединения WiFi
#define LedMqttConect           14  // D5     // Светодиод соединения MQTT
#define LedReceiver_IR_433      12  // D6     // Светодиод моргает когда считал сигнал по ИК или 433


//***********************************************************************
// для того чтобы в сериал монитор выводилась информация нужно разкоментировать ниже строчку #define TestSerialPrint
//#define TestSerialPrint
    // #ifdef TestSerialPrint
    // Код который тут написан будет компелироваться при #define TestSerialPrint
    // #endif
//**********************************************************************    
// 192.168.4.1 Заходим по этому адресу когда в режиме точки доступа

RCSwitch mySwitch = RCSwitch();
IRrecv irrecv(IRpin);
uint32_t codeNow = 0;

const char* const ssidAP PROGMEM = "ESP_GateWay";     // Имя сети в режиме точка доступа
const char* const passwordAP PROGMEM = "1234567890";  // Пароль сети в режиме точка доступа

const uint32_t timeoutWiFi = 60L*1000L; // В течении указанного времени пытаемся соеденится с сетью если не получилось запускается режим точки доступа
const uint32_t timeoutMQTT = 30L*1000L; // Время попытки переподключения MQTT
const uint32_t APtoSTAreconnect = 3L*60L*1000L;    // Время когда в режиме точки доступа переодически пробивает наличие сети чтобы к ней подключиться

const char configSign[4] PROGMEM = { '#', 'E', 'S', 'P' };  // Сигнатура для начала чтения памяти если начало есть то читаем все настройки если нету то не читаем.
const byte maxStrParamLength = 32;    // Максимальная длина задаваемых параметров настроек

          const char* const ssidArg PROGMEM = "ssid";
          const char* const passwordArg PROGMEM = "password";
          const char* const domainArg PROGMEM = "domain";
          const char* const serverArg PROGMEM = "server";
          const char* const portArg PROGMEM = "port";
          const char* const userArg PROGMEM = "user";
          const char* const devName PROGMEM = "name";
          const char* const hostName PROGMEM = "host";
          const char* const mqttpswdArg PROGMEM = "mqttpswd";
          const char* const clientArg PROGMEM = "client";
          const char* const rebootArg PROGMEM = "reboot";
          const char* const wifimodeArg PROGMEM = "wifimode";
          const char* const wifiRssi PROGMEM = "wifirssi";
          const char* const mqttconnectedArg PROGMEM = "mqttconnected";
          const char* const uptimeArg PROGMEM = "uptime";

          const char* const codeArg PROGMEM = "codemode";

          const char* const code1 PROGMEM = "code1";
          const char* const topic1 PROGMEM = "topic1";
          const char* const value1 PROGMEM = "value1";

          const char* const code2 PROGMEM = "code2";
          const char* const topic2 PROGMEM = "topic2";
          const char* const value2 PROGMEM = "value2";

          const char* const code3 PROGMEM = "code3";
          const char* const topic3 PROGMEM = "topic3";
          const char* const value3 PROGMEM = "value3";

          const char* const code4 PROGMEM = "code4";
          const char* const topic4 PROGMEM = "topic4";
          const char* const value4 PROGMEM = "value4";

          const char* const code5 PROGMEM = "code5";
          const char* const topic5 PROGMEM = "topic5";
          const char* const value5 PROGMEM = "value5";

          const char* const code6 PROGMEM = "code6";
          const char* const topic6 PROGMEM = "topic6";
          const char* const value6 PROGMEM = "value6";

          const char* const code7 PROGMEM = "code7";
          const char* const topic7 PROGMEM = "topic7";
          const char* const value7 PROGMEM = "value7";

          const char* const code8 PROGMEM = "code8";
          const char* const topic8 PROGMEM = "topic8";
          const char* const value8 PROGMEM = "value8";

          const char* const code9 PROGMEM = "code9";
          const char* const topic9 PROGMEM = "topic9";
          const char* const value9 PROGMEM = "value9";

          const char* const code10 PROGMEM = "code10";
          const char* const topic10 PROGMEM = "topic10";
          const char* const value10 PROGMEM = "value10";

          const char* const code11 PROGMEM = "code11";
          const char* const topic11 PROGMEM = "topic11";
          const char* const value11 PROGMEM = "value11";

          const char* const code12 PROGMEM = "code12";
          const char* const topic12 PROGMEM = "topic12";
          const char* const value12 PROGMEM = "value12";

          const char* const code13 PROGMEM = "code13";
          const char* const topic13 PROGMEM = "topic13";
          const char* const value13 PROGMEM = "value13";

          const char* const code14 PROGMEM = "code14";
          const char* const topic14 PROGMEM = "topic14";
          const char* const value14 PROGMEM = "value14";

          const char* const code15 PROGMEM = "code15";
          const char* const topic15 PROGMEM = "topic15";
          const char* const value15 PROGMEM = "value15";

          const char* const code16 PROGMEM = "code16";
          const char* const topic16 PROGMEM = "topic16";
          const char* const value16 PROGMEM = "value16";

          const char* const code17 PROGMEM = "code17";
          const char* const topic17 PROGMEM = "topic17";
          const char* const value17 PROGMEM = "value17";

          const char* const code18 PROGMEM = "code18";
          const char* const topic18 PROGMEM = "topic18";
          const char* const value18 PROGMEM = "value18";

          const char* const code19 PROGMEM = "code19";
          const char* const topic19 PROGMEM = "topic19";
          const char* const value19 PROGMEM = "value19";

          const char* const code20 PROGMEM = "code20";
          const char* const topic20 PROGMEM = "topic20";
          const char* const value20 PROGMEM = "value20";

          
String nameDevice = "ESP GateWay", aHostname = "ESP8266-GateWay";  // Указываем параметры по умолчанию
String ssid, password, domain;

int32_t rssi = 0;


String mqttServer, mqttUser, mqttPassword, mqttClient = "GateWay";

        String mqttTopic1 = "/Topic_1", mqttTopic2 = "/Topic_2", mqttTopic3 = "/Topic_3", mqttTopic4 = "/Topic_4", mqttTopic5 = "/Topic_5", mqttTopic6 = "/Topic_6", mqttTopic7 = "/Topic_7", mqttTopic8 = "/Topic_8", mqttTopic9 = "/Topic_9", mqttTopic10 = "/Topic_10"; // Указываем параметры по умолчанию
        String mqttTopic11 = "/Topic_11", mqttTopic12 = "/Topic_12", mqttTopic13 = "/Topic_13", mqttTopic14 = "/Topic_14", mqttTopic15 = "/Topic_15", mqttTopic16 = "/Topic_16", mqttTopic17 = "/Topic_17", mqttTopic18 = "/Topic_18", mqttTopic19 = "/Topic_19", mqttTopic20 = "/Topic_20";
        
        String mqttCode1 = "code1", mqttCode2 = "code2", mqttCode3 = "code3", mqttCode4 = "code4", mqttCode5 = "code5", mqttCode6 = "code6", mqttCode7 = "code7", mqttCode8 = "code8", mqttCode9 = "code9", mqttCode10 = "code10";
        String mqttCode11 = "code11", mqttCode12 = "code12", mqttCode13 = "code13", mqttCode14 = "code14", mqttCode15 = "code15", mqttCode16 = "code16", mqttCode17 = "code17", mqttCode18 = "code18", mqttCode19 = "code19", mqttCode20 = "code20";
        
        String mqttValue1 = "mqttValue1", mqttValue2 = "mqttValue2", mqttValue3 = "mqttValue3", mqttValue4 = "mqttValue4", mqttValue5 = "mqttValue5", mqttValue6 = "mqttValue6", mqttValue7 = "mqttValue7", mqttValue8 = "mqttValue8", mqttValue9 = "mqttValue9", mqttValue10 = "mqttValue10";
        String mqttValue11 = "mqttValue11", mqttValue12 = "mqttValue12", mqttValue13 = "mqttValue13", mqttValue14 = "mqttValue14", mqttValue15 = "mqttValue15", mqttValue16 = "mqttValue16", mqttValue17 = "mqttValue17", mqttValue18 = "mqttValue18", mqttValue19 = "mqttValue19", mqttValue20 = "mqttValue20";
                             
uint16_t mqttPort = 1883;   // Указываем порт по умолчанию



ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;

PubSubClient pubsubClient(espClient);
SimpleTimer timer;
SimpleTimer timerRssi;
SimpleTimer timerUptime;  // Таймер такт времени прошедшего от запуска

uint32_t timeValue = 0;  

// Переменная для хранения номера таймера
uint16_t timerId;

//===== Данные для авторизации на WEB странице ====================================================
String strAdminName = "admin";
String adminPassword = "PaSSword";

bool adminAuthenticate() {
  if (adminPassword.length()) {
    if (! httpServer.authenticate(String (strAdminName).c_str(), adminPassword.c_str())) {
      httpServer.requestAuthentication();
      return false;
    }
  }
  return true;
}
//=================================================================================================

// Функция отображения времени от старта включения модуля
String timeStart(unsigned long t){
                          
    unsigned long d = t/24/60/60;
    unsigned long h = (t/3600)%24;
    String H = h<10? "0" + String(h) + "h ": String(h) + "h ";
    unsigned long m = (t/60)%60;
    String M = m<10? "0" + String(m) + "m ": String(m) + "m " ;
    unsigned long s = t%60;
    String S = s<10? "0" + String(s) + "s": String(s) + "s";
                        
    return String(d) + "d " + H + M + S;
}

//** EEPROM configuration functions*******************************************

// Фукнкция чтения данных из памяти*****************************
uint16_t readEEPROMString(uint16_t offset, String& str) {
  char buffer[maxStrParamLength + 1];

  buffer[maxStrParamLength] = 0;
  for (uint16_t i = 0; i < maxStrParamLength; i++) {
    if (! (buffer[i] = EEPROM.read(offset + i)))
      break;
  }
  str = String(buffer);

  return offset + maxStrParamLength;
}

// Функция записи данных в память*********************
uint16_t writeEEPROMString(uint16_t offset, const String& str) {
  for (uint16_t i = 0; i < maxStrParamLength; i++) {
    if (i < str.length())
      EEPROM.write(offset + i, str[i]);
    else
      EEPROM.write(offset + i, 0);
  }

  return offset + maxStrParamLength;
}

// Функция чтения конфигураций из памяти*******************
bool readConfig() {
  uint16_t offset = 0;
  #ifdef TestSerialPrint
  Serial.println(F("Reading config from EEPROM"));
  #endif
  for (uint16_t i = 0; i < sizeof(configSign); i++) {
    char c = pgm_read_byte(configSign + i);
    if (EEPROM.read(offset + i) != c)
      return false;
  }
  // При чтении каждого элемента увеличивается значение offset ( содержит адрес конца считываного одного элемента тем самым знаем где начинать считыватьследующий элемент )
  //** считываем  строковые данные *****************
  offset += sizeof(configSign);
  offset = readEEPROMString(offset, ssid);
  offset = readEEPROMString(offset, password);
  offset = readEEPROMString(offset, domain);
  offset = readEEPROMString(offset, mqttServer);
  offset = readEEPROMString(offset, nameDevice);
  offset = readEEPROMString(offset, aHostname);
  //**************************************************

  //** считываем  целочисленные данные *****************
  EEPROM.get(offset, mqttPort);
  offset += sizeof(mqttPort);
  //****************************************************

  //** считываем строковые данные *****************
  offset = readEEPROMString(offset, mqttUser);
  offset = readEEPROMString(offset, mqttPassword);
  offset = readEEPROMString(offset, mqttClient);
  
            offset = readEEPROMString(offset, mqttCode1);
            offset = readEEPROMString(offset, mqttTopic1);
            offset = readEEPROMString(offset, mqttValue1);

            offset = readEEPROMString(offset, mqttCode2);
            offset = readEEPROMString(offset, mqttTopic2);
            offset = readEEPROMString(offset, mqttValue2);

            offset = readEEPROMString(offset, mqttCode3);
            offset = readEEPROMString(offset, mqttTopic3);
            offset = readEEPROMString(offset, mqttValue3);

            offset = readEEPROMString(offset, mqttCode4);
            offset = readEEPROMString(offset, mqttTopic4);
            offset = readEEPROMString(offset, mqttValue4);

            offset = readEEPROMString(offset, mqttCode5);
            offset = readEEPROMString(offset, mqttTopic5);
            offset = readEEPROMString(offset, mqttValue5);

            offset = readEEPROMString(offset, mqttCode6);
            offset = readEEPROMString(offset, mqttTopic6);
            offset = readEEPROMString(offset, mqttValue6);

            offset = readEEPROMString(offset, mqttCode7);
            offset = readEEPROMString(offset, mqttTopic7);
            offset = readEEPROMString(offset, mqttValue7);

            offset = readEEPROMString(offset, mqttCode8);
            offset = readEEPROMString(offset, mqttTopic8);
            offset = readEEPROMString(offset, mqttValue8);

            offset = readEEPROMString(offset, mqttCode9);
            offset = readEEPROMString(offset, mqttTopic9);
            offset = readEEPROMString(offset, mqttValue9);

            offset = readEEPROMString(offset, mqttCode10);
            offset = readEEPROMString(offset, mqttTopic10);
            offset = readEEPROMString(offset, mqttValue10);

            offset = readEEPROMString(offset, mqttCode11);
            offset = readEEPROMString(offset, mqttTopic11);
            offset = readEEPROMString(offset, mqttValue11);

            offset = readEEPROMString(offset, mqttCode12);
            offset = readEEPROMString(offset, mqttTopic12);
            offset = readEEPROMString(offset, mqttValue12);

            offset = readEEPROMString(offset, mqttCode13);
            offset = readEEPROMString(offset, mqttTopic13);
            offset = readEEPROMString(offset, mqttValue13);

            offset = readEEPROMString(offset, mqttCode14);
            offset = readEEPROMString(offset, mqttTopic14);
            offset = readEEPROMString(offset, mqttValue14);

            offset = readEEPROMString(offset, mqttCode15);
            offset = readEEPROMString(offset, mqttTopic15);
            offset = readEEPROMString(offset, mqttValue15);

            offset = readEEPROMString(offset, mqttCode16);
            offset = readEEPROMString(offset, mqttTopic16);
            offset = readEEPROMString(offset, mqttValue16);

            offset = readEEPROMString(offset, mqttCode17);
            offset = readEEPROMString(offset, mqttTopic17);
            offset = readEEPROMString(offset, mqttValue17);

            offset = readEEPROMString(offset, mqttCode18);
            offset = readEEPROMString(offset, mqttTopic18);
            offset = readEEPROMString(offset, mqttValue18);

            offset = readEEPROMString(offset, mqttCode19);
            offset = readEEPROMString(offset, mqttTopic19);
            offset = readEEPROMString(offset, mqttValue19);

            offset = readEEPROMString(offset, mqttCode20);
            offset = readEEPROMString(offset, mqttTopic20);
            offset = readEEPROMString(offset, mqttValue20);
  //********************************************

  return true;
}

// Функция записи конфигураций в память****************
void writeConfig() {
  uint16_t offset = 0;
  #ifdef TestSerialPrint
  Serial.println(F("Writing config to EEPROM"));
  #endif
  for (uint16_t i = 0; i < sizeof(configSign); i++) {
    char c = pgm_read_byte(configSign + i);
    EEPROM.write(offset + i, c);
  }
  // При записи каждого элемента увеличивается значение offset ( содержит адрес конца считываного одного элемента тем самым знаем где начинать считыватьследующий элемент )
  //** записываем строковые данные *****************
  offset += sizeof(configSign);
  offset = writeEEPROMString(offset, ssid);
  offset = writeEEPROMString(offset, password);
  offset = writeEEPROMString(offset, domain);
  offset = writeEEPROMString(offset, mqttServer);
  offset = writeEEPROMString(offset, nameDevice);
  offset = writeEEPROMString(offset, aHostname);
  //*********************************************************

  //** записываем целочисленные данные *****************
  EEPROM.put(offset, mqttPort);
  offset += sizeof(mqttPort);
  //******************************************************
  
  //** записываем строковые данные *****************
  offset = writeEEPROMString(offset, mqttUser);
  offset = writeEEPROMString(offset, mqttPassword);
  offset = writeEEPROMString(offset, mqttClient);

          offset = writeEEPROMString(offset, mqttCode1);
          offset = writeEEPROMString(offset, mqttTopic1);
          offset = writeEEPROMString(offset, mqttValue1);
        
          offset = writeEEPROMString(offset, mqttCode2);
          offset = writeEEPROMString(offset, mqttTopic2);
          offset = writeEEPROMString(offset, mqttValue2);

          offset = writeEEPROMString(offset, mqttCode3);
          offset = writeEEPROMString(offset, mqttTopic3);
          offset = writeEEPROMString(offset, mqttValue3);

          offset = writeEEPROMString(offset, mqttCode4);
          offset = writeEEPROMString(offset, mqttTopic4);
          offset = writeEEPROMString(offset, mqttValue4);

          offset = writeEEPROMString(offset, mqttCode5);
          offset = writeEEPROMString(offset, mqttTopic5);
          offset = writeEEPROMString(offset, mqttValue5);

          offset = writeEEPROMString(offset, mqttCode6);
          offset = writeEEPROMString(offset, mqttTopic6);
          offset = writeEEPROMString(offset, mqttValue6);

          offset = writeEEPROMString(offset, mqttCode7);
          offset = writeEEPROMString(offset, mqttTopic7);
          offset = writeEEPROMString(offset, mqttValue7);

          offset = writeEEPROMString(offset, mqttCode8);
          offset = writeEEPROMString(offset, mqttTopic8);
          offset = writeEEPROMString(offset, mqttValue8);

          offset = writeEEPROMString(offset, mqttCode9);
          offset = writeEEPROMString(offset, mqttTopic9);
          offset = writeEEPROMString(offset, mqttValue9);

          offset = writeEEPROMString(offset, mqttCode10);
          offset = writeEEPROMString(offset, mqttTopic10);
          offset = writeEEPROMString(offset, mqttValue10);

          offset = writeEEPROMString(offset, mqttCode11);
          offset = writeEEPROMString(offset, mqttTopic11);
          offset = writeEEPROMString(offset, mqttValue11);
        
          offset = writeEEPROMString(offset, mqttCode12);
          offset = writeEEPROMString(offset, mqttTopic12);
          offset = writeEEPROMString(offset, mqttValue12);

          offset = writeEEPROMString(offset, mqttCode13);
          offset = writeEEPROMString(offset, mqttTopic13);
          offset = writeEEPROMString(offset, mqttValue13);

          offset = writeEEPROMString(offset, mqttCode14);
          offset = writeEEPROMString(offset, mqttTopic14);
          offset = writeEEPROMString(offset, mqttValue14);

          offset = writeEEPROMString(offset, mqttCode15);
          offset = writeEEPROMString(offset, mqttTopic15);
          offset = writeEEPROMString(offset, mqttValue15);

          offset = writeEEPROMString(offset, mqttCode16);
          offset = writeEEPROMString(offset, mqttTopic16);
          offset = writeEEPROMString(offset, mqttValue16);

          offset = writeEEPROMString(offset, mqttCode17);
          offset = writeEEPROMString(offset, mqttTopic17);
          offset = writeEEPROMString(offset, mqttValue17);

          offset = writeEEPROMString(offset, mqttCode18);
          offset = writeEEPROMString(offset, mqttTopic18);
          offset = writeEEPROMString(offset, mqttValue18);

          offset = writeEEPROMString(offset, mqttCode19);
          offset = writeEEPROMString(offset, mqttTopic19);
          offset = writeEEPROMString(offset, mqttValue19);

          offset = writeEEPROMString(offset, mqttCode20);
          offset = writeEEPROMString(offset, mqttTopic20);
          offset = writeEEPROMString(offset, mqttValue20);
  //**************************************************

  EEPROM.commit();
}
//***********************************************************************************


//*********** Настройки WiFi**********************************

// Настройка подключения как клиент к роутеру*****************************
bool setupWiFiAsStation() {

  uint32_t maxtime = millis() + timeoutWiFi;
  
  #ifdef TestSerialPrint
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  #endif
  
  WiFi.mode(WIFI_STA);    // Принудительно переводим в режим клиента
  WiFi.begin(ssid.c_str(), password.c_str());   // Подключаем к сети

  while (WiFi.status() != WL_CONNECTED) {   // Проверяем статус соеденения
   // digitalWrite(pinBuiltinLed, LOW);
    delay(500);
   // digitalWrite(pinBuiltinLed, HIGH);
    #ifdef TestSerialPrint
    Serial.print(".");
    #endif
    if (millis() >= maxtime) {
      #ifdef TestSerialPrint
      Serial.println(F(" fail!")); 
      #endif
      return false;
    }
  }
  
  digitalWrite(LedWiFiConect, HIGH);
  #ifdef TestSerialPrint
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  #endif
  timer.restartTimer(timerId); // Заставляет указанный таймер начинать отсчет с «сейчас» (Перезапуск таймера), то есть момента, когда вызывается restartTimer. Таймер Обратный вызов не запускается. 
  timer.disable(timerId); // Выключаем таймер по Id.
  
  
  return true;
}

// Настройка сети как точка доступа*********************************
void setupWiFiAsAP() {
  #ifdef TestSerialPrint
  Serial.print(F("Configuring access point "));
  Serial.println(ssidAP);
  #endif
  digitalWrite(LedWiFiConect, LOW);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidAP, passwordAP);
  // Если хотим сделать скрытую сеть тв режиме точка доступа то ниже приведен пример такого подключения
  // ssid, password, channel, hide
  // WiFi.softAP(ssidAP, passwordAP, 1, 1);
  #ifdef TestSerialPrint
  Serial.print(F("IP address: "));
  Serial.println(WiFi.softAPIP());
  #endif
}

// Настройки Wi-Fi **********************************************
void setupWiFi() {
  if ((! ssid.length()) || (! setupWiFiAsStation())){  // Если имя сети не указано или неможем подключится к сети то  запускаем в режиме точки доступа.
    setupWiFiAsAP();
  }
  
  if (domain.length()) {  // Если определен домен мднс то попвтка его зарегестрировать.
    if (MDNS.begin(domain.c_str())) {
      MDNS.addService("http", "tcp", 80);
      #ifdef TestSerialPrint
      Serial.println(F("mDNS responder started"));
      #endif
    } else {
      #ifdef TestSerialPrint
      Serial.println(F("Error setting up mDNS responder!"));
      #endif
    }
  }

  httpServer.begin();   // Запускаем сервер
  #ifdef TestSerialPrint
  Serial.println(F("HTTP server started (==========================)"));
  #endif
}
//*******************************************************************************



//**** Настройки HTTP server functions*********************************************
 

String quoteEscape(const String& str) {
  String result = "";
  int16_t start = 0, pos;

  while (start < str.length()) {
    pos = str.indexOf('"', start);
    if (pos != -1) {
      result += str.substring(start, pos) + F("&quot;");
      start = pos + 1;
    } else {
      result += str.substring(start);
      break;
    }
  }

  return result;
}

// Главная страница *****************************************************

void handleRoot() {

 //=== Проверка авторизвции на WEB странице ==========
  if (! adminAuthenticate()) return;
 //===================================================
 
  String message =
F("<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <title>ESP8266</title>\n\
  <script type=\"text/javascript\">\n\
    function openUrl(url) {\n\
      var request = new XMLHttpRequest();\n\
      request.open('GET', url, true);\n\
      request.send(null);\n\
    }\n\
    function refreshData() {\n\
      var request = new XMLHttpRequest();\n\
      request.open('GET', '/data', true);\n\
      request.onreadystatechange = function() {\n\
        if (request.readyState == 4) {\n\
          var data = JSON.parse(request.responseText);\n\
                                          document.getElementById('");
                                  message += FPSTR(codeArg);
                                  message += F("').innerHTML = data.");
                                  message += FPSTR(codeArg);
                                  message += F(";\n\
          document.getElementById('");
  message += FPSTR(wifimodeArg);
  message += F("').innerHTML = data.");
  message += FPSTR(wifimodeArg);
  message += F(";\n\
          document.getElementById('");
  message += FPSTR(wifiRssi);
  message += F("').innerHTML = data.");
  message += FPSTR(wifiRssi);
  message += F(";\n\
          document.getElementById('");
  message += FPSTR(mqttconnectedArg);
  message += F("').innerHTML = (data.");
  message += FPSTR(mqttconnectedArg);
  message += F(" != true ? \"not \" : \"\") + \"connected\";\n\
          document.getElementById('");
  message += FPSTR(uptimeArg);
  message += F("').innerHTML = data.");
  message += FPSTR(uptimeArg);
  message += F(";\n\
        }\n\
      }\n\
      request.send(null);\n\
    }\n\
    setInterval(refreshData, 500);\n\
  </script>\n\
</head>\n\
<body>\n\
  <form>\n\
    <h3>"); 
  message += nameDevice;
  message += F("</h3>\n\
    <p>\n\
                                              Code: <span id=\"");
                                            message += FPSTR(codeArg);
                                            message += F("\">?</span><br/>\n\
    WiFi mode: <span id=\"");
  message += FPSTR(wifimodeArg);
  message += F("\">?</span><br/>\n\
    WiFi Connection: <span id=\"");
  message += FPSTR(wifiRssi);
  message += F("\"> ?</span> dBm<br/>\n\
    MQTT broker: <span id=\"");
  message += FPSTR(mqttconnectedArg);
  message += F("\">?</span><br/>\n\
    Uptime: <span id=\"");
  message += FPSTR(uptimeArg);
  message += F("\">0</span></p>\n\
    <p>\n\
    <input type=\"button\" value=\"WiFi Setup\" onclick=\"location.href='/wifi';\" />\n\
    <input type=\"button\" value=\"MQTT Setup\" onclick=\"location.href='/mqtt';\" />\n\
    <input type=\"button\" value=\"Device name\" onclick=\"location.href='/name';\" />\n\
    <input type=\"button\" value=\"Reboot!\" onclick=\"if (confirm('Are you sure to reboot?')) location.href='/reboot';\" />\n\
  </form>\n\
</body>\n\
</html>");

  httpServer.send(200, F("text/html"), message);
}


// Страница настройки Wi-Fi *****************************************************************

void handleWiFiConfig() {

 //=== Проверка авторизвции на WEB странице ==========
  if (! adminAuthenticate()) return;
 //===================================================
 
  String message =
F("<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <title>WiFi Setup</title>\n\
</head>\n\
<body>\n\
  <form name=\"wifi\" method=\"get\" action=\"/store\">\n\
    <h3>WiFi Setup</h3>\n\
    SSID:<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(ssidArg);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(ssid);
  message += F("\" />\n\
    <br/>\n\
    Password:<br/>\n\
    <input type=\"password\" name=\"");
  message += FPSTR(passwordArg);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(password);
  message += F("\" />\n\
    <br/>\n\
    mDNS domain:<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(domainArg);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(domain);
  message += F("\" />\n\
    .local (leave blank to ignore mDNS)\n\
    <p>\n\
    <input type=\"submit\" value=\"Save\" />\n\
    <input type=\"hidden\" name=\"");
  message += FPSTR(rebootArg);
  message += F("\" value=\"1\" />\n\
  </form>\n\
</body>\n\
</html>");

  httpServer.send(200, F("text/html"), message);
}

// Страница настройки имени устройства ************************************************

void deviceName(){

 //=== Проверка авторизвции на WEB странице ==========
  if (! adminAuthenticate()) return;
 //===================================================
 
    String message =
F("<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <title>Device name</title>\n\
</head>\n\
<body>\n\
  <form name=\"name\" method=\"get\" action=\"/store\">\n\
    <h3>Device name</h3>\n\
    Device name:<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(devName);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(nameDevice);
  message += F("\" />\n\ 
    <br/>\n\
    Host name (device name on the network):<br/>(For OTA to work, you can not use the character \"_\" in the name)<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(hostName);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(aHostname);
  message += F("\" />\n\         
    <p>\n\
    <input type=\"submit\" value=\"Save\" />\n\
    <input type=\"hidden\" name=\"");
  message += FPSTR(rebootArg);
  message += F("\" value=\"0\" />\n\
  </form>\n\
</body>\n\
</html>");

  httpServer.send(200, F("text/html"), message);
}
// Страница настройки MQTT ******************************************************************

void handleMQTTConfig() {

 //=== Проверка авторизвции на WEB странице ==========
  if (! adminAuthenticate()) return;
 //===================================================
 
  String message =
F("<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <title>MQTT Setup</title>\n\
</head>\n\
<body>\n\
  <form name=\"mqtt\" method=\"get\" action=\"/store\">\n\
    <h3>MQTT Setup</h3>\n\
    Server:<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(serverArg);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(mqttServer);
  message += F("\" onchange=\"document.mqtt.reboot.value=1;\" />\n\
    (leave blank to ignore MQTT)\n\
    <br/>\n\
    Port (default 1883):<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(portArg);
  message += F("\" maxlength=5 value=\"");
  message += String(mqttPort);
  message += F("\" onchange=\"document.mqtt.reboot.value=1;\" />\n\
    <br/>\n\
    User (if authorization is required on MQTT server):<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(userArg);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(mqttUser);
  message += F("\" />\n\
    (leave blank to ignore MQTT authorization)\n\
    <br/>\n\
    Password:<br/>\n\
    <input type=\"password\" name=\"");
  message += FPSTR(mqttpswdArg);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(mqttPassword);
  message += F("\" />\n\
    (leave blank to ignore MQTT authorization)\n\
    <br/>\n\
    Client (device name):<br/>\n\
    <input type=\"text\" name=\"");
  message += FPSTR(clientArg);
  message += F("\" maxlength=");
  message += String(maxStrParamLength);
  message += F(" value=\"");
  message += quoteEscape(mqttClient);
  message += F("\" />\n\
    <br/>\n\
    <p>\n\
    <p>");
                                message += F("<h3>CODE: ");
                                message +=String(codeNow);
                                message += F("</h3><p>\n\
                                Code 1: ======================================<br/>\n\
                                Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code1);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode1);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic1);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic1);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value1);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue1);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\                 
                                Code 2: ======================================<br/>\n\
                                Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code2);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode2);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic2);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic2);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value2);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue2);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 3: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code3);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode3);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic3);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic3);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value3);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue3);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 4: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code4);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode4);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic4);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic4);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value4);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue4);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 5: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code5);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode5);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic5);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic5);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value5);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue5);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 6: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code6);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode6);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic6);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic6);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value6);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue6);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 7: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code7);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode7);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic7);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic7);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value7);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue7);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 8: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code8);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode8);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic8);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic8);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value8);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue8);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 9: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code9);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode9);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic9);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic9);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value9);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue9);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 10: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code10);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode10);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic10);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic10);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value10);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue10);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\  
                                  Code 11: ======================================<br/>\n\
                                Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code11);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode11);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic11);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic11);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value11);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue11);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\                 
                                Code 12: ======================================<br/>\n\
                                Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code12);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode12);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic12);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic12);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value12);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue12);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 13: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code13);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode13);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic13);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic13);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value13);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue13);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 14: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code14);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode14);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic14);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic14);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value14);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue14);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 15: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code15);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode15);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic15);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic15);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value15);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue15);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 16: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code16);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode16);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic16);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic16);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value16);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue16);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 17: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code17);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode17);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic17);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic17);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value17);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue17);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 18: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code18);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode18);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic18);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic18);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value18);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue18);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 19: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code19);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode19);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic19);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic19);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value19);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue19);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
                                  Code 20: ======================================<br/>\n\
                                  Code: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(code20);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttCode20);
                              message += F("\" /> \n\
                                Topic:(format => /Topic/Value): \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(topic20);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttTopic20);
                              message += F("\" /> \n\
                                Value: \n\
                                <input type=\"text\" name=\"");
                              message += FPSTR(value20);
                              message += F("\" maxlength=");
                              message += String(maxStrParamLength);
                              message += F(" value=\"");
                              message += quoteEscape(mqttValue20);
                              message += F("\" />\n\
                                  <br/>\n\
                                  <p>\n\
    <input type=\"submit\" value=\"Save\" />\n\
    <input type=\"hidden\" name=\"");
  message += FPSTR(rebootArg);
  message += F("\" value=\"0\" />\n\
  </form>\n\
</body>\n\
</html>");

  httpServer.send(200, F("text/html"), message);
}

// Считываем в переменные если заполнились или изменились поля и нажали кнопку сохронить*****************************

void handleStoreConfig() {
  String argName, argValue;
  #ifdef TestSerialPrint
  Serial.print(F("/store("));
  #endif
  for (uint16_t i = 0; i < httpServer.args(); i++) {
    #ifdef TestSerialPrint
    if (i){
      Serial.print(F(", "));
    }
    #endif
    argName = httpServer.argName(i);
    #ifdef TestSerialPrint
    Serial.print(argName);
    Serial.print(F("=\""));
    #endif
    argValue = httpServer.arg(i);
    #ifdef TestSerialPrint
    Serial.print(argValue);
    Serial.print(F("\""));
    #endif
//**************************************************************************************************************************************************
    if      (argName.equals(FPSTR(ssidArg))) { ssid = argValue; } 
    else if (argName.equals(FPSTR(passwordArg))) { password = argValue; } 
    else if (argName.equals(FPSTR(domainArg))) { domain = argValue;} 
    else if (argName.equals(FPSTR(serverArg))) {mqttServer = argValue;} 
    else if (argName.equals(FPSTR(portArg))) {mqttPort = argValue.toInt();} 
    else if (argName.equals(FPSTR(userArg))) {mqttUser = argValue;} 
    else if (argName.equals(FPSTR(devName))) {nameDevice = argValue;} 
    else if (argName.equals(FPSTR(hostName))) {aHostname = argValue;} 
    else if (argName.equals(FPSTR(mqttpswdArg))) {mqttPassword = argValue;} 
    else if (argName.equals(FPSTR(clientArg))) {mqttClient = argValue;} 
    
            else if (argName.equals(FPSTR(code1))) {mqttCode1 = argValue;} 
            else if (argName.equals(FPSTR(topic1))) {mqttTopic1 = argValue;} 
            else if (argName.equals(FPSTR(value1))) {mqttValue1 = argValue;} 

            else if (argName.equals(FPSTR(code2))) {mqttCode2 = argValue;} 
            else if (argName.equals(FPSTR(topic2))) {mqttTopic2 = argValue;} 
            else if (argName.equals(FPSTR(value2))) {mqttValue2 = argValue;} 

            else if (argName.equals(FPSTR(code3))) {mqttCode3 = argValue;} 
            else if (argName.equals(FPSTR(topic3))) {mqttTopic3 = argValue;} 
            else if (argName.equals(FPSTR(value3))) {mqttValue3 = argValue;} 

            else if (argName.equals(FPSTR(code4))) {mqttCode4 = argValue;} 
            else if (argName.equals(FPSTR(topic4))) {mqttTopic4 = argValue;} 
            else if (argName.equals(FPSTR(value4))) {mqttValue4 = argValue;} 

            else if (argName.equals(FPSTR(code5))) {mqttCode5 = argValue;} 
            else if (argName.equals(FPSTR(topic5))) {mqttTopic5 = argValue;} 
            else if (argName.equals(FPSTR(value5))) {mqttValue5 = argValue;} 

            else if (argName.equals(FPSTR(code6))) {mqttCode6 = argValue;} 
            else if (argName.equals(FPSTR(topic6))) {mqttTopic6 = argValue;} 
            else if (argName.equals(FPSTR(value6))) {mqttValue6 = argValue;} 

            else if (argName.equals(FPSTR(code7))) {mqttCode7 = argValue;} 
            else if (argName.equals(FPSTR(topic7))) {mqttTopic7 = argValue;} 
            else if (argName.equals(FPSTR(value7))) {mqttValue7 = argValue;} 

            else if (argName.equals(FPSTR(code8))) {mqttCode8 = argValue;} 
            else if (argName.equals(FPSTR(topic8))) {mqttTopic8 = argValue;} 
            else if (argName.equals(FPSTR(value8))) {mqttValue8 = argValue;} 

            else if (argName.equals(FPSTR(code9))) {mqttCode9 = argValue;} 
            else if (argName.equals(FPSTR(topic9))) {mqttTopic9 = argValue;} 
            else if (argName.equals(FPSTR(value9))) {mqttValue9 = argValue;} 

            else if (argName.equals(FPSTR(code10))) {mqttCode10 = argValue;} 
            else if (argName.equals(FPSTR(topic10))) {mqttTopic10 = argValue;} 
            else if (argName.equals(FPSTR(value10))) {mqttValue10 = argValue;} 

            else if (argName.equals(FPSTR(code11))) {mqttCode11 = argValue;} 
            else if (argName.equals(FPSTR(topic11))) {mqttTopic11 = argValue;} 
            else if (argName.equals(FPSTR(value11))) {mqttValue11 = argValue;} 

            else if (argName.equals(FPSTR(code12))) {mqttCode12 = argValue;} 
            else if (argName.equals(FPSTR(topic12))) {mqttTopic12 = argValue;} 
            else if (argName.equals(FPSTR(value12))) {mqttValue12 = argValue;} 

            else if (argName.equals(FPSTR(code13))) {mqttCode13 = argValue;} 
            else if (argName.equals(FPSTR(topic13))) {mqttTopic13 = argValue;} 
            else if (argName.equals(FPSTR(value13))) {mqttValue13 = argValue;} 

            else if (argName.equals(FPSTR(code14))) {mqttCode14 = argValue;} 
            else if (argName.equals(FPSTR(topic14))) {mqttTopic14 = argValue;} 
            else if (argName.equals(FPSTR(value14))) {mqttValue14 = argValue;} 

            else if (argName.equals(FPSTR(code15))) {mqttCode15 = argValue;} 
            else if (argName.equals(FPSTR(topic15))) {mqttTopic15 = argValue;} 
            else if (argName.equals(FPSTR(value15))) {mqttValue15 = argValue;} 

            else if (argName.equals(FPSTR(code16))) {mqttCode16 = argValue;} 
            else if (argName.equals(FPSTR(topic16))) {mqttTopic16 = argValue;} 
            else if (argName.equals(FPSTR(value16))) {mqttValue16 = argValue;} 

            else if (argName.equals(FPSTR(code17))) {mqttCode17 = argValue;} 
            else if (argName.equals(FPSTR(topic17))) {mqttTopic17 = argValue;} 
            else if (argName.equals(FPSTR(value17))) {mqttValue17 = argValue;} 

            else if (argName.equals(FPSTR(code18))) {mqttCode18 = argValue;} 
            else if (argName.equals(FPSTR(topic18))) {mqttTopic18 = argValue;} 
            else if (argName.equals(FPSTR(value18))) {mqttValue18 = argValue;} 

            else if (argName.equals(FPSTR(code19))) {mqttCode19 = argValue;} 
            else if (argName.equals(FPSTR(topic19))) {mqttTopic19 = argValue;} 
            else if (argName.equals(FPSTR(value19))) {mqttValue19 = argValue;} 

            else if (argName.equals(FPSTR(code20))) {mqttCode20 = argValue;} 
            else if (argName.equals(FPSTR(topic20))) {mqttTopic20 = argValue;} 
            else if (argName.equals(FPSTR(value20))) {mqttValue20 = argValue;}

  }
  #ifdef TestSerialPrint
  Serial.println(F(")"));
  #endif

  writeConfig(); // Сохраняем данные в память

// Сообщение о том что все сохранено и через 5 сек возвращаемся на главную страницу 

  String message =
F("<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <title>Store Setup</title>\n\
  <meta http-equiv=\"refresh\" content=\"5; /index.html\">\n\
</head>\n\
<body>\n\
  Configuration stored successfully.\n");
  if (httpServer.arg(rebootArg) == "1")
    message += F("  <br/>\n\
  <i>You must reboot module to apply new configuration!</i>\n");
  message += F("  <p>\n\
  Wait for 5 sec. or click <a href=\"/index.html\">this</a> to return to main page.\n\
</body>\n\
</html>");

  httpServer.send(200, F("text/html"), message);
}

//  проверяем нажатие перезагрузка. **************************************************

void handleReboot() {

 //=== Проверка авторизвции на WEB странице ==========
  if (! adminAuthenticate()) return;
 //===================================================
  #ifdef TestSerialPrint
  Serial.println(F("/reboot()"));
  #endif

  String message =
F("<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <title>Rebooting</title>\n\
  <meta http-equiv=\"refresh\" content=\"5; /index.html\">\n\
</head>\n\
<body>\n\
  Rebooting...\n\
</body>\n\
</html>");

  httpServer.send(200, F("text/html"), message);

  ESP.restart();
}

// Обновление данных на главной странице ****************************************

void handleData() {
  String message = F("{\"");
                    message += FPSTR(codeArg);
                    message += F("\":\"");
                    message += String(codeNow);
                    message += F("\",\"");
  message += FPSTR(wifimodeArg);
  message += F("\":\"");
  switch (WiFi.getMode()) {
    case WIFI_OFF:
      message += F("OFF");
      break;
    case WIFI_STA:
      message += F("Station (Client)");
      break;
    case WIFI_AP:
      message += F("Access Point");
      break;
    case WIFI_AP_STA:
      message += F("Hybrid (AP+STA)");
      break;
    default:
      message += F("Unknown!");
  }
  message += F("\",\"");
  message += FPSTR(wifiRssi);
  message += F("\":\" -");  
  message += String(rssi);
  message += F("\",\"");
  message += FPSTR(mqttconnectedArg);
  message += F("\":");
  if (pubsubClient.connected())
    message += F("true");
  else
    message += F("false");
  message += F(",\"");
  message += FPSTR(uptimeArg);
  message += F("\":\"");  
  message += timeStart( timeValue );
  message += F("\"");      
  message += F("}");
                      // Строка которую формируем для парсинга должна быть следующего характера:
                      // {"ds18b20":30.37,"wifimode":"Station","mqttconnected":true,"freeheap":39216,"uptime":"0d 00h 00m 02s","relay":false}
                      //Serial.println(message);  // Для отладки
  httpServer.send(200, F("text/html"), message);
}


// Переподключение  *******************************************

bool mqttReconnect() {

  static uint32_t lastTime;
  bool result = false;

// Данный код будет выполнятся каждый раз при попытке соеденения к серверу MQTT ( в данном случае каждые timeout )
  if ( (millis() > lastTime + timeoutMQTT) ) {
    // Выводим сообщение о попвтке соеденения с сервером
    #ifdef TestSerialPrint
    Serial.print(F("Attempting MQTT connection..."));
    #endif
    digitalWrite(LedMqttConect, HIGH);
    if (mqttUser.length())
      result = pubsubClient.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPassword.c_str());
    else
      result = pubsubClient.connect(mqttClient.c_str());
    digitalWrite(LedMqttConect, LOW);
    if (result) {
      #ifdef TestSerialPrint
      Serial.println(F(" connected"));
      #endif
      digitalWrite( LedMqttConect, HIGH );  // При соеденении с брокером включаем светодиод индикатор
    } else {
      digitalWrite( LedMqttConect, LOW );   // При отсудствии соеденения с брокером выключаем светодиод индикатор
      #ifdef TestSerialPrint
      Serial.print(F(" failed, rc="));
      Serial.println(pubsubClient.state());
      #endif
    }
    lastTime = millis();
  }

  return result;
}

 // Подписываемся на топик ***********************************************
 
bool mqtt_subscribe(PubSubClient& client, const String& topic) {
  #ifdef TestSerialPrint
  Serial.print(F("Subscribing to "));
  Serial.println(topic);
  #endif

  return client.subscribe(topic.c_str());
}

// Публикуем топик ****************************************************

bool mqtt_publish(PubSubClient& client, const String& topic, const String& value, boolean saveValueServer) {
  #ifdef TestSerialPrint
  Serial.print(F("Publishing topic "));
  Serial.print(topic);
  Serial.print(F(" = "));
  Serial.println(value);
  #endif

  return client.publish(topic.c_str(), value.c_str(), saveValueServer);  // Третий пораметр сохранять на сервере даные или нет ( истина сохранять ложь нет )
}

void wifiRssiNow(){
//======= Определения уровня сигнала сети =============================================================================================================================      
    // Уровень сигнала к подключенному роутеру ( возвращает данные в dBm отрицательное значение )
    if (((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) && (WiFi.status() == WL_CONNECTED)){
        rssi = WiFi.RSSI() * (-1);
    }
    else rssi = 0;
   //  Идеальным Wi-Fi сигналом считается уровень от-60 dBm до -65 dBm. Все что выше -60 dBm (например -45 dBm) — это слишком мощный сигнал, все что ниже -80 dBm (например -87 dBm) — слишком слабый сигнал.      
//======================================================================================================================================================================
}


void IRorRF433(){
  decode_results results;
  if (mySwitch.available()) { // Если пришли данные на RF433 приемник
      digitalWrite(LedReceiver_IR_433,HIGH);
      codeNow = mySwitch.getReceivedValue();
      delay(150);     
      if( pubsubClient.connected() ) codeActive( String(codeNow) );
      digitalWrite(LedReceiver_IR_433,LOW);
      mySwitch.resetAvailable();
      
  }
   if (irrecv.decode(&results)) {// Если пришли данные на IR приемник
      digitalWrite(LedReceiver_IR_433,HIGH);
      codeNow = uint32_t(results.value);
      delay(150);
      if( codeNow>100 && pubsubClient.connected() ) codeActive( String(codeNow) );  //Проверяем если код больше 100 ( обычно помехи ловит и выдает до сотни ) тогда проверяем с базой кодов.
      digitalWrite(LedReceiver_IR_433,LOW);
      irrecv.resume();
   }
}

//===================================

void codeActive(String str){

    if( !str.compareTo(mqttCode1) ){ mqtt_publish(pubsubClient, mqttTopic1, mqttValue1, true); }
    if( !str.compareTo(mqttCode2) ){ mqtt_publish(pubsubClient, mqttTopic2, mqttValue2, true); }
    if( !str.compareTo(mqttCode3) ){ mqtt_publish(pubsubClient, mqttTopic3, mqttValue3, true); }
    if( !str.compareTo(mqttCode4) ){ mqtt_publish(pubsubClient, mqttTopic4, mqttValue4, true); }
    if( !str.compareTo(mqttCode5) ){ mqtt_publish(pubsubClient, mqttTopic5, mqttValue5, true); }
    if( !str.compareTo(mqttCode6) ){ mqtt_publish(pubsubClient, mqttTopic6, mqttValue6, true); }
    if( !str.compareTo(mqttCode7) ){ mqtt_publish(pubsubClient, mqttTopic7, mqttValue7, true); }
    if( !str.compareTo(mqttCode8) ){ mqtt_publish(pubsubClient, mqttTopic8, mqttValue8, true); }
    if( !str.compareTo(mqttCode9) ){ mqtt_publish(pubsubClient, mqttTopic9, mqttValue9, true); }
    if( !str.compareTo(mqttCode10) ){ mqtt_publish(pubsubClient, mqttTopic10, mqttValue10, true); }

    if( !str.compareTo(mqttCode11) ){ mqtt_publish(pubsubClient, mqttTopic11, mqttValue11, true); }
    if( !str.compareTo(mqttCode12) ){ mqtt_publish(pubsubClient, mqttTopic12, mqttValue12, true); }
    if( !str.compareTo(mqttCode13) ){ mqtt_publish(pubsubClient, mqttTopic13, mqttValue13, true); }
    if( !str.compareTo(mqttCode14) ){ mqtt_publish(pubsubClient, mqttTopic14, mqttValue14, true); }
    if( !str.compareTo(mqttCode15) ){ mqtt_publish(pubsubClient, mqttTopic15, mqttValue15, true); }
    if( !str.compareTo(mqttCode16) ){ mqtt_publish(pubsubClient, mqttTopic16, mqttValue16, true); }
    if( !str.compareTo(mqttCode17) ){ mqtt_publish(pubsubClient, mqttTopic17, mqttValue17, true); }
    if( !str.compareTo(mqttCode18) ){ mqtt_publish(pubsubClient, mqttTopic18, mqttValue18, true); }
    if( !str.compareTo(mqttCode19) ){ mqtt_publish(pubsubClient, mqttTopic19, mqttValue19, true); }
    if( !str.compareTo(mqttCode20) ){ mqtt_publish(pubsubClient, mqttTopic20, mqttValue20, true); }
}

//============================================================

void TimeUpStart(){
  timeValue++;
} 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  #ifdef TestSerialPrint
  Serial.begin(115200);
  Serial.println();
  #endif

  mySwitch.enableReceive(digitalPinToInterrupt(RF433pin)); 
  irrecv.enableIRIn();

  pinMode(LedWiFiConect, OUTPUT);
  pinMode(LedMqttConect, OUTPUT);
  pinMode(LedReceiver_IR_433, OUTPUT);

  digitalWrite(LedWiFiConect,LOW);
  digitalWrite(LedMqttConect,LOW);
  digitalWrite(LedReceiver_IR_433,LOW);
  

  timerId = timer.setInterval( APtoSTAreconnect, setupWiFi );
  timerRssi.setInterval( 1L*1000L, wifiRssiNow );
  timerUptime.setInterval(1000L, TimeUpStart );

     EEPROM.begin(16384);
  // EEPROM.begin(4096);
  // EEPROM.begin(1024);
  
  if (! readConfig()) // Читаем из памяти EEPROM все настройки и данные.
    #ifdef TestSerialPrint
    Serial.println(F("EEPROM is empty!"));
    #endif

  ArduinoOTA.setHostname( aHostname.c_str() ); // Задаем имя сетевого порта для OTA-прошивки
  ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки
  ArduinoOTA.begin(); // Инициализируем OTA
  
  WiFi.hostname (aHostname);    // Задаем имя хоста.
  
  setupWiFi();

  httpUpdater.setup(&httpServer);
  httpServer.onNotFound([]() {
    httpServer.send(404, F("text/plain"), F("FileNotFound"));
  });
  httpServer.on("/", handleRoot);
  httpServer.on("/index.html", handleRoot);
  httpServer.on("/wifi", handleWiFiConfig);
  httpServer.on("/mqtt", handleMQTTConfig);
  httpServer.on("/store", handleStoreConfig);
  httpServer.on("/reboot", handleReboot);
  httpServer.on("/data", handleData);
  httpServer.on("/name", deviceName);

  if (mqttServer.length()) {
    pubsubClient.setServer(mqttServer.c_str(), mqttPort);
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  yield();  // Вызов этой функции передает управление другим задачам. В идеале, вызов yield() должен осуществляться в функциях, выполнение которых занимает некоторое время.
  
  IRorRF433();
  
  ArduinoOTA.handle(); // Всегда готовы к OTA-прошивки

  timer.run(); 
  timerRssi.run(); 
  timerUptime.run();
  
  if (((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) && (WiFi.status() != WL_CONNECTED)) {
    digitalWrite(LedWiFiConect, LOW);
    setupWiFi();
  }
  if ((WiFi.getMode() == WIFI_AP) ){
    digitalWrite(LedWiFiConect, LOW);
    timer.enable(timerId);  // Включаем таймер по Id.
  }

  httpServer.handleClient();

  if (mqttServer.length() && ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA))) { 
    if (! pubsubClient.connected())
      mqttReconnect();
    if (pubsubClient.connected())
      pubsubClient.loop();
  }
  else digitalWrite( LedMqttConect, LOW );
  
  delay(5);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include <RCSwitch.h>             // https://github.com/sui77/rc-switch/
//#include <IRremoteESP8266.h>      // https://github.com/markszabo/IRremoteESP8266
//#include <IRrecv.h>
//#include <RCSwitch.h>
//#include <IRutils.h>
//
//#define IRpin D7
//#define RF433pin D2
//
//RCSwitch mySwitch = RCSwitch();
//IRrecv irrecv(IRpin);
//
//void setup() {
//  Serial.begin(9600);
//  mySwitch.enableReceive(digitalPinToInterrupt(RF433pin)); 
//  irrecv.enableIRIn();
//}
//
//void loop() {
//decode_results results;
//  if (mySwitch.available()) { // Если пришли данные на RF433 приемник
//      Serial.print( mySwitch.getReceivedValue() );
//      delay(300);
//      mySwitch.resetAvailable();
//  }
//   if (irrecv.decode(&results)) {// Если пришли данные на IR приемник
//      uint32_t A= uint32_t(results.value);
//      Serial.println(A);
//      delay(300);
//      irrecv.resume();
//   }
// 
//}
