#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID           "TMP000002"
#define BLYNK_TEMPLATE_NAME         "Esp-bee-2"
#define BLYNK_AUTH_TOKEN            "6GJOOLhGyPpGwSosOMXiOZILtMd6Wmv5" // 3-й токен для дома в Подгорах

//#include <Narodmon.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoOTA.h>
#include <ESP8266httpUpdate.h>

// GPIO0-реле
#define RELAY_PIN 0   // GPIO14-реле
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
BlynkTimer timer;
//int overRideValue;

float temp = 0;
int k = 0;
int res = 0;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] =BLYNK_AUTH_TOKEN;
char serv[] = "blynk.su"; // 39 + \0
char butt[2];  //  1 + \0

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] ="wi210_2ghz"; //"ASUS"; //"HUAWEI-E8231-63ae"; "wi210_2ghz";//"Point";//
char pass[] ="DVtV42odc-6"; //"nilzhurav555"; // "b2bqm5r0"; // "DVtV42odc-6"; //"GR0LP25NQSMN";//
int count = 0;
int b = 36;
int maxcount = 3;
int maxap = 10;
File testFile;
WiFiClient client1;

//===============================================================================
void wificon()                    // ----------------------WiFi Conn
{
  if (count < maxcount){
    yield();
    WiFi.mode(WIFI_STA);
    Serial.println();
    Serial.print("Connect: ");Serial.print(ssid);Serial.print(" ");Serial.println(pass);
    WiFi.begin(ssid, pass); 
    }else {
    if (count == maxcount){
      WiFi.mode(WIFI_AP);
      WiFi.softAP("esp01", "esp");
      Serial.println("create AP esp01/esp");
      count = maxap;
      Serial.println(WiFi.softAPIP());
    }
  }
}
void blynkconn()                  // -----------------Blynk Conn
{
  if(WiFi.status() == WL_CONNECTED) {
    if (b < 1) {
      b = 36;  // 6 минут ждем до следующего подключения blynk
      Blynk.config(auth, serv, 8080);
      yield();
      Blynk.connect(3000);
      if (Blynk.connected() == true ){
        Serial.println ("Blynk connected");
        Serial.println (WiFi.localIP());
        }
      else {
        Serial.println ("Disconnected");
        }
      }
      else {
        b = b-1;
      }
  }else
  {
    Serial.println("Wifi - disconnected, reconnect...");
    yield();
    wificon();
  }
}
//---------------------------------------------
void readfile(char *str, const char *path) //-------------ReadFile----------
{  
  testFile = LittleFS.open(path, "r");
  if (testFile){
        Serial.print("Read file ");
        Serial.print(path);
        Serial.print(" :");
        int i = 0;
        while (testFile.available()) {
          *(str+i) = testFile.read(); //Serial.write(testFile.read());
          i++;
          }
        str[i] = '\0';  
    }else{
        Serial.println("Problem on read file!");
    }  
    Serial.println(str);
    testFile.close();
}
void writefile(char *str, const char *path) //------------WriteFile---------
{  
  testFile = LittleFS.open(path, "w");
  if (testFile){
        Serial.print("Write file ");
        Serial.print(path);
        Serial.print(" :");
//        for (int i = 0; i < sizeof(str); i++ ) {
//          if (*(str+i) == '\0') { break; }        
//          else { testFile.write(*(str+i)); }          
//        }
        int i = 0;
        while (*(str+i) != '\0') {
          testFile.write(*(str+i));
          //*(str+i) = testFile.read(); //Serial.write(testFile.read());
          i++;
        }
        
    }else{
        Serial.println("Problem on write file!");
    }  
    Serial.println(str);
    testFile.close();
}
void toggleRelay(char *status)   // --------------toggleRelay в зависимости от butt[0]
{
  pinMode(RELAY_PIN,OUTPUT);
  if (status[0] == '1'){
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("делаем LOW - Включаем РЕЛЕ, т.к. butt.txt = 1");
  }  
  else {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("делаем HIGH - OFF - Выкл реле, т.к. butt.txt = 0");
  }
}
void narodsend(){              //--------------отправка на NARODMON-----
      Serial.println("---");
      String buf; // Буфер для отправки на NarodMon
      String buf2; // Буфер для ответа сервера
      buf="#ESP" + WiFi.macAddress()+ "\r\n";buf.replace(":", ""); //формируем заголовок
      //buf = "#ESP2462AB08BE5C\r\n"; // номер прибора Подгоры
      //buf = "#ESPCC50E3DAE838\r\n"; // номер прибора дома
      buf = buf + "#";
      //DeviceAddress tempDeviceAddress; // Переменная для получения адреса датчика
      //DS18B20.getAddress(tempDeviceAddress, 0); // Получаем уникальный адрес датчика
      //for (uint8_t i = 0; i < 8; i++) {if (tempDeviceAddress[i] < 16) buf = buf + "0"; buf = buf + String(tempDeviceAddress[i], HEX);} // адрес датчика в буфер
      buf = buf + "355102d3";      
      buf = buf + "#"+ String(temp)+ "\r\n"; //температура улица
      //buf = buf + "#103d593f020800d1#" + String(temp)+ "\r\n"; //температура улица
      //buf = buf + "#123456780#" + String(humm)+ "\r\n"; //влажность улица
      //buf = buf + "#123456781#" + String(t) + "\r\n"; // температура омш
      //buf = buf + "#123456782#" + String(h) + "\r\n"; // влажность омш
      buf = buf + "##\r\n"; // закрываем пакет
      Serial.println(buf); //шлем буфер по COM-порту
      
      client1.connect("narodmon.ru", 8283); //Подключаемся
      client1.print(buf); // И отправляем данные
      Serial.println("-----");
      buf2 = client1.readStringUntil('\n'); // считываем ответ сервера
      client1.stop();

      Serial.println(buf2);
      Serial.println("-----");
      if (buf2 == "#relay=1") {
        Serial.print("ВКЛ РЕЛЕ ");
        butt[0] = '1';
        writefile(butt, "butt.txt");
        toggleRelay(butt);        
      }
      if (buf2 == "#relay=0") {
        Serial.print("ВЫКЛ РЕЛЕ ");
        butt[0] = '0';
        writefile(butt, "butt.txt");
        toggleRelay(butt);        
      }
      if (buf2 == "#update") {
        Serial.println("Update now ");
        update_and();
      }
}
//----------------Update-----------
void update_and() //HTTP_UPDATE_FAILD Error (-1): HTTP error: connection failed
{
  t_httpUpdate_return ret = ESPhttpUpdate.update(client1, "ftp://anyboard.ru:50021/pub/nil/podg1.bin");
  switch (ret) {
    case HTTP_UPDATE_FAILED: Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); break;
    case HTTP_UPDATE_NO_UPDATES: Serial.println("HTTP_UPDATE_NO_UPDATES"); break;
    case HTTP_UPDATE_OK: Serial.println("HTTP_UPDATE_OK"); break;
    }
}
//=============================================
void setup()
{
  // Debug console
  digitalWrite(RELAY_PIN,HIGH); // начальное выключение реле
  Serial.begin(9600);
  DS18B20.begin();
  wificon();
  
  ArduinoOTA.setHostname("Esp-DOM");              //OTA Задаем имя сетевого порта
  ArduinoOTA.setPassword((const char *)"0001");    //OTA Задаем пароль доступа для удаленной прошивки
  ArduinoOTA.begin();                              //OTA Запуск
  
  Serial.println("Ready OTA");

  if (LittleFS.begin()){
      Serial.println(F("done."));
    } else {
        Serial.println(F("fail."));
      }    
  readfile(butt, "butt.txt");
  toggleRelay(butt);  
  
  Serial.println("Start timer");
  timer.setInterval(10000, sendTemps); // каждые 10 секунд
  timer.run();  
}

