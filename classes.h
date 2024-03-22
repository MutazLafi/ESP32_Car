class CarMovment {
public:


  static void initMotors() {
    pinMode(MotorPins.IN1, OUTPUT);
    pinMode(MotorPins.IN2, OUTPUT);
    pinMode(MotorPins.IN3, OUTPUT);
    pinMode(MotorPins.IN4, OUTPUT);
    Stop();
  }

  static void initServos() {
    SteeringServo.attach(SteeringServo_Pin);
    RotatingServo.attach(RotatingServo_Pin);

    SteeringServo.write(90);
    RotatingServo.write(90);
  }

  static void Forward() {
    digitalWrite(MotorPins.IN1, HIGH);
    digitalWrite(MotorPins.IN2, LOW);
    digitalWrite(MotorPins.IN3, HIGH);
    digitalWrite(MotorPins.IN4, LOW);
  }

  static void Backward() {
    digitalWrite(MotorPins.IN1, LOW);
    digitalWrite(MotorPins.IN2, HIGH);
    digitalWrite(MotorPins.IN3, LOW);
    digitalWrite(MotorPins.IN4, HIGH);
  }

  static void Right() {
    digitalWrite(MotorPins.IN1, LOW);
    digitalWrite(MotorPins.IN2, HIGH);
    digitalWrite(MotorPins.IN3, HIGH);
    digitalWrite(MotorPins.IN4, LOW);
  }

  static void Left() {
    digitalWrite(MotorPins.IN1, LOW);
    digitalWrite(MotorPins.IN2, HIGH);
    digitalWrite(MotorPins.IN3, LOW);
    digitalWrite(MotorPins.IN4, HIGH);
  }

  static void Stop() {
    digitalWrite(MotorPins.IN1, LOW);
    digitalWrite(MotorPins.IN2, LOW);
    digitalWrite(MotorPins.IN3, LOW);
    digitalWrite(MotorPins.IN4, LOW);
  }
};

class RequestHandlers {

public:
  static void RootHandler(void) {
    server.send(404, "text/Plain", RootMessege);
  }

  static void NotFoundHandler(void) {
    server.send(404, "text/plain", "404 Page Not Found");
  }

  static void StreamingHandler(void) {


    WiFiClient client = server.client();

    char buf[32];
    int FrameSize;

    client.write(Header, HdrSize);
    client.write(Boundry, BdrSize);

    while (true) {
      if (!client.connected()) break;
      server.handleClient();
      camera.run();
      FrameSize = camera.getSize();
      client.write(ContentType, CntSize);
      sprintf(buf, "%d\r\n\r\n", FrameSize);
      client.write(buf, strlen(buf));
      client.write((char *)camera.getfb(), FrameSize);
      client.write(Boundry);
    }
  }

  static void FlashHandler(void) {
    pinMode(Flash_Pin, OUTPUT);
    digitalWrite(Flash_Pin, !FlashState);
    FlashState = !FlashState;
    StreamingHandler();
  }

  static void SaveImageHandler() {  // Not Used in Server "on" Function as a handler due to not having enough pins
    fs::FS &fs = SD_MMC;
    char buf[32];
    int s;
    camera.run();
    s = camera.getSize();

    File file = fs.open("/image.jpg", FILE_WRITE);

    if (!file) {
      Serial.println("faild to open file");
    }
    camera.run();
    delay(1000);
    camera.run();
    if (file.write(camera.getfb(), s)) {
      Serial.println("File WRITTEN");
    } else {
      Serial.println("faild to write to file");
    }

    file.close();
    StreamingHandler();
  }

  static void SwitchHandler(void) {
    ControlState = !ControlState;
    StreamingHandler();
  }

  static void ForwardHandler(void) {
    CarMovment::initMotors();
    if (!ControlState) {
      CarMovment::Forward();
    } else {
      RotatingServoAngleCounter += 20;
      if (RotatingServoAngleCounter > 180) {
        RotatingServoAngleCounter = 180;
      }
      RotatingServo.write(RotatingServoAngleCounter);
    }
    StreamingHandler();
  }

  static void BackwardHandler() {

    if (!ControlState) {


    CarMovment::Stop();
    delay(20);
    CarMovment::Backward();
   
  
    } else {
      RotatingServoAngleCounter -= 20;
      if (RotatingServoAngleCounter < 0) {
        RotatingServoAngleCounter = 0;
      }
      RotatingServo.write(RotatingServoAngleCounter);
    }
    StreamingHandler();
  }

  static void RightHandler(void) {
    CarMovment::Stop();
    if (!ControlState) {
      CarMovment::Right();
    } else {
      SteeringServoAngleCounter += 20;
      if (SteeringServoAngleCounter > 180) {
        SteeringServoAngleCounter = 180;
      }
      SteeringServo.write(SteeringServoAngleCounter);
    }
    StreamingHandler();
  }

  static void LeftHandler(void) {
    CarMovment::Stop();
    if (!ControlState) {
      CarMovment::Left();
    } else {
      SteeringServoAngleCounter -= 20;
      if (SteeringServoAngleCounter < 0) {
        SteeringServoAngleCounter = 0;
      }
      SteeringServo.write(SteeringServoAngleCounter);
    }
    StreamingHandler();
  }
};
