#include <Wire.h>
#include <RTClib.h>
#include "DST_RTC.h" // download from https://github.com/andydoro/DST_RTC
#include <Adafruit_NeoPixel.h>

// define pins
#define NEOPIN 3
#define NEOPIN2 4

#define STARTPIXEL 0 // where do we start on the loop? use this to shift the arcs if the wiring does not start at the "12" point

RTC_DS1307 rtc; // Establish clock object
DST_RTC dst_rtc; // DST object

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, NEOPIN, NEO_RGBW + NEO_KHZ800); // strip object
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(12, NEOPIN2, NEO_GRB + NEO_KHZ800); // strip object

byte pixelColorRed, pixelColorGreen, pixelColorBlue; // holds color values

// nighttime dimming constants
// brightness based on time of day- could try warmer colors at night?
#define DAYBRIGHTNESS 64
#define NIGHTBRIGHTNESS 20

// cutoff times for day / night brightness. feel free to modify.
#define MORNINGCUTOFF 7  // when does daybrightness begin?   7am
#define NIGHTCUTOFF 20 // when does nightbrightness begin? 8pm

long secondsInYear = 31557600; // How many seconds in a year
int flowerLEDCount = 12; // Middle Flower LED count

// Set up some variables
unsigned long anniversaryRemaining,birthdayRemaining;
long anniversaryYearCheckRemaining,birthdayYearCheckRemaining;
int anniversaryRemainingPercent,anniversaryLEDsToLight,birthdayRemainingPercent,birthdayLEDsToLight;
float anniversaryDecimal,birthdayDecimal;

int pixNum;
int toggle = 1;

void setup () {
  Serial.begin(57600);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

   Wire.begin();  // Begin I2C
   pinMode(NEOPIN, OUTPUT);
   pinMode(NEOPIN2, OUTPUT);


  if (! rtc.isrunning()) {
    //    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
    // DST? If we're in it, let's subtract an hour from the RTC time to keep our DST calculation correct. This gives us
    // Standard Time which our DST check will add an hour back to if we're in DST.
    DateTime standardTime = rtc.now();
    if (dst_rtc.checkDST(standardTime) == true) { // check whether we're in DST right now. If we are, subtract an hour.
      //standardTime = standardTime.unixtime() - 3600;
    }
    rtc.adjust(standardTime);
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
   //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));


  strip.begin();
  strip2.begin();
  //strip.show(); // Initialize all pixels to 'off'

  strip.setBrightness(DAYBRIGHTNESS); // set brightness
  strip2.setBrightness(10); // set brightness

  // startup sequence
  delay(500);
  colorWipe(strip.Color(255, 0, 0), 20); // Green
  colorWipe(strip.Color(0, 255, 0), 20); // Red
  colorWipe(strip.Color(0, 0, 255), 20); // Blue
  delay(500);
  // Colors are differnt, depends on if you have an RGBW or just RGB
  colorWipe2(strip2.Color(255, 0, 0), 30, 12); // Red
  colorWipe2(strip2.Color(0, 255, 0), 30, 12); // Green
  colorWipe2(strip2.Color(0, 0, 255), 30, 12); // Blue
  
  getAnniversaryData();

}

void loop () {


    // get time
  DateTime theTime = dst_rtc.calculateTime(rtc.now()); // takes into account DST

  byte secondval = theTime.second();  // get seconds
  byte minuteval = theTime.minute();  // get minutes
  int hourval = theTime.hour();   // get hours

  // change brightness if it's night time
  // check less often, once per minute
  if (secondval == 0) {
    if (hourval < MORNINGCUTOFF || hourval >= NIGHTCUTOFF) {
      strip.setBrightness(NIGHTBRIGHTNESS);
    } else {
      strip.setBrightness(DAYBRIGHTNESS);
    }

	// Toggle back and forth every minute
    if ( (toggle % 2) == 0) {

      getAnniversaryData();
      toggle = 1;

    } else {

      getBirthdayData();
      toggle = 2;
  
    }
  }

  hourval = hourval % 12; // This clock is 12 hour, if 13-23, convert to 0-11`

  hourval = (hourval * 60 + minuteval) / 12; //each red dot represent 24 minutes.

  // arc mode
  for (uint8_t i = 0; i < strip.numPixels(); i++) {

    if (i <= secondval) {
      // calculates a faded arc from low to maximum brightness
      pixelColorBlue = (i + 1) * (255 / (secondval + 1));
      //pixelColorBlue = 255;
    }
    else {
      pixelColorBlue = 0;
    }

    if (i <= minuteval) {
      pixelColorGreen = (i + 1) * (255 / (minuteval + 1));
      //pixelColorGreen = 255;
    }
    else {
      pixelColorGreen = 0;
    }

    if (i <= hourval) {
      pixelColorRed = (i + 1) * (255 / (hourval + 1));
      //pixelColorRed = 255;
    }
    else {
      pixelColorRed = 0;
    }

    strip.setPixelColor((i + STARTPIXEL) % 60, strip.Color(pixelColorRed, pixelColorGreen, pixelColorBlue));
  }

  //display
  strip.show();

  // printTheTime(theTime);

  // wait
  delay(1000);

}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    //strip.setPixelColor(i, c);
    strip.setPixelColor((i + STARTPIXEL) % 60, c);
    strip.show();
    delay(wait);
  }
}

