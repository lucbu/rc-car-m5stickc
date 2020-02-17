#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

// Pin to go left
const int  out_1 = 13;
// Pin to go right
const int  out_2 = 12;
// Pin to go straight
const int  out_3 = 14;
// Pin to go backward
const int  out_4 = 27;

const char* ssid = "CarServer";
const char* password = "12345678";

using namespace websockets;

WebsocketsServer server;
WebsocketsClient client;

void setup() {  
//  Serial.begin(115200);

  pinMode(out_1, OUTPUT);
  pinMode(out_2, OUTPUT);
  pinMode(out_3, OUTPUT);
  pinMode(out_4, OUTPUT);

  WiFi.softAP(ssid, password);

//  Serial.println("");
//  Serial.println("WiFi connected");
//  Serial.println("IP address: ");
//  Serial.println(WiFi.softAPIP());
  
  server.listen(55);
  
//  Serial.print("Is server live? ");
//  Serial.println(server.available());
}

void loop() {
  if(!client.available()) {
    client = server.accept();
  }
  while(client.available()) {
    auto msg = client.readBlocking();
    
    // log
    //Serial.print("Got Message: ");
    //Serial.println(msg.data());
    handleReceivedMessage(msg.data());
  }
  
  delay(1000);
}

void handleReceivedMessage(String message){
  StaticJsonBuffer<500> JSONBuffer;
  JsonObject& parsed = JSONBuffer.parseObject(message);
 
  if (!parsed.success()) {
    //Serial.println("Parsing failed");
    return;
  }
 
  int motor = parsed["motor"];
  int steering = parsed["steering"];
  setMotor(motor);
  setSteering(steering);
}

void setMotor(int value) {
  if(value == 1) {
    digitalWrite(out_4, 0);
    digitalWrite(out_3, 1);
  } else if (value == -1) {
    digitalWrite(out_3, 0);
    digitalWrite(out_4, 1);
  } else {
    digitalWrite(out_4, 0);
    digitalWrite(out_3, 0);
  }
}
void setSteering(int value) {
  if(value == 1) {
    digitalWrite(out_1, 0);
    digitalWrite(out_2, 1);
  } else if (value == -1) {
    digitalWrite(out_2, 0);
    digitalWrite(out_1, 1);
  } else {
    digitalWrite(out_2, 0);
    digitalWrite(out_1, 0);
  }
}
