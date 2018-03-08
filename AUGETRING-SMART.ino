
// POWERED BY ETHICS
/*-----( Import needed libraries )-----*/
#include <LiquidCrystal.h>
#include <Wire.h>
#include "RTClib.h"
#include <EEPROM.h>

// Declare RTC
RTC_DS1307 rtc;    // Create a RealTimeClock object

// Declare LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/*-----( Declare Variables )-----*/
const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 511;
char *buf;
int range_duration = 2;
String alarm;
String input;
String duration;
#define RELAY_OFF 1
#define RELAY_ON 0
#define Relay_1  8 

boolean eeprom_is_addr_ok(int addr)
{
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}

boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes)
{
  int i;

  if
    (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes))
  {
    return false;
  }

  for(i = 0; i < numBytes; i++) {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}

boolean eeprom_write_string(int addr, const char* string)
{
  int numBytes;
  numBytes = strlen(string) + 1;
  return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}

boolean eeprom_read_string(int addr, char* buffer, int bufSize)
{
  byte ch;
  int bytesRead;
  if(!eeprom_is_addr_ok(addr))
  {
    return false;
  }
  if(bufSize == 0)
  {
    return false;
  }
  if(bufSize == 1) {
    buffer[0] = 0;
    return true;
  }
  bytesRead = 0;
  ch = EEPROM.read(addr + bytesRead);
  buffer[bytesRead] = ch;
  bytesRead++;
  while( (ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR) )
  {
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch;
    bytesRead++;
  }
  if((ch != 0x00) && (bytesRead >= 1))
  {
    buffer[bytesRead - 1] = 0;
  }
  return true;
}

boolean splitData(String data, String time, char delimiter = ',') {
  int maxIndex = data.length()-1;
  int strIndex[] = {0, -1};
  
  for(int i=0; i <= maxIndex; i++){
    if(data.charAt(i) == delimiter || i == maxIndex){
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
        String val = data.substring(strIndex[0], strIndex[1]);
        val += ":00";
        if (val.equals(time)) {
          return true;
        }
    }
  }
  
  return false;
}

void setup()   /****** SETUP: RUNS ONCE ******/
{
  while (!Serial);
  Serial.begin(9600);

  /*RELAY*/
  pinMode(Relay_1, OUTPUT);   
  digitalWrite(Relay_1, RELAY_OFF);
  
  
  /*RTC SETUP*/
  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif

  rtc.begin(); // Start the RTC library code
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  rtc.adjust(DateTime(2017, 10, 14, 14, 40, 0));
  
  // LCD SETUP
  lcd.begin(16, 2);

}
//--(end setup )---

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  digitalWrite(Relay_1, RELAY_OFF); // set the Relay OFF
  
  // RTC LOOP
  DateTime now = rtc.now();  // Read data from the RTC Chip
  String zero_hour = now.hour() < 10 ? "0" : "";
  zero_hour += String(now.hour());
  String zero_minute = now.minute() < 10 ? "0" : "";
  zero_minute += String(now.minute());
  String zero_second = now.second() < 10 ? "0" : "";
  zero_second += String(now.second());
  String currentTime = zero_hour + ':'+ zero_minute + ':' + zero_second; // String of Current Time

//  Serial.println();
//  Serial.println("Sekarang Jam: " + currentTime); // Print Current Time
  
  // LCD LOOP
  lcd.setCursor(0, 1);
  lcd.print("JAM : " + currentTime);
  
  if(Serial.available())
  {
    // Bluetooth LOOP
    while(Serial.available() == 0){}
      
    while(Serial.available() > 0)
    {
      input = Serial.readString();
      char* input_buf = input.c_str();
      eeprom_write_string(0, input_buf);
    }
  }
  
  // EEPROM LOOP
  if (input.length() > 0) {
//    Serial.println("INPUT: "+ input);
    char* input_buf = input.c_str();
    boolean read_eeprom = eeprom_read_string(0, input_buf, strlen(input_buf)+1); // Read Time from EEPROM
    
    if (read_eeprom) {
      int idx = input.indexOf("|");
      alarm = input.substring(idx+1, input.length());
      duration = input.substring(0, idx);
      
//      Serial.println("alarm: "+ alarm);
//      Serial.println("duration: "+ duration);
      // LOOP For Checking Time
      boolean connect = splitData(alarm, currentTime, ',');
      
      if (connect){
        lcd.setCursor(0,0);
        lcd.print("MENYIRAM");
        digitalWrite(Relay_1, RELAY_ON); // set the Relay ON
        delay((duration.toInt() + range_duration) * 1000);
        lcd.print(" SELESAI");
        delay(1000);
        lcd.clear(); 
      }
    }
  }
  
  delay(1000); 
}
/*RTC END LOOP*/

//*********( THE END )***********
 
