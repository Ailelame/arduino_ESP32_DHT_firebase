#include <NTPClient.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "DHTesp.h"

// Temp
DHTesp dht;
int dhtPin = 17;

int UPLOAD_DATA_INTERVAL = 60000;

// Wifi & firebase
#define FIREBASE_HOST "FIREBASE_HOST" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "FIREBASE_AUTH"
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"
FirebaseData firebaseData;
FirebaseJson json;
String databasePath = "/Temps";
String sensorId = "Cave à bière"; // Id of the sensor for Firbase 

// Sync NTC
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup()
{
  Serial.begin(115200);

  //temp
  dht.setup(dhtPin, DHTesp::DHT22);
  Serial.println("DHT initiated");

  //Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Firebase init
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  // Ntc
  timeClient.begin();
}

void loop() {
  getTemperature();
  delay(UPLOAD_DATA_INTERVAL);
}

/**    REGION TEMPERATURE */

bool getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    Serial.println("DHT22 error status: " + String(dht.getStatusString()));
    return false;
  }

  Serial.println("Reading values  =>  T:" + String(newValues.temperature) + "°C H:" + String(newValues.humidity) + "%");
  saveData(newValues);
  return true;
}
/**  END  REGION TEMPERATURE */

/**  REGION FIREBASE */

void saveData(TempAndHumidity newValues) {
  // Preparing Json
  Serial.println("------------------------------------");
  Serial.println("Preparing push firebase");
  json.clear().add("temperatureInCelcius" , String(newValues.temperature));
  json.add("sensorId", sensorId); 
  json.add("humidity", String(newValues.humidity));

  timeClient.update();
  long date = timeClient.getEpochTime();
  json.add("timestamp", String(date));

  // Pushing Json
  if (Firebase.pushJSON(firebaseData, databasePath, json)) {
    Serial.println("------------------------------------");
    Serial.println("PUSH SUCCESSFUL");
  } else {
    Serial.println("ERROR PUSHING JSON DATA");
  }
}
