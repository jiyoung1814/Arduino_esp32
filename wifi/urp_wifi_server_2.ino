//#include <FS.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <BH1745.h>
#include <Arduino.h>
#include <time.h> 
#include "AsyncJson.h"
#include "ArduinoJson.h"
#define SDA SDA
#define SCL SCL

AsyncWebServer server(80);
BH1745 bh = BH1745();

const char* ssid = "*****" ;//wifi ID
const char* password = "*******"; //wifi PASSWORD

const char* PARAM_MESSAGE = "message";
String jsondata ="";
long r,g,b;

//시간 받아오기
int timezone = 3; 
int dst = 0; 
unsigned long previousMillis = 0;
const long interval = 1000; 
String datetime="";


void setup() {

    Serial.begin(115200);

    //bh1745 연결
    bool result = bh.begin(SDA, SCL);
    if (!result){
    Serial.println("Device Error");
    while (1){;;}
    }
    bh.setGain(bh.GAIN_16X);
    bh.setRgbcMode(bh.RGBC_8_BIT);

   //시간 받아오기
   configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); 
   Serial.println("\nWaiting for time"); 
   while (!time(nullptr)) { 
     Serial.print("."); 
     delay(1000); 
   }
   Serial.println(""); 

    //wifi 연결
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    setApi();
    startServer();
}
void setApi(){
  
  
  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/post/channel", [](AsyncWebServerRequest *request, JsonVariant &json) {
    jsondata ="";
    
    JsonObject& jsonObj = json.as<JsonObject>();
    jsonObj.printTo(jsondata);
    Serial.println(jsondata);

    int ch1 = jsonObj["ch1"];  // get value of the sensor measurement
    int ch2 = jsonObj["ch2"];
    int ch3 = jsonObj["ch3"];
    int ch4 = jsonObj["ch4"];
    
      AsyncJsonResponse * response = new AsyncJsonResponse();
      response->addHeader("Server","ESP Async Web Server");
      JsonObject& root = response->getRoot();
      root["ch1"] = ch1;
      root["ch2"] = ch2;
      root["ch3"] = ch3;
      root["ch4"] = ch4;
      response->setLength();
      request->send(response);
      
  });
  server.addHandler(handler);
  
    server.on("/get/rgb", HTTP_GET, [] (AsyncWebServerRequest *request) {
      
    AsyncJsonResponse * response = new AsyncJsonResponse();
    response->addHeader("Server","ESP Async Web Server");
    JsonObject& root = response->getRoot();
    root["datetime"] = datetime;
    root["r"] = r;
    root["g"] = g;
    root["b"] = b;
    response->setLength();
    request->send(response);
    });

//      server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
//      if (request->url() == "/test") {
////        if (!handleTest(request, data)) request->send(200, "text/plain", "false");
//
//          for(size_t i=0; i<len; i++){
//            Serial.write(data[i]);
////            buf.push(data[i]);
//          }
//
//          request->send(200, "text/plain", "Hello World!");
//      }
//    });

//     server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//     String message;
//      if (request->hasParam(PARAM_MESSAGE)) {
//          message = request->getParam(PARAM_MESSAGE)->value();
//      } else {
//          message = "No message sent";
//      }
//        request->send(200, "text/plain", "Hello, world");
//    });


    // Send a POST request to <IP>/post with a form field message set to <message>
//    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){ 
//        String message;
//        if (request->hasParam(PARAM_MESSAGE, true)) {//파라미터 확인 if 있다면 값을 추출해 HTTP response의 body에 담아 보냄
//            message = request->getParam(PARAM_MESSAGE, true)->value();
//        } else {
//            message = "No message sent";
//        }
//        request->send(200, "text/plain", "Hello, POST: " + message);
//    });

    server.onNotFound(notFound);
}

void startServer(){
  server.begin();
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


void loop() {

  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    time_t now = time(nullptr); 
    datetime = ctime(&now);
    datetime = datetime.substring(0,(datetime.length()-1));
    Serial.print(datetime);Serial.print(',');

    bh.read();
    r = bh.red_65536;
    g = bh.green_65536;
    b = bh.blue_65536;
//      r=100;
//      g=100;
//      b=100;
    Serial.print("r:");Serial.print(r);Serial.print(',');
    Serial.print("g:");Serial.print(g);Serial.print(',');
    Serial.print("b:");Serial.print(b);Serial.println();
 
  }
  
   
   
//  Serial.print("r:");Serial.print(bh.red_65536);Serial.print(',');
//  Serial.print("g:");Serial.print(bh.green_65536);Serial.print(',');
//  Serial.print("b:");Serial.print(bh.blue_65536);Serial.println();
   
//   delay(1000);
  
  
}