void sendTemps() //-----------обработка таймера----------
{
  //Serial.println("Timer ON");
  DS18B20.requestTemperatures();
  float temp1 = DS18B20.getTempCByIndex(0);
  int i = 0;
  Serial.println(temp);
  if ((temp1 < 100) && (temp1 > -100)) temp = temp1;
  Blynk.virtualWrite(V3,temp);
                                                         //---обработка RELAY_PIN для BLYNK - LED и LEVEL
  Blynk.virtualWrite(V4,1023-1023*digitalRead(RELAY_PIN));         //для LEVEL
  Blynk.virtualWrite(V5,255-255*digitalRead(RELAY_PIN)); // для LED
  if (k > 30) { // 30 это 5 минут
     narodsend();
     k = 0;      
   } //if k>5
   else {res = res + 1; k = k + 1;}
//   Serial.println(overRideValue);
  //---------------------------------------------------- обработка RELAY_PIN - запись в FS, если вкл/выкл с тел
  if (digitalRead(RELAY_PIN) == HIGH) { //HIGH - реле выключено
    //Serial.print("-HIGH- -");
    //Serial.println(butt[0]);
    if (butt[0] == '1') {
      butt[0] = '0';
      writefile(butt, "butt.txt");
      }
    }
  else {
    //Serial.print("-LOW- -");
    //Serial.println(butt[0]);
    if (butt[0] == '0') {
      butt[0] = '1';
      writefile(butt, "butt.txt");  
      }
    }
//-------------------рестарт через 60 минут-------------------------------
   if (res > 360) { // 360 это 1 час
     res = 0;
//     if (Blynk.connected() == false) {
//       Serial.print("system_restart");
//       system_restart();//if (WiFi.status() != WL_CONNECTED) {Serial.print("system_restart"); system_restart();}  
//       }
   }
   
// --------Проверка на подключение к Blynk и запуск переподключения--------
  if (Blynk.connected() == false)
    {
    Serial.println(" Blynk disconnected!!! Go blynkconn()");
    blynkconn();
    }
}
 
void loop()
{
//  Blynk.run();
  if(Blynk.connected() == true) {
    Blynk.run();
    }
  timer.run();
  ArduinoOTA.handle();

}