void colorWipe2(uint32_t c, uint8_t wait, int pixNum) {
  for (uint16_t i = 0; i < pixNum; i++) {
    //strip.setPixelColor(i, c);
    strip2.setPixelColor((i + STARTPIXEL) % 60, c);
    strip2.show();
    delay(wait);
  }
}

void getAnniversaryData() {

    DateTime now = rtc.now();
 
	// This would be for the new year.  1/2/xxxx.  I pick the day after so it stays 100% lit for that day.
    DateTime anniversaryYearCheck(now.year(), 1, 2, 0, 0, 0);
    anniversaryYearCheckRemaining = (anniversaryYearCheck.unixtime() - now.unixtime());

    // If it already happened, add 1 to the year so we can figure out how long it is till that date in the future.
	if (anniversaryYearCheckRemaining < 0 ) {
      
      Serial.println("In the past");
      DateTime anniversary(now.year()+1, 8, 2, 0, 0, 0);
      anniversaryRemaining = (anniversary.unixtime() - now.unixtime()); 

    } else {

      DateTime anniversary(now.year(), 8, 2, 0, 0, 0);
      anniversaryRemaining = (anniversary.unixtime() - now.unixtime());
      Serial.println("In the future");
    
    }

	// Figure out in percent how much time till that date
    anniversaryRemainingPercent = (anniversaryRemaining*100)/secondsInYear;

    Serial.print("Anniversary Percent Remaining: ");
    Serial.println(anniversaryRemainingPercent);

    anniversaryDecimal = anniversaryRemainingPercent*0.01;
    anniversaryLEDsToLight = flowerLEDCount - round(anniversaryDecimal*flowerLEDCount); // How many LEDs to light up
    
    Serial.print("Anniversary LEDs to Light: ");
    Serial.println(anniversaryLEDsToLight);

    colorWipe2(strip2.Color(0, 0, 0), 30, 12); 
    colorWipe2(strip2.Color(255, 0, 15), 30,anniversaryLEDsToLight); 

    Serial.println();
}


void getBirthdayData() {

    DateTime now = rtc.now();
 	
	// This would be for Holloween.  11/1/xxxx.  I pick the day after so it stays 100% lit for that day.
    DateTime birthdayYearCheck(now.year(), 11, 1, 0, 0, 0);
    birthdayYearCheckRemaining = (birthdayYearCheck.unixtime() - now.unixtime());

    // If it already happened, add 1 to the year so we can figure out how long it is till that date in the future.
    if (birthdayYearCheckRemaining < 0 ) {
      
      Serial.println("In the past");
      DateTime birthday(now.year()+1, 5, 4, 0, 0, 0);
      birthdayRemaining = (birthday.unixtime() - now.unixtime()); 

    } else {

      DateTime birthday(now.year(), 5, 4, 0, 0, 0);
      birthdayRemaining = (birthday.unixtime() - now.unixtime());
      Serial.println("In the future");
    
    }

	// Figure out in percent how much time till that date
    birthdayRemainingPercent = (birthdayRemaining*100)/secondsInYear;

    Serial.print("Birthday Percent Remaining: ");
    Serial.println(birthdayRemainingPercent);

    birthdayDecimal = birthdayRemainingPercent*0.01;
    birthdayLEDsToLight = flowerLEDCount - round(birthdayDecimal*flowerLEDCount); // How many LEDs to light up
    
    Serial.print("Birhtday LEDs to Light: ");
    Serial.println(birthdayLEDsToLight);

    colorWipe2(strip2.Color(0, 0, 0), 30, 12); 
    colorWipe2(strip2.Color(0, 255, 0), 30,birthdayLEDsToLight); 
    
    Serial.println();
}
