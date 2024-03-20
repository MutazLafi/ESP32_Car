
/*
ESP32 Car By Mutaz Lafi 
Written in 2024
*/

//Libraries

#include <WiFi.h>
#include <WiFiclient.h>
#include <WebServer.h>
#include "OV2640.h"
#include "FS.h"
#include "SD_MMC.h"
#include <ESP32Servo.h>
#include <EEPROM.h>

// Camera Pins definitions

#include "cameraPins.h"

String RootMessege = "404 PageNotFound \n You are on Root Page Please Go to Streaming For Camera Streaming";


const char Header[] = "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789781000000000987654321\r\n";
const char Boundry[] = "\r\n--123456789781000000000987654321\r\n";
const char ContentType[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int HdrSize = strlen(Header);
const int BdrSize = strlen(Boundry);
const int CntSize = strlen(ContentType);


//definitions

// Networks Saved
#define SSID1 "HPLAPTOP 9019"
#define PWD1 "12345678"

#define SSID2 "HUAWEI"
#define PWD2 "shifa600"

#define SSID3 "Galaxy A52"
#define PWD3 "nedallafi15"


#define Flash_Pin 4
#define SteeringServo_Pin 13
#define RotatingServo_Pin 12

#define EEPROM_SIZE 64

//Variables


/*struct MotorPinsStruct {
  const byte IN1 = 2;
  const byte IN2 = 14;
  const byte IN3 = 15;
  const byte IN4 = 16;
};
*/

boolean WiFiConnectStatus = true;
boolean FlashState = false;
boolean ControlState = true;  // true for Car Control false for servo Control
int FileNameCounter = 0;
int RotatingServoAngleCounter = 0;
int SteeringServoAngleCounter = 0;

int EEPROM_Data;

//definitions used to control the code
#define DEBUG                // define for serial messges
#define CONTINUE_WITHOUT_SD  // define to continue the code if there is no SD Card presented
//Objects
WebServer server(80);
OV2640 camera;

Servo SteeringServo;
Servo RotatingServo;



#include "classes.h"

void setup() {


#ifdef DEBUG
  Serial.begin(115200);
#endif


  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Frame parameters
  //  config.frame_size = FRAMESIZE_UXGA;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif
  camera.init(config);
  EEPROM.begin(EEPROM_SIZE);


BeforeSwitch:

  EEPROM.get(0, EEPROM_Data);
  EEPROM_Data = 2;
  switch (EEPROM_Data) {
    case 1:
      WiFi.mode(WIFI_STA);
      WiFi.begin(SSID1, PWD1);
      Serial.println("Network 1 selected");
      break;

    case 2:
      WiFi.mode(WIFI_STA);
      WiFi.begin(SSID2, PWD2);
      Serial.println("Network 2 selected");
      break;

    case 3:
      WiFi.mode(WIFI_STA);
      WiFi.begin(SSID3, PWD3);
      Serial.println("Network 3 selected");
      break;

    default:
      Serial.println("Error Network is not selected");
      Serial.println(EEPROM_Data);
      while (true) {
        while (Serial.available()) {
          int Read = Serial.parseInt();
          EEPROM.put(0, Read);
          EEPROM.commit();
          Serial.println("Value Recieved: ");
          Serial.println(Read);
          goto BeforeSwitch;
        }
      }

      break;
  }

  while (WiFi.status() != WL_CONNECTED) {
    if (WiFiConnectStatus) {
      Serial.println("Can't connect To WiFi");
      Serial.print("Trying to Reconnect");
      WiFiConnectStatus = !WiFiConnectStatus;
    }
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi Connected");

  Serial.println("initializing SD Card....");

  if (!SD_MMC.begin()) {
    Serial.println("Error With SD Card , Card Mount Faild");
#ifndef CONTINUE_WITHOUT_SD
    while (true)
      ;
#endif
    Serial.println("Continue Without SD Card");
  } else {

    Serial.print("SD Card Started");
  }

  Serial.print("ESP 32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Streaming Website: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/Streaming");
  Serial.println("Server Started");

  CarMovment::initMotors();
  CarMovment::initServos();

  server.on("/", HTTP_GET, RequestHandlers::RootHandler);
  server.on("/Streaming", HTTP_GET, RequestHandlers::StreamingHandler);
  server.on("/Streaming/FLASH", HTTP_GET, RequestHandlers::FlashHandler);

  server.on("/Streaming/F", HTTP_GET, RequestHandlers::ForwardHandler);

  server.on("/Streaming/B", HTTP_GET, RequestHandlers::BackwardHandler);
  server.on("/Streaming/R", HTTP_GET, RequestHandlers::RightHandler);
  server.on("/Streaming/L", HTTP_GET, RequestHandlers::LeftHandler);
  //server.on("/Streaming/S", HTTP_GET, StopHandler);

  server.on("/Streaming/SWITCH", HTTP_GET, RequestHandlers::SwitchHandler);

  server.on("/Streaming/SaveIMG", HTTP_GET, RequestHandlers::SaveImageHandler);

  server.onNotFound(RequestHandlers::NotFoundHandler);


  server.begin();
}

void loop() {

  server.handleClient();
}