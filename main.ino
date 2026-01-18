#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

// --- Configuration ---
const char* ssid = "YOUR WIFI NAME";
const char* password = "YOUR WIFI PASSWORD";
const char* webhookURL = "YOUR DISCORD WEBHOOK URL"; // <--- PASTE YOUR URL HERE

const int TRIGGER_PIN = 18; // GPIO 18 (D18)
const int DEBOUNCE_DELAY = 50; 
const int COOLDOWN_DELAY = 5000; // 5 Seconds cooldown

int lastSteadyState = HIGH;       
int lastFlickerableState = HIGH;  
unsigned long lastDebounceTime = 0;  

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.println("\n========================================");
  Serial.println("      ESP32 SOS SYSTEM - ACTIVE         ");
  Serial.println("========================================\n");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Hotspot [ANANYA]...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n[SYSTEM] WiFi Connected!");
  Serial.println("[SYSTEM] Ready for Trigger (Pin 18 to GND)");
  Serial.println("----------------------------------------");
}

void sendDiscordSOS() {
  if (WiFi.status() == WL_CONNECTED) {
    NetworkClientSecure *client = new NetworkClientSecure;
    client->setInsecure(); 
    
    HTTPClient http;
    Serial.println("[HTTP] Sending SOS Alert...");
    
    if (http.begin(*client, webhookURL)) {
      http.addHeader("Content-Type", "application/json");
      
      // Cleaned up the JSON string - removed the stray backslash
      String jsonPayload = "{\"content\": \"🚨 **SOS ALERT!** Button pressed.\"}";

      int httpResponseCode = http.POST(jsonPayload);
      
      if (httpResponseCode > 0) {
        Serial.printf("[HTTP] Response: %d\n", httpResponseCode);
        if(httpResponseCode == 204) Serial.println("[SUCCESS] Discord notified!");
      } else {
        Serial.printf("[ERROR] Failed: %s\n", http.errorToString(httpResponseCode).c_str());
      }
      http.end();
    }
    delete client;
  }
}
     

void loop() {
  int currentState = digitalRead(TRIGGER_PIN);

  if (currentState != lastFlickerableState) {
    lastDebounceTime = millis();
    lastFlickerableState = currentState;
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (lastSteadyState == HIGH && currentState == LOW) {
      Serial.println("\n[TRIGGER] SOS Triggered!");
      
      sendDiscordSOS(); // Send the message
      
      Serial.println("[COOLDOWN] Waiting 5 seconds...");
      delay(COOLDOWN_DELAY); // 5-second delay between presses
      Serial.println("[SYSTEM] Ready for next trigger.");
      
    }
    lastSteadyState = currentState;
  }
}