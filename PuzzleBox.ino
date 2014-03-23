
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <TinyGPS.h>
#include <Servo.h>
#include "dots.h"

#define PIN_SERVO_POWER 12
#define PIN_SERVO_POSITION 11

#define PIN_SHUTDOWN 13

#define PIN_LCD_RS 2
#define PIN_LCD_RW 3
#define PIN_LCD_EN 4
#define PIN_LCD_D4 5
#define PIN_LCD_D5 6
#define PIN_LCD_D6 7
#define PIN_LCD_D7 8
#define PIN_LCD_BL 9

#define PIN_LOCK_1 0
#define PIN_LOCK_2 1

#define LOCK_VOLT_1 1024
#define LOCK_VOLT_2 512

#define SERVO_LOCK 900
#define SERVO_UNLOCK 2100

#define EEPROM_ATTEMPTS 0

#define MAX_ATTEMPTS 50

#define LOCATION_LAT 43.023311
#define LOCATION_LONG -81.237548
#define LOCATION_TOLERANCE 1000

#define STRING(str) #str

Servo servo;
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_RW, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
TinyGPS gps;
Dots dots(&lcd);

void setup()
{
  // Set up pins
  pinMode(PIN_SERVO_POWER, OUTPUT);
  pinMode(PIN_SERVO_POSITION, OUTPUT);
  pinMode(PIN_SHUTDOWN, OUTPUT);
  pinMode(PIN_LCD_BL, OUTPUT);

  digitalWrite(PIN_SERVO_POWER, LOW);
  digitalWrite(PIN_SHUTDOWN, LOW);

  servo.attach(PIN_SERVO_POSITION);
  servo.writeMicroseconds(SERVO_LOCK);

  // Enable serial
  Serial.begin(9600);

  // Set up LCD
  lcd.begin(16, 2);
  analogWrite(PIN_LCD_BL, 128);

  // Read and increment attempt counter
  unsigned char attempts = EEPROM.read(EEPROM_ATTEMPTS);
  attempts++;

  if(attempts >= MAX_ATTEMPTS)
  {
    setLcd("Maximum number", "of attempts!");
    digitalWrite(PIN_SHUTDOWN, HIGH);
  }

  EEPROM.write(EEPROM_ATTEMPTS, attempts);

  // Display 'Welcome'
  char msg[17];
  String strAttempts = "Attempt ";
  strAttempts += attempts + " of " STRING(MAX_ATTEMPTS);
  strAttempts.toCharArray(msg, 17);

  setLcd("    Welcome!    ", msg);
  delay(2000);
  lcd.clear();

  // Display 'Searching for signal...'
  setLcd("Searching for", "signal");
  dots.location(7, 2);
}

void loop()
{
  // Check to see if unlock circuit is hooked up
  int key1 = analogRead(PIN_LOCK_1);
  int key2 = analogRead(PIN_LOCK_2);
  if(within(key1, LOCK_VOLT_1, 50) && within(key2, LOCK_VOLT_2, 50))
  {
    // unlock the box
  }

  // Pump serial data into TinyGPS
  if(Serial.available() && gps.encode(Serial.read()))
  {
    float lat, lon;
    unsigned long age;

    gps.f_get_position(&lat, &lon, &age);

    // Check to see if we have aquired a GPS signal
    if(age != TinyGPS::GPS_INVALID_AGE)
    {
      // Calculate distance from here to target
      float distance = TinyGPS::distance_between(lat, lon, LOCATION_LAT, LOCATION_LONG);

      if(distance <= LOCATION_TOLERANCE)
      {
        digitalWrite(PIN_SERVO_POWER, HIGH);
        servo.writeMicroseconds(SERVO_UNLOCK);
        digitalWrite(PIN_SERVO_POWER, LOW);
        
        // Display congradulations
        setLcd("Congratulations!", "  Open the box  ");
      }
      else
      {
        setLcd("    Distance    ", "");
        lcd.setCursor(0, 2);

        if(distance < 1000)
        {
          lcd.print((int)(distance / 1000));
          lcd.print("km");
        }
        else
        {
          lcd.print((int)distance);
          lcd.print("m");
        }
      }

      delay(5000);
      digitalWrite(PIN_SHUTDOWN, HIGH);
    }
    else
    {
      // Flash dots
      dots.tick();
    }
  }

  if(millis() >= 300000)
  {
    setLcd("No signal found.", "Shutdown.");
    delay(2000);
    digitalWrite(PIN_SHUTDOWN, HIGH);
  }
}

void setLcd(char *top, char* bottom)
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(top);

  lcd.setCursor(0, 2);
  lcd.print(bottom);
}

bool within(int val, int target, int tolerance)
{
  return val < target + tolerance && val > target - tolerance;
}

