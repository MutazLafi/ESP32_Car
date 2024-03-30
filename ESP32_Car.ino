
/*
ESP32 Car By Mutaz Lafi 
Written in 2024
*/

//Libraries

#include <WiFi.h>
#include <WiFiclient.h>
#include <WebServer.h>
#include "OV2640.h"
//#include "FS.h"  
//#include "SD_MMC.h"
#include <ESP32Servo.h>
#include <EEPROM.h>


// Camera Pins definitions

#include "cameraPins.h"

String RootMessege = "404 PageNotFound \n You are on Root Page Please Go to /Streaming For Camera Streaming";


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
#define StatusLed_Pin 33

#define SteeringServo_Pin 13
#define RotatingServo_Pin 12

#define SpeedSet_Pin 1

#define EEPROM_Size 64  // The Maximum Size Used or Higher in bytes



//Variables


struct MotorPinsStruct {
  const byte IN1 = 2;
  const byte IN2 = 14;
  const byte IN3 = 15;
  const byte IN4 = 3;
};

MotorPinsStruct MotorPins;

boolean WiFiConnectStatus = true;
boolean FlashState = false;

int DefaultRotatingServoAngle = 80;
int DefaultSteeringServoAngle = 90;

int RotatingServoAngleCounter;
int SteeringServoAngleCounter;

int Speed = 125;
// Note : This is the Default Time and it change with app control
float DirectionTime = 1000;  // Time in milliseconds for how many time the car will keep going right or left


//definitions used to control the code
#define DEBUG                // define for serial messges
//#define ConstSpeed  120  //from 0 to 255 define for contant speed
#define DirectionSpeed 255 // Speed used in Right/Left 

//Objects
WebServer server(80);
OV2640 camera;

Servo SteeringServo;
Servo RotatingServo;


// File that have All classes and functions in it
#include "classes.h"

void setup() {
  CarMovment::initMotors();  
  CarMovment::Stop();

  CarMovment::initServos();

#ifdef DEBUG
  Serial.begin(115200);
#endif

  // Camera Pins
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


  camera.init(config);
  EEPROM.begin(EEPROM_Size);
  pinMode(StatusLed_Pin, OUTPUT);
  
  // Changing Network operation
  Serial.println("Choosing Network....");
  while (millis() < 1000) {
    while (Serial.available() > 0) {
      char Read = Serial.read();
      Serial.println("Choosing New Network...");
      if (Read == 'A') {
        EEPROM.write(0, 1);
        EEPROM.commit();
        goto ChoosingNetwork;
      }

      if (Read == 'B') {
        EEPROM.write(0, 2);
        EEPROM.commit();
        goto ChoosingNetwork;
      }

      if (Read == 'C') {
        EEPROM.write(0, 3);
        EEPROM.commit();
        goto ChoosingNetwork;
      }
    }
  }


ChoosingNetwork:
  byte EEPROM_Data;

  EEPROM_Data = EEPROM.read(0);
  Serial.println(EEPROM_Data);
  switch (EEPROM_Data) {
    case 1:
      WiFi.mode(WIFI_STA);
      WiFi.begin(SSID1, PWD1);
      Serial.println("Network A selected");
      break;

    case 2:
      WiFi.mode(WIFI_STA);
      WiFi.begin(SSID2, PWD2);
      Serial.println("Network B selected");
      break;

    case 3:
      WiFi.mode(WIFI_STA);
      WiFi.begin(SSID3, PWD3);
      Serial.println("Network C selected");
      break;

    default:
      Serial.println("Error Network is not selected");
      Serial.println("Please Choose A Network");
      Serial.println("A - HPLAPTOP 9019");
      Serial.println("B - HUAWEI");
      Serial.println("C - Galaxy A52");
      while (Serial.available() > 0) {
        char Read = Serial.read();
        Serial.println("Choosing New Network...");
        if (Read == 'A') {
          EEPROM.write(0, 1);
          EEPROM.commit();
          goto ChoosingNetwork;
        }

        if (Read == 'B') {
          EEPROM.write(0, 2);
          EEPROM.commit();
          goto ChoosingNetwork;
        }

        if (Read == 'C') {
          EEPROM.write(0, 3);
          EEPROM.commit();
          goto ChoosingNetwork;
        }
      }


      break;
  }

  while (WiFi.status() != WL_CONNECTED) {  // Checking Wifi Status
    if (WiFiConnectStatus) {
      Serial.println("Can't connect To WiFi");
      Serial.print("Trying to Reconnect");
      WiFiConnectStatus = !WiFiConnectStatus;
    }
    Serial.print(".");
    digitalWrite(StatusLed_Pin, HIGH);
    delay(250);
    digitalWrite(StatusLed_Pin, LOW);
    delay(250);
  }
  digitalWrite(StatusLed_Pin, LOW);

  Serial.println("WiFi Connected");


  // You can put here SD Card Initialization ( Not used in My code Due To Not Having Enough Pins)



  Serial.print("ESP 32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Streaming Website: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/Streaming");
  Serial.println("Server Started");


  server.on("/", HTTP_GET, RequestHandlers::RootHandler);
  server.on("/Streaming", HTTP_GET, RequestHandlers::StreamingHandler);
  server.on("/Streaming/FLASH", HTTP_GET, RequestHandlers::FlashHandler);

  server.on("/Streaming/F", HTTP_GET, RequestHandlers::ForwardHandler);

  server.on("/Streaming/B", HTTP_GET, RequestHandlers::BackwardHandler);
  server.on("/Streaming/R", HTTP_GET, RequestHandlers::RightHandler);
  server.on("/Streaming/L", HTTP_GET, RequestHandlers::LeftHandler);
  server.on("/Streaming/S", HTTP_GET, RequestHandlers::StopHandler);



  
 
  server.on("/Streaming/RST", HTTP_GET, [] {
    ESP.restart();
  });

  server.on("/Streaming/SpeedData/", HTTP_GET, RequestHandlers::SpeedDataHandler);

  // Can't Work Due To Pins Not Enough
  //server.on("/Streaming/SaveIMG", HTTP_GET, RequestHandlers::SaveImageHandler);

  server.onNotFound(RequestHandlers::NotFoundHandler);


  server.begin();
}

void loop() {

  server.handleClient();
  delay(2);  // for stability
}