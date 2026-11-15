#include <LiquidCrystal.h>

/*
 * GSM Based Fire Alert System
 * Board  : Arduino Uno
 * GSM    : SIM800L (3.7-4.2V supply, use a level shifter or voltage divider on RX)
 * Sensors: LM35 (temperature), MQ-2 (smoke), flame sensor module (digital)
 * Output : 16x2 I2C LCD, buzzer, red LED
 *
 * Wiring
 *   LM35 OUT     -> A0
 *   MQ-2 AO      -> A1
 *   Flame DO     -> D2   (LOW = flame detected on most modules)
 *   Buzzer       -> D8
 *   Red LED      -> D9
 *   SIM800L TX   -> D10  (Arduino RX via SoftwareSerial)
 *   SIM800L RX   -> D11  (Arduino TX, through 5V -> 3.3V divider)
 *   LCD SDA/SCL  -> A4/A5
 */

#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- Configuration ----------
const char *ALERT_NUMBERS[] = {"+910000000001", "+910000000002"};
const uint8_t NUMBER_COUNT = 2;

const float TEMP_THRESHOLD_C = 55.0;
const int SMOKE_THRESHOLD = 400;            // raw ADC value (0-1023), calibrate for your MQ-2
const unsigned long SMS_REPEAT_MS = 30000;  // repeat SMS while alarm lasts

// ---------- Pins ----------
const uint8_t PIN_TEMP = A0;
const uint8_t PIN_SMOKE = A1;
const uint8_t PIN_FLAME = 2;
const uint8_t PIN_BUZZER = 8;
const uint8_t PIN_LED = 9;

SoftwareSerial gsm(10, 11);  // RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2);

enum State { NORMAL, WARNING, ALARM };
State state = NORMAL;
unsigned long lastSms = 0;
bool smsSentOnce = false;

// ---------- GSM helpers ----------
String readGsm(unsigned long timeoutMs) {
  String resp = "";
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (gsm.available()) resp += (char)gsm.read();
  }
  return resp;
}

bool sendAt(const char *cmd, const char *expect, unsigned long timeoutMs = 1000) {
  gsm.println(cmd);
  String r = readGsm(timeoutMs);
  return r.indexOf(expect) >= 0;
}

bool initGsm() {
  gsm.begin(9600);
  delay(1000);
  if (!sendAt("AT", "OK")) return false;
  sendAt("ATE0", "OK");
  if (!sendAt("AT+CPIN?", "READY", 2000)) return false;
  if (!sendAt("AT+CMGF=1", "OK")) return false;  // text mode
  return true;
}

void sendSms(const char *number, const String &text) {
  gsm.print("AT+CMGS=\"");
  gsm.print(number);
  gsm.println("\"");
  delay(500);
  gsm.print(text);
  delay(200);
  gsm.write(26);  // Ctrl+Z
  readGsm(5000);
}

void sendAlerts(float t, int smoke, bool flame) {
  String msg = "FIRE ALERT!\nTemp: " + String(t, 1) + "C\nSmoke: " + String(smoke) +
               "\nFlame: " + (flame ? "YES" : "NO");
  for (uint8_t i = 0; i < NUMBER_COUNT; i++) {
    sendSms(ALERT_NUMBERS[i], msg);
    delay(1000);
  }
}

// ---------- Sensors ----------
float readTemperatureC() {
  // LM35: 10 mV per degree C, 5V reference, 10-bit ADC
  long sum = 0;
  for (uint8_t i = 0; i < 10; i++) {
    sum += analogRead(PIN_TEMP);
    delay(5);
  }
  float mv = (sum / 10.0) * (5000.0 / 1023.0);
  return mv / 10.0;
}

int readSmoke() {
  return analogRead(PIN_SMOKE);
}

bool readFlame() {
  return digitalRead(PIN_FLAME) == LOW;
}

// ---------- Setup / loop ----------
void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_FLAME, INPUT);
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.print("Fire Alert Sys");
  lcd.setCursor(0, 1);
  lcd.print("Starting GSM...");

  bool ok = initGsm();
  lcd.clear();
  lcd.print(ok ? "GSM Ready" : "GSM Error");
  delay(1500);
}

void loop() {
  float t = readTemperatureC();
  int smoke = readSmoke();
  bool flame = readFlame();

  bool hot = t >= TEMP_THRESHOLD_C;
  bool smoky = smoke >= SMOKE_THRESHOLD;

  if (flame || (hot && smoky)) state = ALARM;
  else if (hot || smoky) state = WARNING;
  else state = NORMAL;

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print((int)t);
  lcd.print("C S:");
  lcd.print(smoke);
  lcd.print("    ");
  lcd.setCursor(0, 1);

  switch (state) {
    case NORMAL:
      lcd.print("System Normal   ");
      digitalWrite(PIN_BUZZER, LOW);
      digitalWrite(PIN_LED, LOW);
      smsSentOnce = false;
      break;

    case WARNING:
      lcd.print("WARNING         ");
      digitalWrite(PIN_LED, HIGH);
      digitalWrite(PIN_BUZZER, HIGH);
      delay(100);
      digitalWrite(PIN_BUZZER, LOW);
      smsSentOnce = false;
      break;

    case ALARM:
      lcd.print("!! FIRE ALERT !!");
      digitalWrite(PIN_LED, HIGH);
      digitalWrite(PIN_BUZZER, HIGH);
      if (!smsSentOnce || millis() - lastSms >= SMS_REPEAT_MS) {
        lcd.setCursor(0, 1);
        lcd.print("Sending SMS...  ");
        sendAlerts(t, smoke, flame);
        lastSms = millis();
        smsSentOnce = true;
      }
      break;
  }

  Serial.print("T=");
  Serial.print(t);
  Serial.print(" Smoke=");
  Serial.print(smoke);
  Serial.print(" Flame=");
  Serial.print(flame);
  Serial.print(" State=");
  Serial.println(state);

  delay(500);
}
