// Motion Sensitive LCD Real-Time Clock/Alarm/Timer
// by Mike Soniat (msoniat@gmail.com)
// This sketch was created by integrating open source sketches from Adafruit and DFRobot.
// 7/15/2012

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

RTC_DS1307 RTC;
DateTime now;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define variables
int lcd_key = 0;
int adc_key_in = 0;
int lastDay = 0;
int lastMonth = 0;
int lastYear = 0;
int lastHour = 0;
int lastMinute = 0;
int movementTimer = 0;
int menuOptions = 3;
int menuOption = 0;
int alarmHours = 0;
int alarmMinutes = 0;
bool alarmPM = 0;
bool alarmSet = 0;
bool backLightOn = 1;

// define constants
const int backLight = 10; 
const int pirPin = 16;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define beeper A1
#define shortBeep 100
#define longBeep  500

void setup () {
  Serial.begin(57600);
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, LOW); // turn backlight off  
  pinMode(beeper, OUTPUT);
  digitalWrite(beeper, LOW);  
  pinMode(pirPin, INPUT);  
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop () {
  now = RTC.now();
  digitalClockDisplay( ); // update clock
  movementTimer++;
  if (movementTimer > 30) //turn off backlight after 30 cycles
  {
    digitalWrite(backLight, LOW); // turn backlight off
    movementTimer = 0;
  }
  for (int i = 0; i < 10000; i++)
  {
    button_loop(); //check for button pushed
    int val = digitalRead(pirPin); //read motion sensor
    if (val == HIGH)
    {
      //sense movement?
      digitalWrite(backLight, HIGH); // turn backlight on
      movementTimer = 0;
    }
  }
}

void printDigits(byte digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits,DEC);
}

void digitalClockDisplay()
{
  bool clockPM = 0;

  if (now.day() != lastDay)
  {
    lcd.begin(16,2);
    lcd.setCursor(3,0);

    if(now.month() < 10)
      lcd.print('0');
    lcd.print(now.month(), DEC);
    lcd.print("/");

    if(now.day() < 10)
      lcd.print('0');
    lcd.print(now.day(), DEC);
    lcd.print("/");

    int thisYear = now.year();
    lcd.print(thisYear, DEC);
  }

  if (now.minute() != lastMinute)
  {
    if(now.hour() < 10)
      lcd.setCursor(5,1);
    lcd.setCursor(4,1);

    if(now.hour() > 12)
    {
      lcd.print(now.hour()-12, DEC);
      printDigits(now.minute());
      clockPM = true;
      lcd.print(" PM");    
    }
    else
    {
      lcd.print(now.hour(), DEC);
      printDigits(now.minute());
      clockPM = false;      
      lcd.print(" AM");
    }
  }

  lastDay = now.day();
  lastMonth = now.month();
  lastYear = now.year();
  lastHour = now.hour();
  lastMinute = now.minute();

  //check for alarm
  if (alarmSet)
  {
    if (alarmHours == lastHour && alarmMinutes == lastMinute && alarmPM == clockPM)
    {
      //sound alarm
      setOffAlarm();
    } 
  }
}

void button_loop()
{
  int button = read_LCD_buttons();
  if (button == btnSELECT)
  {
    timedBeep(shortBeep,1); 
    selectMenu();
  }
}

void selectMenu()
{
  int button = 0; 
  menuOption = 1;
  lcdClear();
  lcd.print("Minute Timer");  

  while (menuOption <= menuOptions)
  {
    button = read_LCD_buttons();
    if (button == btnSELECT)
    {
      timedBeep(shortBeep,1);   
      menuOption++;

      if (menuOption == 2)
      {
        lcdClear();
        lcd.print("Set Alarm");            
      }
      if (menuOption == 3)
      {
        lcdClear();
        lcd.print("Set Date/Time");            
      }
    } 

    if (button == btnLEFT)
    {
      if (menuOption == 1)
      {
        timedBeep(shortBeep,1);
        minuteTimer();
        return;
      }
      if (menuOption == 2)
      {
        timedBeep(shortBeep,1);
        setAlarm();
        return;
      }
      if (menuOption == 3)
      {
        timedBeep(shortBeep,1);
        return;
      } 
    }
  }
}  

void minuteTimer()
{
  int timerMinutes = getTimerMinutes("Set Minutes", 0);
  if (timerMinutes > 0)
  {
    timedCountDown(timerMinutes*60, "Minute Timer");
  }
  else
  {
    timerCancelled("Timer");       
  }
  return;
}

