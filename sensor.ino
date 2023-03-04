#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define RELAY_FAN_PIN 22 // Arduino pin connected to relay which connected to fan
#define DHTPIN 23           // Arduino pin connected to relay which connected to DHT sensor
#define DHTTYPE DHT11

float SETTING_TEMP = 25.0; // upper threshold of temperature, change to your desire value

DHT dht(DHTPIN, DHTTYPE);

float temperature;    // temperature in Celsius

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

void setup()
{
  Serial.begin(115200); // initialize serial
  dht.begin();        // initialize the sensor
  pinMode(RELAY_FAN_PIN, OUTPUT); // initialize digital pin as an output

  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  if (strcmp(topic, "output") == 0){
    char* str = (char* )payload;
    str[length]= '\0'; 
    Serial.println("");

    char* token = strtok(str, ",");
    
    while (token != NULL) {
      float n = atoi(token);
      SETTING_TEMP = n;
      Serial.print("setting temperature: ");
      Serial.print(n);
      Serial.println("°C");
      token = strtok(NULL, ";");
    }
  }
}


void reconnect(){
  while (!client.connected()){
    Serial.print("trying to connect MQTT...");
    if (client.connect("ESP8266Client")){ 
      Serial.println("connected!");
      client.subscribe("output");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("  try again");
      delay(5000);
    }
  }
}


void loop()
{

  if (!client.connected()){
    reconnect();
  }
  client.loop();
  
  delay(2000);

  temperature = dht.readTemperature();;  // read temperature in Celsius
  
  char tempString[8];
  dtostrf(temperature, 1, 2, tempString);
  client.publish("current", tempString);  
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    if(temperature > SETTING_TEMP){
      Serial.println("The fan is turned on");
      digitalWrite(RELAY_FAN_PIN, HIGH); // turn on
      Serial.print(temperature);
      Serial.println(F("°C "));
    } else{
      Serial.println("The fan is turned off");
      digitalWrite(RELAY_FAN_PIN, LOW); // turn off
      Serial.print(temperature);
      Serial.println(F("°C "));
    }
  }
}
