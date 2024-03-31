class CarMovment {
public:


  static void initMotors() {
    pinMode(MotorPins.IN1, OUTPUT);
    pinMode(MotorPins.IN2, OUTPUT);
    pinMode(MotorPins.IN3, OUTPUT);
    pinMode(MotorPins.IN4, OUTPUT);
    pinMode(SpeedSet_Pin, OUTPUT);

    Stop();
  }

  static void initServos() {
    SteeringServo.attach(SteeringServo_Pin);
    RotatingServo.attach(RotatingServo_Pin);

    SteeringServo.write(DefaultSteeringServoAngle);
    RotatingServo.write(DefaultRotatingServoAngle);
    SteeringServoAngleCounter = DefaultSteeringServoAngle;
    RotatingServoAngleCounter = DefaultRotatingServoAngle;
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
    digitalWrite(MotorPins.IN2, LOW);
    digitalWrite(MotorPins.IN3, HIGH);
    digitalWrite(MotorPins.IN4, LOW);
  }

  static void Left() {
    digitalWrite(MotorPins.IN1, HIGH);
    digitalWrite(MotorPins.IN2, LOW);
    digitalWrite(MotorPins.IN3, LOW);
    digitalWrite(MotorPins.IN4, LOW);
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
    server.handleClient();

    WiFiClient client;
    server.handleClient();
    client = server.client();
    server.handleClient();

    char buf[64];
    int FrameSize;
    server.handleClient();
    client.write(Header, HdrSize);
    server.handleClient();
    client.write(Boundry, BdrSize);
    server.handleClient();

    while (true) {
      server.handleClient();
      if (!client.connected()) break;
      server.handleClient();
      camera.run();
      server.handleClient();
      FrameSize = camera.getSize();
      server.handleClient();
      client.write(ContentType, CntSize);
      server.handleClient();
      sprintf(buf, "%d\r\n\r\n", FrameSize);
      server.handleClient();
      client.write(buf, strlen(buf));
      server.handleClient();
      client.write((char *)camera.getfb(), FrameSize);
      server.handleClient();
      client.write(Boundry);
      server.handleClient();
    }
  }

  static void FlashHandler(void) {
    pinMode(Flash_Pin, OUTPUT);
    digitalWrite(Flash_Pin, !FlashState);
    FlashState = !FlashState;
    StreamingHandler();
  }




  static void ForwardHandler(void) {
    CarMovment::initMotors();
    analogWrite(SpeedSet_Pin, Speed);
    CarMovment::Forward();

    StreamingHandler();
  }

  static void BackwardHandler() {
    CarMovment::initMotors();
    analogWrite(SpeedSet_Pin, Speed);
    CarMovment::Backward();



    StreamingHandler();
  }

  static void RightHandler(void) {
    CarMovment::initMotors();
    analogWrite(SpeedSet_Pin, DirectionSpeed);
    CarMovment::Right();
    delay(DirectionTime);
    CarMovment::initMotors();

    StreamingHandler();
  }

  static void LeftHandler(void) {
    CarMovment::initMotors();
    analogWrite(SpeedSet_Pin, DirectionSpeed);
    CarMovment::Left();
    delay(DirectionTime);
    CarMovment::initMotors();

    StreamingHandler();
  }

  static void StopHandler(void) {
    CarMovment::initMotors();
    CarMovment::Stop();
    StreamingHandler();
  }

  static void SpeedDataHandler(void) {
    String SpeedString = server.arg("Speed");
    int SpeedInt = SpeedString.toInt();
    Speed = SpeedInt;


    String SteeringString = server.arg("Steering");
    SteeringServo.write(SteeringString.toInt());



    String RotatingString = server.arg("Rotating");
    RotatingServo.write(RotatingString.toInt());

    String DelayTimeString = server.arg("Delay");
    float Time = DelayTimeString.toFloat();
    DirectionTime = (1000 * Time);

    StreamingHandler();
  }

  static void NetworkChangeHandler(void) {
    String NumberString = server.arg("Network");

    int Number = NumberString.toInt();

    switch (Number) {
      case 1:
        EEPROM.write(0, 1);
        EEPROM.commit();
        ESP.restart();

      case 2:
        EEPROM.write(0, 2);
        EEPROM.commit();
        ESP.restart();

      case 3:
        EEPROM.write(0, 3);
        EEPROM.commit();
        ESP.restart();

      case 3:
        EEPROM.write(0, 4);
        EEPROM.commit();
        ESP.restart();
    }
  }


  /*static void SaveImage() {  // Not Used in Server "on" Function as a handler due to not having enough pins

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
  }*/
};
