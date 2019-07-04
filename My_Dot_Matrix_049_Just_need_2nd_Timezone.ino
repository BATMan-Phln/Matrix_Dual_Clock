
// Use the DHT22 temp and humidity sensor
//#define USE_DHT22 1

// Use the DS1307 clock module
#define USE_DS1307 1

// Header file includes
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "Font_Data.h"

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 8

#define CLK_PIN   13        //WHITE
#define DATA_PIN  11        //ORANGE
#define CS_PIN    10        //YELLOW

#define PRINT(x)

//#if USE_DHT22

#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
//dht DHT;
//#endif

#if USE_DS1307
#include <MD_DS1307.h>
//#include <DS1307RTC.h>   // https://github.com/PaulStoffregen/DS1307RTC
#include <Timezone.h>    // https://github.com/JChristensen/Timezone

#include <Wire.h>
#endif

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
// MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define SPEED_TIME  75
#define PAUSE_TIME  0

#define ZULU_PAUSE_TIME  1500
#define TEXT_PAUSE_TIME   1000

#define MAX_MESG  20

// Turn on debug statements to the serial output
#define DEBUG 0

// Global variables
char szTime[9];    // mm:ss\0
char szMesg[MAX_MESG+1] = "";      
//char zuluTime;
//char hour;
char h;

uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C  
uint8_t degF[] = { 6, 3, 3, 124, 20, 20, 4 }; // Deg F

char *mon2str(uint8_t mon, char *psz, uint8_t len)
// Get a label from PROGMEM into a char array
{
  static const char str[][4] PROGMEM =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  *psz = '\0';
  mon--;
  if (mon < 12)
  {
    strncpy_P(psz, str[mon], len);
    psz[len] = '\0';
  }

  return(psz);
}

char *dow2str(uint8_t code, char *psz, uint8_t len)
{
  static const char str[][4] PROGMEM =
  {
     "Sun", "Mon", "Tue", "Wed",
    "Thu", "Fri", "Sat"
  };

  *psz = '\0';
  code--;
  if (code < 7)
  {
    strncpy_P(psz, str[code], len);
    psz[len] = '\0';
  }

  return(psz);
}


void getTime(char *psz, bool f = true)
// Code for reading clock time
{

  RTC.readTime();
  sprintf(psz, "%02d%c%02d", RTC.h, (f ? ':' : ' '), RTC.m);

}

void getDate(char *psz)
// Code for reading clock date
{
#if	USE_DS1307
  char  szBuf[10];
  
  RTC.readTime();
  sprintf(psz, "%s %d", mon2str(RTC.mm, szBuf, sizeof(szBuf)-1), RTC.dd);       // sprintf(psz, "%s %d %04d", mon2str(RTC.mm, szBuf, sizeof(szBuf)-1), RTC.dd, RTC.yyyy);

#endif
}

void setup(void)
{


/////////// TESTING TEMP SENSOR
  Serial.begin(9600);
  Serial.println("DHT");

  dht.begin();
/////////// END TEMP SENSOR

  
  P.begin(2);
  P.setInvert(false);
  P.setIntensity(6);

  P.setZone(0, 0, MAX_DEVICES-5);
  P.setZone(1, MAX_DEVICES-4, MAX_DEVICES-1);
  P.setFont(1, numeric7Seg);

  P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
 //  COMMENT ABOVE and UNCOMMENT HERE LATER   P.displayZoneText(0, szMesg, PA_CENTER, SPEED_TIME, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
P.displayZoneText(0, szMesg, PA_CENTER, SPEED_TIME, ZULU_PAUSE_TIME, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  P.addChar('$', degC);       //NOT NEEDED
  P.addChar('&', degF);

#if USE_DS1307
  RTC.control(DS1307_CLOCK_HALT, DS1307_OFF);
  RTC.control(DS1307_12H, DS1307_OFF);
#endif

 // getTime(szTime);
}

void loop(void)
{
 
////////// TRYING TO FIX TEMP SENSOR

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temp = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // float fern = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
            if (isnan(hum) || isnan(temp)) {           //  if (isnan(hum) || isnan(temp)) {     
    return;
  }                ////////THIS MAKES THE SKETCH STOP FOR A MOMENT - WHY???

//////// END TEMP SENSOR
  
  static uint32_t lastTime = 0; // millis() memory
  static uint8_t  display = 0;  // current display mode
  static bool flasher = false;  // seconds passing flasher

float frnt = (temp*1.8+32);           //Changed int to float to get decimals - WORKS

  P.displayAnimate();

  P.setIntensity(0);                  //Intensity - Change with time later
  if (P.getZoneStatus(0))
  {
    switch (display)
    {
      case 0: //SET AS ZULU TIME 
        P.setTextEffect(0, PA_FADE, PA_FADE);
        display++;  

        /////////Now I just need to stop the glitch
        /////////and add 420 to time (ZULU)

       getTime((szMesg), "z"); 
 
   ///////////I've tried to get this to flash like the other clock but it
   ///////////isn't working 
        break;
                                            
      case 1: // Temperature deg C
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_UP_LEFT);
        display++;
        
          dtostrf(temp, 3, 1, szMesg);
          strcat(szMesg, "$");
     
        break;
 
      case 2: // Temperature deg F
        P.setTextEffect(0, PA_SCROLL_UP_LEFT, PA_SCROLL_LEFT);
        display++;
        
          dtostrf(frnt, 3, 1, szMesg);
          strcat(szMesg, "&");

        break;

      case 3: // Relative Humidity
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;

          dtostrf(hum, 3, 0, szMesg);
          strcat(szMesg, "% H ");

        break;
  

      case 4: // day of week  //////THIS ONLY SHOWS WEDNESDAY - Today's Tuesday
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;


        dow2str(RTC.dow, szMesg, MAX_MESG);

        break;


      default:  // Calendar
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display = 0;
        getDate(szMesg);
      break; 
    }

    P.displayReset(0);
  }
  // Finally, adjust the time string if we have to
  if (millis() - lastTime >= 1000)
  {
    lastTime = millis();
   getTime(szTime, flasher);
    flasher = !flasher;
    P.displayReset(1);
  }
}
