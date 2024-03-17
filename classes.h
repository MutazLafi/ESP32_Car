class RequestHandlers {

public:
  static void RootHandler(void) {
    server.send(404, "text/Plain", RootMessege);
  }

  static void NotFoundHandler(void) {
    server.send(404, "text/plain", "404 Page Not Found");
  }

  static void StreamingHandler(void) {


    char buf[32];
    int FrameSize;
    WiFiClient client = server.client();
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

    digitalWrite(4, FlashState);
    FlashState = !FlashState;
    StreamingHandler();
  }

  static void SaveImageHandler(void) {
    fs::FS &fs = SD_MMC;
    camera.run();
    int FrameSize = camera.getSize();

    FileNameCounter++;
    File file = fs.open(("/ESP32CAR/image" + String(FileNameCounter) + ".jpg"), FILE_WRITE);

    if (!file) {
      Serial.println("Faild to open File or Directory");
    }

    if (file.write(camera.getfb(), FrameSize)) {
      Serial.println("Faild To write To The File");
    }

    file.close();

    StreamingHandler();
  }
 
  static void SwitchHandler(void) {

    ControlState = !ControlState;  // true for Car Control false for servo Control
    StreamingHandler();
  }

};
