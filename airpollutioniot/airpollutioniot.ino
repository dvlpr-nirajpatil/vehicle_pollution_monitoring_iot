// Include required libraries
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <LiquidCrystal_I2C.h>
#include<wire.h>
#include <TimeLib.h>


// Define DHT sensor parameters
#define mqPin A0

// Define WiFi credentials
#define WIFI_SSID "patils"
#define WIFI_PASSWORD "patils@221976"

// Define Firebase API Key, Project ID, and user credentials
#define API_KEY "AIzaSyASUrcasjS6raxm4GDywW_zVBAj-TOFUFA"
#define FIREBASE_PROJECT_ID "vehiclepollutionmonitori-8c43d"
#define USER_EMAIL "rajranka@gmail.com"
#define USER_PASSWORD "123456789"

LiquidCrystal_I2C lcd(0x3F, 16, 2);

// Define Firebase Data object, Firebase authentication, and configuration
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup()
{
    // Initialize serial communication for debugging
    Serial.begin(115200);
    Wire.begin(4, 5);
    setTime(0);

    lcd.begin(16, 2);
    lcd.backlight();
    lcd.home();

    lcd.print("IoT base ");
    lcd.setCursor(0, 1);
    lcd.print("Air Pollution");

    delay(2000); //
    delay(10);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Detection &");
    lcd.setCursor(0, 1);
    lcd.print("Monitoring");
    delay(2000);
    lcd.clear();
    // Connect to Wi-Fi
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
    Serial.println();

    // Print Firebase client version
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // Assign the API key
    config.api_key = API_KEY;

    // Assign the user sign-in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // Assign the callback function for the long-running token generation task
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

    // Begin Firebase with configuration and authentication
    Firebase.begin(&config, &auth);

    // Reconnect to Wi-Fi if necessary
    Firebase.reconnectWiFi(true);
}

void loop()
{
    // Define the path to the Firestore document
    String documentPath = "Area/GangapurRoad";

    // Create a FirebaseJson object for storing data
    FirebaseJson content;

    float sensorValue = analogRead(mqPin);
    unsigned long currentTimestamp = millis();

  // Format timestamp as DateTime string
    char dateTimeString[20];
    sprintf(dateTimeString, "%04d-%02d-%02dT%02d:%02d:%02d",
            year(), month(), day(), hour(), minute(), second());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Air Pollution");
    lcd.setCursor(0, 1);
    lcd.print(sensorValue);
    delay(3000);

    // Print temperature and humidity values
    Serial.println(sensorValue);
    Serial.println(dateTimeString);
    // Check if the values are valid (not NaN)
    if (!isnan(sensorValue) && !isnan(*dateTimeString))
    {
        // Set the 'Temperature' and 'Humidity' fields in the FirebaseJson object
        content.set("fields/ppm/stringValue", String(sensorValue, 2));
        content.set("fields/last_updated/stringValue", String(dateTimeString));
        Serial.print("Update/ Add PPM... ");

        // Use the patchDocument method to update the Temperature and Humidity Firestore document
        if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "ppm") && Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "last_updated"))
        {
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }
    }
    else
    {
        Serial.println("Failed to read PPM data.");
    }

    // Delay before the next reading
    delay(5000);
}