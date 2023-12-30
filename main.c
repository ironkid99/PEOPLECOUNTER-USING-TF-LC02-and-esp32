#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqttServer = ""; 
const char* topic = "/v1.6/devices/esp32";   

const int sensorTxPin = 17;
const int sensorRxPin = 16;

int peopleCount = 0;
bool personInFOV = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, sensorRxPin, sensorTxPin);
  delay(1000);
  Serial.println("Device initialized");


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);

  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if(!client.connected()) {
    reconnect();
  }
  
  byte command[] = {0x55, 0xAA, 0x81, 0x00, 0xFA};
  Serial2.write(command, sizeof(command));
  delay(100);
  
  while (Serial2.available()) {
    byte data = Serial2.read();

    if (data == 0x55) {
      data = Serial2.read();
      if (data == 0xAA) {
        int distance = Serial2.read() | (Serial2.read() << 8);
        byte status = Serial2.read();
        Serial.println(status);
        if (status == 0x00 || status == 0x01 || status == 0x02 || status == 0x03 || status == 0x04 || status == 0x05) {
          if (!personInFOV) {
            peopleCount++;
            personInFOV = true;
            Serial.print("People Count: ");
            Serial.println(peopleCount);
            sendToServer(peopleCount);
          }
        } else {
          Serial.println("No person detected: ");
          personInFOV = false;
        }
      }
    }
  }
  delay(100);
  client.loop();
}
void sendToServer(int count) {
  char payload[50];
  snprintf(payload, sizeof(payload), "{\"people_count\":%d}", count);

  client.publish(topic, payload);

  Serial.println("Data sent to Ubidots: ");
  Serial.println(payload);
}

void reconnect() {
  while(!client.connected()) {
    Serial.println("MQTT reconnecting.....");
    if(client.connect("ESP32Client")){
      Serial.println("Connected to MQTT");
    }
    else{
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(client.state());
      Serial.println("Retrying in 5 seconds");
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
;
}
