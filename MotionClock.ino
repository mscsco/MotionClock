// Motion Sensitive LCD Real-Time Clock/Alarm/Timer
// by Mike Soniat (msoniat@gmail.com)
// This sketch was created by integrating open source sketches from Adafruit and DFRobot.
// 7/15/2012 Posted to github
// 7/29/2012 Fixed noon showing as 12am
// 7/29/2012 Fixed displaying PMM; added space after "PM"
// 7/29/2012 Added alarm set indicator
// 7/29/2012 Validate alarm hours > 0 and < 13
// 7/29/2012 Fixed allow alarm minutes to be 0
// 7/29/2012 Added setDateTime feature
// 7/29/2012 Added clearAlarm feature
// 8/1/2012 Fixed default day and hour settings on set date/time
// 8/1/2012 Pass maxCount to getTimerMinutes
// 8/1/2012 Fixed alarm set PM
// 8/19/2013 Fixed noon showing as 0:00pm

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
// 8/1/2012 Fixed default day and hour settings on set date/time
bool resetClock = false;

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
  // 8/1/2012 Fixed default day and hour settings on set date/time
  if (now.day() != lastDay || resetClock == true)
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
  // 8/1/2012 Fixed default day and hour settings on set date/time
  if (now.minute() != lastMinute || resetClock == true)
  {
    if(now.hour() < 10)
      lcd.setCursor(5,1);
    lcd.setCursor(4,1);

    // 7/29/2012 Fixed noon showing as 12am
    //    if(now.hour() > 12)
    if(now.hour() > 11)
    {
      // 8/19/2013 Fixed noon showing as 0:00pm
      if (now.hour() == 12)
      {
        lcd.print(now.hour(), DEC);
      }
      else{
        lcd.print(now.hour()-12, DEC);
      }
      printDigits(now.minute());
      clockPM = true;
      // 7/29/2012 Fixed displaying PMM; added space after "PM"
      lcd.print(" PM "); 
      // 7/29/2012 Added alarm set indicator
      if (alarmSet)
        lcd.print("*");
    }
    else
    {
      lcd.print(now.hour(), DEC);
      printDigits(now.minute());
      clockPM = false;      
      lcd.print(" AM ");
      // 7/29/2012 Added alarm set indicator
      if (alarmSet)
        lcd.print("*");      
    }
  }

  // 8/1/2012 Fixed default day and hour settings on set date/time
  resetClock = false;

  lastDay = now.day();
  lastMonth = now.month();
  lastYear = now.year();
  lastHour = now.hour();
  lastMinute = now.minute();

  //check for alarm
  if (alarmSet)
  {
    // 8/1/2012 Fixed alarm set PM
    if (alarmHours == lastHour && alarmMinutes == lastMinute)
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
        // 7/29/2012 Added clearAlarm feature
        lcd.print("Set/Clear Alarm");            
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
        // 7/29/2012 Added clearAlarm feature
        //check for existing alarm
        if (alarmSet)
        {
          clearAlarm();
        }
        else
        {
          setAlarm();
        }
        return;
      }
      if (menuOption == 3)
      {
        timedBeep(shortBeep,1);
        // 7/29/2012 Added setDateTime feature
        setDateTime();
        return;
      } 
    }
  }
}  

// 7/29/2012 Added clearAlarm feature
void clearAlarm()
{
  int button = 0;
  bool clearIt = true;
  char *ampm = "AM";

  lcdClear();
  lcd.print("Alarm Set For");
  lcd.setCursor(0,1);
  lcd.print(alarmHours);   
  lcd.print(":");
  lcd.print(alarmMinutes);
  lcd.print(" ");
  if (alarmPM == 1)
    ampm = "PM";
  lcd.print(ampm);
  delay(2000);
  lcdClear();
  lcd.print("Clear Alarm?");
  lcd.setCursor(0,1);
  lcd.print("Yes");  

  while (button != btnSELECT)
  {
    button = read_LCD_buttons();
    if (button == btnUP)
    {
      timedBeep(shortBeep,1);
      clearIt = !clearIt;
    }
    if (button == btnDOWN)
    {
      timedBeep(shortBeep,1);
      clearIt = !clearIt;
    }
    if (button == btnRIGHT)
    {
      timedBeep(shortBeep,1);
      alarmSet = !clearIt;
      if (clearIt)
      {
        lcdClear();
        timedBeep(shortBeep,2);
        lcd.print("Alarm Cleared!");
        delay(2000);
      }
      return; 
    }
    lcd.setCursor(0,1);
    if (clearIt)
    {
      lcd.print("Yes"); 
    }
    else{
      lcd.print("No ");
    }
  }   
}

