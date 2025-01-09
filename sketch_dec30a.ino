#include <SoftwareSerial.h>    // SoftwareSerial library
#include <dht11.h>               // DHT sensor library
#define DHT11PIN 2

// WiFi and Thingspeak configurations
String agAdi = "AGU-Student";  // Network name
String agSifresi = "NETWORK_PASSWORD"; // Network password
String ip = "THINGSPEAK_IP_ADDRESS"; // ThingSpeak IP address
String apiKey = "THINGSPEAK_API_KEY"; // ThingSpeak API Key

// Pin configurations
int rxPin = 10;                // ESP8266 RX pin
int txPin = 11;                // ESP8266 TX pin
const int ecgPin = A0;         // ECG sensor pin
//const int dhtPin = 2;          // DHT11 sensor pin
dht11 DHT11;

// Variables to store sensor readings
int ecgValue;
float temperature;

// Create a SoftwareSerial instance for ESP8266
SoftwareSerial esp(rxPin, txPin);

void setup() {  
  Serial.begin(9600);          // Start serial communication
  esp.begin(115200);           // Start communication with ESP8266
  //dht.begin();                 // Initialize DHT11 sensor
  
  Serial.println("Started");
  esp.println("AT");           // Send AT command to check the module

  while (!esp.find("OK")) {    // Wait for the ESP8266 to respond
    esp.println("AT");
    Serial.println("ESP8266 not found.");
  }
  Serial.println("ESP8266 Ready");

  esp.println("AT+CWMODE=1");  // Set ESP8266 as client mode
  while (!esp.find("OK")) {
    esp.println("AT+CWMODE=1");
    Serial.println("Setting mode...");
  }
  Serial.println("Mode set to client");

  // Connect to Wi-Fi
  Serial.println("Connecting to Wi-Fi...");
  esp.println("AT+CWJAP=\"" + agAdi + "\",\"" + agSifresi + "\"");
  while (!esp.find("OK")) {
    Serial.println("Failed to connect. Retrying...");
    delay(2000);
  }
  Serial.println("Connected to Wi-Fi.");
  delay(1000);
}

void loop() {
  // Read ECG value
  ecgValue = analogRead(ecgPin);
  Serial.println(ecgValue);

  // Read temperature from DHT11
  int chk = DHT11.read(DHT11PIN);
  Serial.println((float)DHT11.temperature, 2);
  temperature = (float)DHT11.temperature;

  // Connect to ThingSpeak server
  esp.println("AT+CIPSTART=\"TCP\",\"" + ip + "\",80");
  if (esp.find("Error")) {
    Serial.println("Connection error");
    return;
  }

  // Create HTTP GET request
  String veri = "GET /update?api_key=" + apiKey;
  veri += "&field1=" + String(ecgValue); // ECG data
  veri += "&field2=" + String(temperature); // Temperature data
  veri += "\r\n\r\n";

  // Notify ESP8266 about data length
  esp.print("AT+CIPSEND=");
  esp.println(veri.length() + 2);
  delay(2000);

  // Send data to ThingSpeak
  if (esp.find(">")) {
    esp.print(veri);
    Serial.println("Data sent: " + veri);
    delay(1000);
  } else {
    Serial.println("Failed to send data.");
  }

  // Close connection
  Serial.println("Closing connection.");
  esp.println("AT+CIPCLOSE");
  delay(2000);
}
