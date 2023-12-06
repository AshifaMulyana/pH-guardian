#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Ganti dengan kredensial jaringan Anda
const char *ssid = "terimakasih";
const char *password = "bismillah";

// Inisialisasi BOT Telegram
#define BOTtoken "6925371260:AAGRlzfOLBZUmGYjHm9jSbnezlTYNsgxih0" // Dapatkan dari BotFather
#define CHAT_ID "1756935119"

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Periksa pesan baru setiap 2 detik.
int botRequestDelay = 200;
unsigned long lastTimeBotRan;

bool statusPompaAir = false;
const int pinPompaAir = 27; // Ganti dengan pin GPIO yang sesuai pada ESP32

bool statusPompaPupuk = false;
const int pinPompaPupuk = 26;

// Konfigurasi Sensor pH
const int pinAnalogPh = A0;
const int jumlahPembacaan = 10;
int pembacaan[jumlahPembacaan];
int indeksSekarang = 0;
float faktorPelembutan = 0.5;

// Fungsi untuk mengecek nilai pH
float cekNilaiPh()
{
  pinMode(pinAnalogPh, INPUT);
  analogRead(pinAnalogPh);

  int nilaiSensorPh = analogRead(pinAnalogPh);
  pembacaan[indeksSekarang] = (faktorPelembutan * nilaiSensorPh) + ((1 - faktorPelembutan) * pembacaan[indeksSekarang]);
  indeksSekarang = (indeksSekarang + 1) % jumlahPembacaan;

  int total = 0;
  for (int i = 0; i < jumlahPembacaan; i++)
  {
    total += pembacaan[i];
  }
  int rataRata = total / jumlahPembacaan;

  float nilaiOutput = (-0.0693 * rataRata) + 7.3855;
  return nilaiOutput;
}

// Fungsi untuk mengirim notifikasi
void kirimNotifikasi(String pesan)
{
  String idChat = CHAT_ID;
  bot.sendMessage(idChat, pesan, "");
}

// Fungsi untuk menangani pesan baru
void tanganiPesanBaru(int jumlahPesanBaru)
{
  Serial.println("Menangani Pesan Baru");
  Serial.println(String(jumlahPesanBaru));

  for (int i = 0; i < jumlahPesanBaru; i++)
  {
    String idChat = String(bot.messages[i].chat_id);
    if (idChat != CHAT_ID)
    {
      bot.sendMessage(idChat, "Pengguna tidak diotorisasi", "");
      continue;
    }

    String teks = bot.messages[i].text;
    Serial.println(teks);

    String dariNama = bot.messages[i].from_name;

    if (teks == "/start")
    {
      String selamatDatang = "Selamat datang, " + dariNama + ".\n";
      selamatDatang += "Gunakan perintah berikut untuk mengendalikan perangkat Anda.\n\n";
      selamatDatang += "/cek_ph - Cek nilai pH\n";
      selamatDatang += "/hidupkan_pompa_air - Hidupkan pompa air\n";
      selamatDatang += "/matikan_pompa_air - Matikan pompa air\n";
      selamatDatang += "/hidupkan_pompa_pupuk - Hidupkan pompa pupuk\n";
      selamatDatang += "/matikan_pompa_pupuk - Matikan pompa pupuk\n";
      bot.sendMessage(idChat, selamatDatang, "");
    }

    if (teks == "/cek_ph")
    {
      float nilaiPh = cekNilaiPh();
      String pesanPh = "Nilai pH saat ini: " + String(nilaiPh, 2);
      bot.sendMessage(idChat, pesanPh, "");
    }

    if (teks == "/hidupkan_pompa_air")
    {
      if (!statusPompaAir)
      {
        statusPompaAir = true;
        digitalWrite(pinPompaAir, HIGH); // Aktifkan Pompa Air
        bot.sendMessage(idChat, "Pompa air telah dihidupkan.", "");
        kirimNotifikasi("Pompa air telah dihidupkan. Pin 2 Aktif.");
      }
      else
      {
        bot.sendMessage(idChat, "Pompa air sudah dalam keadaan hidup.", "");
      }
    }

    if (teks == "/matikan_pompa_air")
    {
      if (statusPompaAir)
      {
        statusPompaAir = false;
        digitalWrite(pinPompaAir, LOW); // Matikan Pompa Air
        bot.sendMessage(idChat, "Pompa air telah dimatikan.", "");
        kirimNotifikasi("Pompa air telah dimatikan. Pin 2 Mati.");
      }
      else
      {
        bot.sendMessage(idChat, "Pompa air sudah dalam keadaan mati.", "");
      }
    }

    if (teks == "/hidupkan_pompa_pupuk")
    {
      if (!statusPompaPupuk)
      {
        statusPompaPupuk = true;
        digitalWrite(pinPompaPupuk, HIGH); // Aktifkan Pompa Air
        bot.sendMessage(idChat, "Pompa pupuk telah dihidupkan.", "");
        kirimNotifikasi("Pompa pupuk telah dihidupkan. Pin 2 Aktif.");
      }
      else
      {
        bot.sendMessage(idChat, "Pompa pupuk sudah dalam keadaan hidup.", "");
      }
    }

    if (teks == "/matikan_pompa_pupuk")
    {
      if (statusPompaPupuk)
      {
        statusPompaPupuk = false;
        digitalWrite(pinPompaPupuk, LOW); // Matikan Pompa Air
        bot.sendMessage(idChat, "Pompa pupuk telah dimatikan.", "");
        kirimNotifikasi("Pompa pupuk telah dimatikan. Pin 2 Mati.");
      }
      else
      {
        bot.sendMessage(idChat, "Pompa pupuk sudah dalam keadaan mati.", "");
      }
    }

    // Tambahkan perintah BOT lainnya sesuai kebutuhan
  }
}

void setup()
{
  Serial.begin(115200);

#ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");
  client.setTrustAnchors(&cert);
#endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
#endif

  pinMode(pinPompaAir, OUTPUT); // Inisialisasi Pin Pompa Air
  digitalWrite(pinPompaAir, LOW); // Pastikan Pompa Air Mati pada Awalnya
  pinMode(pinPompaPupuk, OUTPUT); // Inisialisasi Pin Pompa Pupuk
  digitalWrite(pinPompaPupuk, LOW);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Menghubungkan ke WiFi..");
  }

  Serial.println(WiFi.localIP());
}

void loop()
{
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int jumlahPesanBaru = bot.getUpdates(bot.last_message_received + 1);

    while (jumlahPesanBaru)
    {
      Serial.println("Mendapatkan respons");
      tanganiPesanBaru(jumlahPesanBaru);
      jumlahPesanBaru = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}