void setAlarm()
{
  int button = 0;
  char *ampm = "AM";
  alarmHours = getTimerMinutes("Set Hour", alarmHours);  
  if (alarmHours > 0)
  {
    alarmMinutes = getTimerMinutes("Set Minutes", alarmMinutes);

    if (alarmMinutes > 0)
    {
      lcdClear();
      lcd.print("Toggle AM/PM");
      lcd.setCursor(0,1);
      //display alarm time
      lcd.print(alarmHours);       
      lcd.print(":");
      if (alarmMinutes < 10)
        lcd.print("0");
      lcd.print(alarmMinutes);
      lcd.setCursor(6,1);
      lcd.print(ampm);
      //get AM/PM
      button = 6;
      while (button != btnSELECT && button != btnRIGHT)
      {
        button = read_LCD_buttons();
        if (button == btnUP || button == btnDOWN)
        {
          timedBeep(shortBeep,1);
          if (ampm == "AM")
          {
            ampm = "PM";
          }
          else
          {
            ampm = "AM";
          }
          lcd.setCursor(6,1);
          lcd.print(ampm);         
        }
      }

      if (button == btnRIGHT)
      {
        timedBeep(shortBeep,1);
        alarmSet = true; 
        lcd.setCursor(0,0);
        lcd.print("Alarm Set for");
        delay(1000);
        return;       
      }
      else
      {
        timerCancelled("Alarm");
        return;  
      }   
    }
    else
    {
      timerCancelled("Alarm");     
    }    
  }     
  else
  {
    timerCancelled("Alarm");       
  }
}

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT; 
  if (adc_key_in < 195)  return btnUP;
  if (adc_key_in < 380)  return btnDOWN;
  if (adc_key_in < 555)  return btnLEFT;
  if (adc_key_in < 790)  return btnSELECT;  
  return btnNONE;  // when all others fail, return this...

}

void timedCountDown(int secondCount, char countLabel[])
{

  long seconds = 0;
  long minutes = 0; 

  lcdClear();
  lcd.print(countLabel);
  for (int i = secondCount; i >= 0; i--)
  {
    seconds = i;
    minutes = i / 60;
    if (minutes > 0)
    {
      seconds = seconds - (minutes * 60);  
    }     

    if (minutes > 0)
    {
      lcd.setCursor(0,1);
      lcd.print(minutes);
      lcd.print(" min ");
    }
    else
    {
      lcd.setCursor(0,1);
    }
    if (seconds < 10) lcd.print("0");
    lcd.print(seconds);
    lcd.print(" sec remaining");
    if (seconds > 0) delay(1000); 
    if (read_LCD_buttons() == btnSELECT) //cancel
    {
      timerCancelled("Timer");
      i = 0;
      return;
    }
  }
  lcd.setCursor(6,1);
  timedBeep(longBeep,3);
}

int getTimerMinutes(char timerText[], int startNum)
{
  int minutes = startNum;
  int button = 0;
  lcdClear();
  lcd.print(timerText);
  lcd.setCursor(0,1);
  lcd.print(minutes);   

  Serial.println("getTimerMinutes");

  while (button != btnSELECT)
  {
    button = read_LCD_buttons();
    Serial.println(button);
    if (button == btnLEFT)
    {
      timedBeep(shortBeep,1);
      minutes = minutes + 10;
    }
    if (button == btnUP)
    {
      timedBeep(shortBeep,1);
      minutes++;
    }
    if (button == btnDOWN && minutes > 0)
    {
      timedBeep(shortBeep,1);
      minutes--;
    }
    if (button == btnRIGHT)
    {
      timedBeep(shortBeep,1);
      return minutes; 
    }
    lcd.setCursor(0,1);
    lcd.print(minutes); 
    lcd.print("   ");
  }
  return 0;
}

void timedBeep(int beepTime, int beepCount)
{
  for (int i = 0; i < beepCount; i ++)
  {
    digitalWrite(beeper, HIGH);
    delay(beepTime);
    digitalWrite(beeper, LOW);
    delay(beepTime);
  }
}

void lcdClear(){
  lastDay = 0;
  lastMinute = 0;
  lcd.clear();
  lcd.begin(16,2);
  lcd.setCursor(0,0); 
}

void timerCancelled(char message[])
{
  lcdClear();
  lcd.print(message);
  lcd.print(" Cancelled");
  timedBeep(shortBeep,3);    
}

void setOffAlarm()
{
  int button = 0;
  int i = 0;
  Serial.println(i);
  digitalWrite(backLight, HIGH); // turn backlight on
  while (button != btnSELECT)
  {
    button = read_LCD_buttons();
    lcdClear();
    i++;
    if (i > 50)
    {
      lcdClear();
      lcd.print("Alert Alert");
      lcd.setCursor(0,1);
      lcd.print("     Alert Alert");      
      i = 0;
      timedBeep(shortBeep,3);
    }

  }     
  timerCancelled("Alarm"); 
  alarmSet = false;  
}




