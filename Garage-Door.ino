/*

    - GND - KEY - Resistor 220 Ohms - D1/GPIO5
    - Switch leg 1 - Resistor 10K Ohms-VCC
    - Switch leg 2 - D2/GPIO4 - GND


   Configuration (HA) : 
    switchs.yaml
      - platform: mqtt
        name: "NBPT switch1"
        state_topic: "NBPT/switch/state"
        command_topic: "NBPT/switch/set"
        payload_on: "ON"
        payload_off: "OFF"
        state_on: "ON"
        state_off: "OFF"
        optimistic: false
        qos: 0
        retain: true

   MQTT ESP8266 library:sketch->manage library->search and install “PubSubClient”


*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>



#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wifi: SSID and password
const char* WIFI_SSID = "hurd";
const char* WIFI_PASSWORD = "7487356660hrd";

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "NBPT_switch1_00001";
const PROGMEM char* MQTT_SERVER_IP = "192.168.101.79";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "homeassistant";
const PROGMEM char* MQTT_PASSWORD = "xee5phuqua6wei3eyahZineNg1equ4ausishaigieThah4Quee7keica3phahch5";

// MQTT: topics
const char* MQTT_SWITCH_STATE_TOPIC = "NBPT/switch/state";
const char* MQTT_SWITCH_COMMAND_TOPIC = "NBPT/switch/set";

// payloads by default (on/off)
const char* SWITCH_ON = "ON";
const char* SWITCH_OFF = "OFF";

// KEY:D1/GPIO5 SENOR:D2/GPIO4
const PROGMEM uint8_t KEY_PIN = 5;
const PROGMEM uint8_t BUTTON_PIN = 4;

boolean m_switch_state = false; // switch is turned off by default

int buttonState;             // the current reading from the input pin
int last_switch_state = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

WiFiClient wifiClient;
PubSubClient client(wifiClient);


// function called to publish the state of the switch (on/off)
void publishSwitchState() {
  if (m_switch_state) {
    client.publish(MQTT_SWITCH_STATE_TOPIC, SWITCH_ON, true);
  } else {
    client.publish(MQTT_SWITCH_STATE_TOPIC, SWITCH_OFF, true);
  }
}

// function called to turn on/off the switch
void setSwitchState() {
    digitalWrite(KEY_PIN, HIGH);
    Serial.println("INFO: Turn switch on...");
    client.publish(MQTT_SWITCH_STATE_TOPIC, SWITCH_ON, true);
    delay(500);
    digitalWrite(KEY_PIN, LOW);
    Serial.println("INFO: Turn switch off...");
    client.publish(MQTT_SWITCH_STATE_TOPIC, SWITCH_OFF, true);
/*  if (m_switch_state) {
    digitalWrite(KEY_PIN, HIGH);
    Serial.println("INFO: Turn switch on...");
  } else {
    digitalWrite(KEY_PIN, LOW);
    Serial.println("INFO: Turn switch off...");
  }*/
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  
  // handle message topic
  if (String(MQTT_SWITCH_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(SWITCH_ON))) {
      if (m_switch_state != true) {
        m_switch_state = true;
        setSwitchState();
        //publishSwitchState();
      }
    } else if (payload.equals(String(SWITCH_OFF))) {
      if (m_switch_state != false) {
        m_switch_state = false;
        setSwitchState();
        //publishSwitchState();
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
      // Once connected, publish an announcement...
      publishSwitchState();
      // ... and resubscribe
      client.subscribe(MQTT_SWITCH_COMMAND_TOPIC);
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void ReadPin() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(BUTTON_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != last_switch_state) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != m_switch_state) {
      m_switch_state = reading;
      publishSwitchState();

      // only toggle the LED if the new button state is HIGH
 //     if (buttonState == HIGH) {
 //       ledState = !ledState;
 //     }
    }
  }
  // set the LED:
 // digitalWrite(ledPin, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  last_switch_state = reading;
}
void setup() {
  // init the serial
  Serial.begin(115200);

  // init the led
  pinMode(BUTTON_PIN, INPUT);
  pinMode(KEY_PIN, OUTPUT);
  analogWriteRange(255);
  //setSwitchState();

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.print("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  ReadPin();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
