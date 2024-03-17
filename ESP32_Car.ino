
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
#include <SD_MMC.h>
#include <ESP32Servo.h>

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
#define SSID "HPLAPTOP 9019"
#define PWD "12345678"


#define Flash_Pin 4
#define SteeringServo_Pin 13
#define RotatingServo_Pin 12

//Variables
struct MotorPins{
   const byte IN1 = 3;
   const byte IN2 = 4;
   const byte IN3 = 5;
   const byte IN4 = 6;
};

boolean WiFiConnectStatus = true;
boolean FlashState = false;
boolean ControlState = true;  // true for Car Control false for servo Control
int FileNameCounter = 0;

//definitions used to control the code
#define DEBUG // define for serial messges
#define CONTINUE_WITHOUT_SD // define to continue the code if there is no SD Card presented
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



  camera.init(config);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);

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
    fs::FS &fs = SD_MMC;
    Serial.print("SD Card Started");
    
    
    Serial.println("Creating Folder For The Car.....");
    if (fs.mkdir("/ESP32CAR")) {

      Serial.println("Created Folder For The car");
    } else {

      Serial.println("Faild To Create Folder");
    }
  }

  Serial.print("ESP 32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Streaming Website: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/Streaming");
  Serial.println("Server Started");


  pinMode(4, OUTPUT);
  server.on("/", HTTP_GET, RequestHandlers::RootHandler);
  server.on("/Streaming", HTTP_GET, RequestHandlers::StreamingHandler);
  server.on("/Streaming/FLASH", HTTP_GET, RequestHandlers::FlashHandler);
  /* 
  server.on("/Streaming/F", HTTP_GET, ForwardHandler);
  server.on("/Streaming/B", HTTP_GET, BackwardHandler);
  server.on("/Streaming/R", HTTP_GET, RightHandler);
  server.on("/Streaming/L", HTTP_GET, LeftHandler);
  server.on("/Streaming/S", HTTP_GET, StopHandler);
  */
  server.on("/Streaming/SWITCH", HTTP_GET, RequestHandlers::SwitchHandler);
  
  server.on("/Streaming/SaveIMG", HTTP_GET, RequestHandlers::SaveImageHandler);

  server.onNotFound(RequestHandlers::NotFoundHandler);

  server.begin();
}

void loop() {
  server.handleClient();
}