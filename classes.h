class CarMovment {
public:


  static void initMotors() {
    pinMode(2, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(15, OUTPUT);
    pinMode(4, OUTPUT);
  }

  static void initServos() {
    SteeringServo.attach(SteeringServo_Pin);
    RotatingServo.attach(RotatingServo_Pin);
  }

  static void Forward() {
    digitalWrite(2, HIGH);
    digitalWrite(14, LOW);
    digitalWrite(15, HIGH);
    digitalWrite(4, LOW);
  }

  static void Backward() {
    digitalWrite(2, LOW);
    digitalWrite(14, HIGH);
    digitalWrite(15, LOW);
    digitalWrite(4, HIGH);
  }

  static void Right() {
    digitalWrite(2, LOW);
    digitalWrite(14, HIGH);
    digitalWrite(15, HIGH);
    digitalWrite(4, LOW);
  }

  static void Left() {
    digitalWrite(2, LOW);
    digitalWrite(14, HIGH);
    digitalWrite(15, LOW);
    digitalWrite(4, HIGH);
  }
};

class RequestHandlers {
  CarMovment Movment;
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
    pinMode(4, OUTPUT);
    digitalWrite(4, FlashState);
    FlashState = !FlashState;
    StreamingHandler();
  }

  static void SaveImageHandler(void) {
  fs::FS &fs = SD_MMC;
  char buf[32];
  int s;
  camera.run();
  s = camera.getSize();

  File file = fs.open("/image.jpg");

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

  static void BackwardHandler(void) {

    if (!ControlState) {
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
