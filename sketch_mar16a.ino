#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <dht11.h>

#define DHT11_PIN 6
#define MQ9_PIN 0

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

dht11 DHT;
int chk, fire_alarm_limit = 200, incomingByte, smoke_level;
String readStr, responseStr, mq9 = "1602182002581275", dht11 = "16021820025888550";
boolean lastAlarm = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
}

void loop() {
  lcd.clear();
  if (Serial.available()) {
    while (Serial.available() > 0) {
      incomingByte = Serial.read();
      readStr += (char) incomingByte;
    }
    processMessage(readStr);
  }
  readStr = "";
  smoke_level = analogRead(MQ9_PIN);
  if (smoke_level > fire_alarm_limit) {
    responseStr = "begin\n" + mq9 + ";";
    responseStr += smoke_level;
    responseStr += "\nend";
    Serial.print(responseStr);
    responseStr = "";
    lastAlarm = true;
  } else if (lastAlarm) {
    responseStr = "begin\n" + mq9 + ";";
    responseStr += smoke_level;
    responseStr += "\nend";
    Serial.print(responseStr);
    lastAlarm = false;
  }
  delay(2000);
}

void processMessage(String input) {
  lcd.print(input);
  lcd.setCursor(0, 1);
  String inputType = splitString(input, ' ', 0);
  if (inputType == "get_params") {
    chk = DHT.read(DHT11_PIN);
    responseStr = "begin\n";
    switch (chk)
    {
      case DHTLIB_OK: {
          responseStr += dht11 + ";";
          responseStr += DHT.temperature;
          responseStr += ",";
          responseStr += DHT.humidity;
          responseStr += "\n";
          lcd.print("T = ");
          lcd.print(DHT.temperature);
          lcd.print(", H = ");
          lcd.print(DHT.humidity);
          break;
        }
      case DHTLIB_ERROR_CHECKSUM:
        lcd.print("Checksum error,\t");
        break;
      case DHTLIB_ERROR_TIMEOUT:
        lcd.print("Time out error,\t");
        break;
      default:
        lcd.print("Unknown error,\t");
        break;
    }
    responseStr += mq9;
    responseStr += ";";
    responseStr += smoke_level = analogRead(MQ9_PIN);
    responseStr += "\n";
    responseStr += "end";
    Serial.print(responseStr);
    return;
  }
  if (inputType == "set_alarm_limit") {
    fire_alarm_limit = splitString(input, ' ', 1).toInt();
    lcd.print("input = " + splitString(input, ' ', 1));
    lcd.setCursor(0,1);
    lcd.print(fire_alarm_limit);
  }
}

String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
