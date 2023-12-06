#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Replace with your network credentials
const char* ssid = "ours";
const char* password = "tanyamahanin";

// Initialize Telegram BOT
#define BOTtoken "6983590652:AAGgFNx0BZLWGa2My8uk9vuu6xqxshx9ufA"  // your Bot Token (Get from Botfather)
#define CHAT_ID "1303585678"

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int sensorPhPin = 32;   // Change to the GPIO pin for pH sensor on ESP32
const int sensorSaltPin = 4; // Change to the GPIO pin for salt sensor on ESP32
const int pinSiramAir = 16;
const int pinSiramPupuk = 17;
float Po = 0;

void setup() {
  pinMode(sensorSaltPin, INPUT);
  pinMode(pinSiramAir, OUTPUT);
  pinMode(pinSiramPupuk, OUTPUT);
  Serial.begin(115200);

#ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
#endif

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
#endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  if (numNewMessages > 0) {
    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      if (chat_id != CHAT_ID) {
        bot.sendMessage(chat_id, "Unauthorized user", "");
        continue;
      }

      String text = bot.messages[i].text;
      if (text == "/get_ph") {
        float phValue = measurePH();
        String response = "Current pH value: " + String(phValue, 3);
        bot.sendMessage(chat_id, response, "");
      } else if (text == "/get_salt") {
        float saltValue = measureSalt();
        String response = "Current salt value: " + String(saltValue, 3) + "ppm";
        bot.sendMessage(chat_id, response, "");
      } else if (text == "/siram_air") {
        String siramValue = siramAir();
        bot.sendMessage(chat_id, siramValue, "");
      } else if (text == "/siram_pupuk") {
        String siramValue = siramPupuk();
        bot.sendMessage(chat_id, siramValue, "");
      }
    }
  }

  delay(1000); // Adjust the delay based on your needs
}

float measurePH() {
  int measurementPh = analogRead(sensorPhPin);
  Serial.print("ADC Value pH: ");
  Serial.println(measurementPh);
  float voltagePh = (measurementPh / 4095.0) * 3.3;  // Calculate voltage from ADC value
  Serial.print("Voltage pH: ");
  Serial.println(voltagePh, 3);

  // Modify the pH value calculation formula according to the new voltage reference
  Po = 7.00 + ((2.6 - voltagePh) / 0.17);

  Serial.print("Liquid pH Value: ");
  Serial.println(Po, 3);
  return Po;
}

float measureSalt() {
   // Add your logic to measure salt value
  // Placeholder implementation, replace with actual sensor reading
  // Default salt value for demonstration

  // Read the analog in value:
  int sensorValue = analogRead(sensorSaltPin);

  // Mathematical Conversion from ADC to conductivity (uSiemens)
  // Rumus berdasarkan datasheet
  float outputValueConductivity = (0.2142 * sensorValue) + 494.93;

  // Mathematical Conversion from ADC to TDS (ppm)
  // Rumus berdasarkan datasheet
  float outputValueTDS = (0.1126 * sensorValue) + 291.26;
  return outputValueTDS;
}

String siramAir() {
  // Mengaktifkan siram air (misalnya, memberikan tegangan tinggi ke pin siram air)
  digitalWrite(pinSiramAir, HIGH);

  // Menunda selama beberapa waktu (sesuai kebutuhan aplikasi Anda)
  delay(5000);  // Contoh: Siram air selama 5 detik

  // Menonaktifkan siram air
  digitalWrite(pinSiramAir, LOW);

  return "Berhasil menyiram";
}

String siramPupuk() {
  // Mengaktifkan siram Pupuk (misalnya, memberikan tegangan tinggi ke pin siram Pupuk)
  digitalWrite(pinSiramPupuk, HIGH);

  // Menunda selama beberapa waktu (sesuai kebutuhan aplikasi Anda)
  delay(5000);  // Contoh: Siram Pupuk selama 5 detik

  // Menonaktifkan siram Pupuk
  digitalWrite(pinSiramPupuk, LOW);

  return "Berhasil menyiram";
}