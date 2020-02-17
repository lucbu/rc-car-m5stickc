#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <M5StickC.h>

const int led = 10;
const int BTN_A = 37;
const int BTN_B = 39;

int btna_last = 0;
int btna_cur = 0;
int btnb_last = 0;
int btnb_cur = 0;

float base_accX = 0;
float base_accY = 0;
float base_accZ = 0;
float base_gyroX = 0;
float base_gyroY = 0;
float base_gyroZ = 0;
float base_temp = 0;

float accX = 0;
float accY = 0;
float accZ = 0;
float gyroX = 0;
float gyroY = 0;
float gyroZ = 0;
float temp = 0;

int motor = 0;
int steering = 0;

bool enableSend = false;

const char* ssid = "CarServer";
const char* password = "12345678";
const char* server = "ws://192.168.4.1:55/";

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
}

void onEventsCallback(WebsocketsEvent event, String data) {
}

WebsocketsClient client;

void connectWifi() {
    WiFi.begin(ssid, password);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(5,5);
    M5.Lcd.printf("Connecting to WiFi..");
    M5.Lcd.setCursor(5,12);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Lcd.print(".");
    }
}

void setup() {
    //Serial.begin(115200);
    M5.begin();
    M5.MPU6886.Init();
    M5.Lcd.setTextSize(1);
    M5.Lcd.setRotation(1);
    
    initOrientationData();
    
    pinMode(led, OUTPUT);
    digitalWrite(led, 1);
    
    connectWifi();
    
    M5.Lcd.fillScreen(BLACK);
    
    // Setup Callbacks
    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
    
    // Connect to server
    client.connect(server);
    drawDisplay();
}

void loop() {
  client.poll();
  if(WiFi.status() != WL_CONNECTED) {
    connectWifi();
    enableSend = false;
    initControl();
  }

  updateOrientationData();
  updateControl();
  
  handleBtnA();
  handleBtnB();
}

void sendControlData() {
  sendControlData(false);
}

void sendControlData(bool bypass) {
  char message[100];
  sprintf(message, "{motor: %d, steering: %d}", motor, steering);
  if(enableSend or bypass) {
    client.send(message);
  }
}

void initControl() {
  motor = 0;
  steering = 0;
  sendControlData(true);
}

void initOrientationData() {
  M5.MPU6886.getGyroData(&base_gyroX,&base_gyroY,&base_gyroZ);
  M5.MPU6886.getAccelData(&base_accX,&base_accY,&base_accZ);
  M5.MPU6886.getTempData(&base_temp);
}

void updateOrientationData() {
  M5.MPU6886.getGyroData(&gyroX,&gyroY,&gyroZ);
  M5.MPU6886.getAccelData(&accX,&accY,&accZ);
  M5.MPU6886.getTempData(&temp);
}

int getMotor(int delta) {
  int actual_delta = ((base_accY*1000) - (accY*1000));
  if(actual_delta > delta) {
    return 1;
  }
  if(actual_delta < -delta) {
    return -1;
  }
  if (-(delta-50) > actual_delta < (delta-50)) {
    return 0;
  }
  return motor;
}

int getSteering(int delta) {
  int actual_delta = ((base_accX*1000) - (accX*1000));
  if(actual_delta > delta) {
    return 1;
  }
  if(actual_delta < -delta) {
    return -1;
  }
  if(-(delta-50) > actual_delta < (delta-50)) {
    return 0;
  }
  return steering;
}

void updateControl() {
  int delta = 250;
  int newMotor = getMotor(delta);
  int newSteering = getSteering(delta);
  if(newMotor != motor or newSteering != steering) {
    motor = newMotor;
    steering = newSteering;
    drawDisplay();
    sendControlData(false);
  }
}

void resetDisplay() {
  M5.Lcd.fillScreen(BLACK);
}

void drawDisplay() {
  resetDisplay();
  showEnableSend();
  showDirection();
}

void showEnableSend() {
  M5.Lcd.setCursor(5, 5);
  if(enableSend) {
    M5.Lcd.printf("+");
  } else {
    M5.Lcd.printf("-");
  }
}

void showDirection() {
  if(motor > 0) {
    M5.Lcd.setCursor(50, 40);
    M5.Lcd.printf("<");
  } else if (motor < 0) {
    M5.Lcd.setCursor(100, 40);
    M5.Lcd.printf(">");
  }
  if(steering > 0) {
    M5.Lcd.setCursor(75, 20);
    M5.Lcd.printf("^");
  } else if (steering < 0) {
    M5.Lcd.setCursor(75, 60);
    M5.Lcd.printf("v");
  }
}
void handleBtnA() {
  btna_cur = digitalRead(BTN_A);

  if(btna_cur != btna_last){
    if(btna_cur==0){
      digitalWrite(led, 0);
      initOrientationData();
      initControl();
      digitalWrite(led, 1);
      drawDisplay();
    }
    btna_last = btna_cur;
  }
}
void handleBtnB() {
  btnb_cur = digitalRead(BTN_B);

  if(btnb_cur != btnb_last){
    if(btnb_cur==0){
      digitalWrite(led, 0);
      enableSend = !enableSend;
      drawDisplay();
      initControl();
      digitalWrite(led, 1);
    }
    btnb_last = btnb_cur;
  }
}