void minuteTimer()
{
  // 8/1/2012 Pass maxCount to getTimerMinutes
  int timerMinutes = getTimerMinutes("Set Minutes", 0, 60);
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
  // 8/1/2012 Pass maxCount to getTimerMinutes
  alarmHours = getTimerMinutes("Set Alarm Hour", alarmHours, 12);  
  // 7/29/2012 Validate alarm hours > 0 and < 13
  if (alarmHours > 0 && alarmHours < 13)
  {
    // 8/1/2012 Pass maxCount to getTimerMinutes
    alarmMinutes = getTimerMinutes("Set Minutes", alarmMinutes, 59);
    // 7/29/2012 Fixed allow alarm minutes to be 0
    //if (alarmMinutes > 0)
    if (alarmMinutes < 60)
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
        // 8/1/2012 Fixed alarm set PM
        if (ampm == "PM") alarmHours += 12;        
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

// 7/29/2012 Added setDateTime feature
void setDateTime()
{
  int button = 0;
  char *ampm = "AM";

  //get month
  // 8/1/2012 Pass maxCount to getTimerMinutes
  int setMonth = getTimerMinutes("Set Month", lastMonth, 12);
  if (setMonth > 0 && setMonth < 13)
  {
    //get day
    // 8/1/2012 Fixed default day and hour settings on set date/time
    // 8/1/2012 Pass maxCount to getTimerMinutes
    int setDay = getTimerMinutes("Set Day", lastDay, 31);
    if (setDay > 0 && setDay < 32)
    {
      //get year
      // 8/1/2012 Pass maxCount to getTimerMinutes
      int setYear = getTimerMinutes("Set Year", lastYear, 2999);
      if (setYear > 2000 && setYear < 3000)
      {
        //get hour
        int thisHour = lastHour;
        // 8/1/2012 Fixed default day and hour settings on set date/time
        if (thisHour > 12) 
        {
          thisHour -= 12;
          ampm = "PM";
        }
        // 8/1/2012 Pass maxCount to getTimerMinutes
        int setHour = getTimerMinutes("Set Hour", thisHour, 12);
        if (setHour > 0 && setHour < 13)
        {
          //get minutes
          // 8/1/2012 Fixed default day and hour settings on set date/time
          // 8/1/2012 Pass maxCount to getTimerMinutes
          int setMinute = getTimerMinutes("Set Minute", lastMinute, 59);
          if (setMinute < 60)
          {
            //get ampm
            lcdClear();
            lcd.print("Toggle AM/PM");
            lcd.setCursor(0,1);
            //display alarm time
            lcd.print(setHour);       
            lcd.print(":");
            if (setMinute < 10)
              lcd.print("0");
            lcd.print(setMinute);
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
              if (ampm == "PM")
                setHour = setHour + 12;
              RTC.adjust(DateTime(setYear,setMonth,setDay,setHour,setMinute));

              lcd.setCursor(0,0);
              // 8/1/2012 Fixed default day and hour settings on set date/time
              lcd.print("Saving...     ");
              delay(1000);
              return;       
            }
            else
            {
              timerCancelled("");
              return;  
            }  
          }
          else
          {
            timerCancelled("");     
          }    
        }     
        else
        {
          timerCancelled("");       
        }
      }
      else
      {
        timerCancelled("");     
      }    
    }     
    else
    {
      timerCancelled("");       
    }
  }
  else
  {
    timerCancelled("");       
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

// 8/1/2012 Pass maxCount to getTimerMinutes
int getTimerMinutes(char timerText[], int startNum, int maxCount)
{
  int minutes = startNum;
  int button = 0;
  lcdClear();
  lcd.print(timerText);
  lcd.setCursor(0,1);
  lcd.print(minutes);   

  while (button != btnSELECT)
  {
    button = read_LCD_buttons();
    Serial.println(button);
    // 8/1/2012 Pass maxCount to getTimerMinutes
    if (button == btnLEFT)
    {
      if ((minutes + 10) <= maxCount)
      {
        timedBeep(shortBeep,1);
        minutes = minutes + 10;
      }
      else
      {
        timedBeep(shortBeep,2); 
      }
    }
    // 8/1/2012 Pass maxCount to getTimerMinutes
    if (button == btnUP)
    {
      if (minutes < maxCount)
      {
        timedBeep(shortBeep,1);
        minutes++;
      }
      else
      {
        timedBeep(shortBeep,2); 
      }
    }
    if (button == btnDOWN)
    {
      if (minutes > 0)
      {
        timedBeep(shortBeep,1);
        minutes--;
      }
      else
      {
        timedBeep(shortBeep,2); 
      }   
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
  // 8/1/2012 Fixed default day and hour settings on set date/time
  //lastDay = 0;
  //lastMinute = 0;
  resetClock = true;
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





















