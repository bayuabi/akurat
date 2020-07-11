#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ESP_WiFiManager.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DEBUG true

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define OLED_RESET     -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String WIFI_SSID = "";
String WIFI_PASSWORD = "";

String MAIN_URL = "http://172.20.10.8/";
String SUBFOLDER_URL = "iot-dashboard/inputdata.php";

String CHIP_ID = "";

const char * API_KEY_TABLE[10] = {
                                  "miq3TdbIZP",
                                  "FDuzbCgPPb",
                                  "NqqRDCN52d",
                                  "QFsMgCkZsL",
                                  "lUpmcjKMeP",
                                  "F10NJIbkPf",
                                  "a1HQ8Gz6lM",
                                  "jPD9rmW5z4",
                                  "2vIt0XlQIQ",
                                  "ZoUBAoZ2uF"
                                 };

#define PUSHBUTTON_PIN  23
#define SOIL_PROBE_PIN  34

unsigned long startTime;
const uint16_t WIFIMANAGER_WAIT_TIME = 3000;
bool wifiSetting = false;

bool wifiInit(uint8_t timeout);
String getChipId();
bool sendData(int data);

void setup() {
  Serial.begin(115200);

  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
  pinMode(SOIL_PROBE_PIN, INPUT);

  ESP_WiFiManager ESP_wifiManager("AutoConnectAP");
  ESP_wifiManager.setDebugOutput(true);
  ESP_wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));
  ESP_wifiManager.setMinimumSignalQuality(-1);
  ESP_wifiManager.setSTAStaticIPConfig(IPAddress(192, 168, 2, 114), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0),
                                       IPAddress(192, 168, 2, 1), IPAddress(8, 8, 8, 8));

  startTime = millis();
  while(millis() - startTime <= WIFIMANAGER_WAIT_TIME){
    if(!digitalRead(PUSHBUTTON_PIN)){
      wifiSetting = true;
    }
    else{
      wifiSetting = false;
    }
  }

  if(wifiSetting){
    String AP_SSID = "AKURAT";
    String AP_PASS = "12345678";
    ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str());
  }

  WIFI_SSID = ESP_wifiManager.getStoredWiFiSSID();
  WIFI_PASSWORD = ESP_wifiManager.getStoredWiFiPass();

  wifiInit(10);

  CHIP_ID = getChipId();
  if(DEBUG) Serial.println(CHIP_ID);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    if(DEBUG) Serial.println(F("SSD1306 allocation failed"));
  }

  display.display();
}

void loop() {
  sendData(random(0,100));
  delay(1000);
}

bool wifiInit(uint8_t timeout){
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
  for(int i=0; i<timeout; i++){
    if(WiFi.status() == WL_CONNECTED){
      if(DEBUG) Serial.println("WIFI CONNECTED");

      return true;
    } 
    delay(1000);
  }
  if(DEBUG) Serial.println("FAIL CONNECT TO WIFI");
  return false;
}

String getChipId(){
  uint64_t ESP_CHIP_ID = ESP.getEfuseMac();
  char chipIdBuf[100];
  sprintf(chipIdBuf, "%x", ESP_CHIP_ID);
  String CHIP_ID = chipIdBuf;
  CHIP_ID.toUpperCase();

  return CHIP_ID;
}

bool sendData(int data){
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    String URL = MAIN_URL + SUBFOLDER_URL;

    http.begin(URL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    char bodyMessage[200];
    sprintf(bodyMessage, "api_key=%s&node_id=%s&soil=%d", API_KEY_TABLE[random(0,9)], CHIP_ID, data);

    int httpResponse = http.POST(bodyMessage);
    if(DEBUG) Serial.print("HTTP Response: "); Serial.println(httpResponse);
    http.end();
  }
}
