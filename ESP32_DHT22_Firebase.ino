#include <WiFi.h>
#include <FirebaseESP32.h>
#include "DHTesp.h"

// Temp
DHTesp dht;
int dhtPin = 17;

// Wifi & firebase
#define FIREBASE_HOST "FIREBASE_HOST" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "FIREBASE_HOST"
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"
FirebaseData firebaseData;
FirebaseJson json;
String databasePath = "/Temps";
TaskHandle_t wifiTask;

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

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void loop() {
  getTemperature();
  delay(50000);
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

	
  Serial.println(" T:" + String(newValues.temperature) + "°C H:" + String(newValues.humidity) +"%"  );
  saveData(newValues);
	return true;
}
/**  END  REGION TEMPERATURE */

/**  REGION FIREBASE */

void saveData(TempAndHumidity newValues){
   Serial.println("temp: C°" + String(newValues.temperature));
   json.clear().add("temperatureInCelcius" , String(newValues.temperature));
   json.add("Humidity",String(newValues.humidity));

 //  json.add("Timestamp","firebase.database.ServerValue.TIMESTAMP");

    //Also can use Firebase.push instead of Firebase.pushJSON
    //Json string is not support in v 2.6.0 and later, only FirebaseJson object is supported.
  

 //   String entryName = 
   // Serial.println("entryName = "+entryName ;
    //int timestp = firebaseData.getInt();
   // String newPath = String(databasePath + String(timestp));
    if (Firebase.pushJSON(firebaseData, databasePath, json))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.print("PUSH NAME: ");
      Serial.println(firebaseData.pushName());
      Serial.println("ETag: " + firebaseData.ETag());
      Serial.println("------------------------------------");
      Serial.println();
      String newPath = databasePath + String("/") + String(firebaseData.pushName());
      Serial.println(newPath);
     // FirebaseData timestampLabel = FirebaseData.
      if(Firebase.pushTimestamp(firebaseData, "/Timestamp")){

            Serial.println("ETag: " + Firebase.getETag(firebaseData, "/Timestamp/" + firebaseData.pushName()));
             Serial.println("------------------------------------");
               Serial.println();
               
         json.clear().add("timestamp",String(firebaseData.intData()));

         
         if(Firebase.pushJSON(firebaseData, newPath, json)){
                Serial.println(newPath);

                  Serial.println("------------------------------------");
                  Serial.println("PUSH SUCCESSFUL");

          }
        };
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    } 
  }
