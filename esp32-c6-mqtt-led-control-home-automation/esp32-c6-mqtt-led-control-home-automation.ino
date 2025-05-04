#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    ""
#define AIO_KEY         ""

// Pins
#define BUTTON_PIN D2  // D2
#define LED_PIN    D1  // D1

WiFiClient client;

// MQTT client
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Feeds
Adafruit_MQTT_Subscribe ledcontrol = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttoncontrol");
Adafruit_MQTT_Publish buttoncontrol = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/buttoncontrol");

bool lastButtonState = HIGH;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" connected!");

  // Setup MQTT
  mqtt.subscribe(&ledcontrol);
  connectMQTT();
}

void loop() {
  // Ensure MQTT is connected
  if (!mqtt.connected()) connectMQTT();
  mqtt.processPackets(10);
  mqtt.ping();

  // Handle incoming MQTT for LED control
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10))) {
    if (subscription == &ledcontrol) {
      String payload = (char *)ledcontrol.lastread;
      Serial.print("Received LED command: "); Serial.println(payload);
      if (payload == "1") {
        digitalWrite(LED_PIN, HIGH);
        ledState = true;
      } else if (payload == "0") {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
      }
    }
  }

  // Handle button press
  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    // Toggle LED
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    buttoncontrol.publish(ledState ? "ON" : "OFF");
    Serial.println("Button pressed - state sent to Adafruit IO");
    delay(200);  // Debounce
    lastButtonState = currentButtonState;
  }else if(currentButtonState == LOW && lastButtonState == HIGH){
    lastButtonState = LOW;
  }
  
}

void connectMQTT() {
  while (mqtt.connect() != 0) {
    Serial.println("MQTT connection failed, retrying...");
    delay(5000);
  }
  Serial.println("MQTT connected!");
}
