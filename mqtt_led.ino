// NeoPixelTest
// This example will cycle between showing four pixels as Red, Green, Blue, White
// and then showing those pixels as Black.
//
// Included but commented out are examples of configuring a NeoPixelBus for
// different color order including an extra white channel, different data speeds, and
// for Esp8266 different methods to send the data.
// NOTE: You will need to make sure to pick the one for your platform 
//
//
// There is serial output of the current state so you can confirm and follow along
//

#include <NeoPixelBrightnessBus.h>
#include "WiFi.h"
#include <PubSubClient.h>

// TODO: Change this to the number of LEDs in your LED strip
const uint16_t PixelCount = 20;
// TODO: Change this to the GPIO pin you will connect to the LED strip
const uint8_t PixelPin = 17;
#define colorSaturation 128
#define WIFI_TIMEOUT_MS 15000

// three element pixels, in different order and speeds
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
//NeoPixelBus<NeoRgbFeature, Neo400KbpsMethod> strip(PixelCount, PixelPin);

// four element pixels, RGBW
NeoPixelBrightnessBus<NeoGrbwFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

RgbColor color []= {red, green, blue, white, black};

// TODO: Replace with your network credentials
const char* ssid     = "Wifi_Name";
const char* password = "Wifi_Password";

// TODO: Replace with your MQTT Broker IP address
const char* mqtt_server = "0.0.0.0";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;

RgbColor values[PixelCount];



void connectToWiFi(){
 // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void setup()
{
    Serial.begin(115200);
    connectToWiFi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();

    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();
    

    Serial.println();
    Serial.println("Running...");
}


void callback(char* topic, byte* message, unsigned int length) {
  char messageTemp[length+10];
  
  for (int i = 0; i < length; i++) {
    messageTemp[i] = (char)message[i];
  }

  if (String(topic) == "esp32/output") {
    const char s[2] = ":";
    const char t[2] = ",";
    char *end_str;
    char* token_1 = strtok_r(messageTemp, s, &end_str);
    int index = 0;
    while (token_1 != NULL){
      char *end_token;
      char* token_2 = strtok_r(token_1, t, &end_token);
      int counter = 0;
      int rgb_values[3]; 
      while(token_2 != NULL){
        rgb_values[counter] = atoi(token_2);
        counter++;
        token_2 = strtok_r(NULL, t, &end_token); 
      }
      values[index] = RgbColor(rgb_values[0], rgb_values[1], rgb_values[2]);
      index++;
      token_1 = strtok_r(NULL, s, &end_str);
    }
    
    for (int i = 0; i < 450; i++){
      strip.SetPixelColor(i, values[i]);
    }
    strip.Show();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32_client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
  }
}
