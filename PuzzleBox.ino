
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <TinyGPS.h>
#include <PWMServo.h>
#include "dots.h"

#define PIN_SERVO_POWER 12
#define PIN_SERVO_POSITION 10

#define PIN_SHUTDOWN 11

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

#define LOCK_VOLT_1 320 //1v56
#define LOCK_VOLT_2 713 //3v48

#define SERVO_LOCK 30
#define SERVO_UNLOCK 150

#define EEPROM_ATTEMPTS 0

#define MAX_ATTEMPTS 50

#define LOCATION_LAT 43.023311
#define LOCATION_LONG -81.237548
#define LOCATION_TOLERANCE 100

#define STRINGIFY(str) #str
#define STRING(str) STRINGIFY(str)

PWMServo servo;
PWMServo bl;
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_RW, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
TinyGPS gps;
Dots dots(&lcd);

void setup()
{
  // Need to make sure power to this pin is killed immediately
  // Otherwise, the box will turn itself off again
  pinMode(PIN_SHUTDOWN, OUTPUT);
  digitalWrite(PIN_SHUTDOWN, LOW);
    
  // Set up pins
  pinMode(PIN_SERVO_POWER, OUTPUT);
  pinMode(PIN_SERVO_POSITION, OUTPUT);
  pinMode(PIN_LCD_BL, OUTPUT);

  digitalWrite(PIN_SERVO_POWER, HIGH);

  servo.attach(PIN_SERVO_POSITION);
  servo.write(SERVO_LOCK);

  // Enable serial
  Serial.begin(9600);
  Serial.println("Initialised.");

  // Set up LCD
  lcd.begin(16, 2);
  analogWrite(PIN_LCD_BL, 255);
  //bl.attach(PIN_LCD_BL);
  //bl.write(1000);
  
  // Check to see if unlock circuit is hooked up
  int key1 = analogRead(PIN_LOCK_1);
  int key2 = analogRead(PIN_LOCK_2);
  if(within(key1, LOCK_VOLT_1, 20) && within(key2, LOCK_VOLT_2, 20))
  {
    // unlock the box
    setLcd("Backdoor!", "Unlocking...");
    EEPROM.write(EEPROM_ATTEMPTS, 0);
    Serial.print("Key1");
    Serial.println(key1);
    Serial.print("Key2");
    Serial.println(key2);
    digitalWrite(PIN_SERVO_POWER, LOW);
    servo.write(SERVO_UNLOCK);
    delay(5000);
    digitalWrite(PIN_SHUTDOWN, HIGH);
  }

  if(within(key1, LOCK_VOLT_2, 20) && within(key2, LOCK_VOLT_1, 20))
  {
    // Lock the box
    setLcd("Backdoor!", "Locking...");
    Serial.print("Key1");
    Serial.println(key1);
    Serial.print("Key2");
    Serial.println(key2);
    digitalWrite(PIN_SERVO_POWER, LOW);
    servo.write(SERVO_LOCK);
    delay(5000);
    digitalWrite(PIN_SHUTDOWN, HIGH);
  }

  // Read and increment attempt counter
  unsigned char attempts = EEPROM.read(EEPROM_ATTEMPTS);
  attempts++;

  if(attempts >= MAX_ATTEMPTS)
  {
    setLcd("Maximum number", "of attempts!");
    delay(5000);
    digitalWrite(PIN_SHUTDOWN, HIGH);
  }

  EEPROM.write(EEPROM_ATTEMPTS, attempts);

  // Display 'Welcome'
  setLcd("    Welcome!    ", "");
  lcd.print("Attempt ");
  lcd.print(attempts);
  lcd.print(" of " STRING(MAX_ATTEMPTS));
  delay(5000);
  lcd.clear();

  // Display 'Searching for signal...'
  setLcd("Searching for", "signal");
  dots.location(7, 2);
  dots.running(true);
}

void loop()
{
  dots.tick();

  // Pump serial data into TinyGPS
  if(Serial.available() && gps.encode(Serial.read()))
  {
    float lat, lon;
    unsigned long age;

    gps.f_get_position(&lat, &lon, &age);

    // Check to see if we have aquired a GPS signal
    if(age != TinyGPS::GPS_INVALID_AGE)
    {
      dots.running(false);

      // Calculate distance from here to target
      float distance = TinyGPS::distance_between(lat, lon, LOCATION_LAT, LOCATION_LONG);

      if(distance <= LOCATION_TOLERANCE)
      {
        digitalWrite(PIN_SERVO_POWER, LOW);
        servo.write(SERVO_UNLOCK);

        // Display congradulations
        setLcd("Congratulations!", "  Open the box  ");
        Serial.print("Latitude: ");
        Serial.println(lat);
        Serial.print("Longitude: ");
        Serial.println(lon);
        Serial.print("Distance: ");
        Serial.println(distance);
      }
      else
      {
        setLcd("    Distance    ", "");
        lcd.setCursor(0, 2);

        if(distance < 1000)
        {
          lcd.print(abs((int)(distance / 1000)));
          lcd.print("km");
        }
        else
        {
          lcd.print(abs((int)distance));
          lcd.print("m");
        }
      }

      delay(5000);
      digitalWrite(PIN_SHUTDOWN, HIGH);
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
  lcd.setCursor(0, 0);
  lcd.print(top);

  lcd.setCursor(0, 1);
  lcd.print(bottom);
}

bool within(int val, int target, int tolerance)
{
  return val < target + tolerance && val > target - tolerance;
}






