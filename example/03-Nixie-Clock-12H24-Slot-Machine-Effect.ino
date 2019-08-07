// Nixie Clock Thermometer Hygrometer Arduino Shield by Marcin Saj https://nixietester.com
// https://github.com/marcinsaj/Nixie-Clock-Thermometer-Hygrometer-Arduino-Shield
//
// 12/24H Nixie Clock & Slot Machine Effect Example
// 
// This example demonstrates how to set the RTC time, read time from RTC and display on nixie tubes.
// The "Slot Machine Effect" is activated on each full minute.
// The user has the choice of 12 or 24 hour display format.
// Serial monitor is required to display basic options.
// DS3231 RTC datasheet: https://datasheets.maximintegrated.com/en/ds/DS3231.pdf
// HV5812 nixie tube driver datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20005629A.pdf
//
// Hardware:
// Arduino Uno & Nixie Clock Thermometer Hygrometer Arduino Shield 
// Schematic: http://bit.ly/Nixie-Clock-Thermometer-Hygrometer-Arduino-Shield-Schematic
// EN        connected to Arduino pin A2
// DIN       connected to Arduino pin 12
// CLK       connected to Arduino pin A1
// STR       connected to Arduino pin A0
// DATA      connected to Arduino pin A3
// DOT       connected to Arduino pin 4
// RTC SDA   connected to Arduino pin SDA
// RTC SCL   connected to Arduino pin SCL

#include <DS3232RTC.h>  // https://github.com/JChristensen/DS3232RTC
                        // Also install the Time library if needed: https://github.com/PaulStoffregen/Time 

// Choose Time Format
#define HH    12        // 12 Hour Clock or 24 Hour Clock
          
#define EN    A2        // Nixie Power Supply Module: "0" - ON, "1" - OFF                  
#define DIN   12        // HV5812 serial data input             
#define CLK   A1        // HV5812 data shift register clock input      
#define STR   A0        // HV5812 output enable input
#define DOT   4         // Nixie tube dot symbol 

// t variable for Arduino Time Library 
time_t t;
tmElements_t tm;
// See the Arduino Time Library for details on the tmElements_t structure: 
// http://playground.arduino.cc/Code/Time
// https://github.com/PaulStoffregen/Time 

// Bit array for 4 nixie tubes, 10 bits for each tube
boolean nixieDisplayArray[40];

// Serial monitor state
boolean serialState = 0;

// Current nixie tube dot state
boolean dotState = 0;

// Assignment of the connected cathodes to the position in the 40 bit array
// Each cathode of nixie tubes is connected to the corresponding output of the HV5812 driver
int nixie1[]={26, 27, 28, 29, 20, 21, 22, 23, 24, 25};    
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9
                     
int nixie2[]={33, 32, 31, 30, 39, 38, 37, 36, 35, 34};
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9

int nixie3[]={ 8,  7,  6,  5,  4,  3,  2,  1,  0,  9};
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9

int nixie4[]={11, 12, 13, 14, 15, 16, 17, 18, 19, 10};
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9

int digit1 = 0;
int digit2 = 0;
int digit3 = 0;
int digit4 = 0;

// Millis delay time variable 
unsigned long previous_millis = 0;


void setup() 
{  
    Serial.begin(9600);

    // Initialize connected pins as an output
    pinMode(EN, OUTPUT);              
    digitalWrite(EN, HIGH);     // Turn off NPS - Nixie Power Supply Module
  
    pinMode(CLK, OUTPUT);       // Nixie driver data shift register clock input 
    digitalWrite(CLK, LOW);
  
    pinMode(DIN, OUTPUT);       // Nixie driver serial data input 
    digitalWrite(DIN, LOW);             
  
    pinMode(STR, OUTPUT);       // Nixie driver output enable input
    digitalWrite(STR, LOW);
  
    pinMode(DOT, OUTPUT);       // Nixie tube dot
    digitalWrite(DOT, HIGH);    // Turn on nixie dot

    Serial.println("#############################################################");
    Serial.println("----- Nixie Clock Thermometer Hygrometer Arduino Shield -----");
    Serial.println("---------------- If you want to set new Time ----------------");
    Serial.println("--------------- press ENTER within 5 seconds ----------------");

    // Millis time start
    unsigned long millis_time_now = millis();
    unsigned long millis_time_now_2 = millis();
    
    // Wait 5 seconds
    while((millis() < millis_time_now + 5000))
    {    
        // Print progress bar      
        if (millis() - millis_time_now_2 > 80)
        {
            Serial.print("#");
            millis_time_now_2 = millis();    
        }

        // Set serialState flag if time settings have been selected 
        if(Serial.available() > 0) 
        {            
            serialState = 1;
            break;  
        }
    }

    Serial.println('\n');
    
    // Clear serial buffer
    while(Serial.available())
    Serial.read();

    // Turn on NPS - Nixie Power Supply Module
    digitalWrite(EN, LOW);  
}

void loop() 
{    
    // Send time to the RTC if the time settings have been selected
    if(serialState == 1)
    {
        SetNewTime();
        serialState = 0;             
    }

    // Millis time start
    unsigned long current_millis = millis();

    // Wait 1 second
    if(current_millis - previous_millis >= 1000)
    {
        previous_millis = current_millis;      
        
        // Blink nixie tube dot
        BlinkDot();

        // Get time from RTC and display on nixie tubes
        DisplayTime();
    }
}

void SetNewTime()
{  
    Serial.println("------ Enter the TIME without spaces in the HHMM format ------");
    Serial.println("- and press enter when you are ready to send data to the RTC -");
    Serial.println('\n');

    // Clear serial buffer
    while(Serial.available())
    Serial.read();
    
    // Wait for the values
    while (!Serial.available()) {}                      

    // Read time as an integer value
    int hhmm_time = Serial.parseInt();

    // Extract minutes and hours
    tm.Minute = (hhmm_time / 1) % 100;
    tm.Hour   = (hhmm_time / 100) % 100;
            
    // Set the time stored in tmElements_t tm structure
    RTC.write(tm);  
}

void DisplayTime()
{
    // Read time from RTC  
    t = RTC.get();
    int timeHour = hour(t);
    int timeMinute = minute(t);
    
    int timeFormat = HH;
    // Check time format and if it is 12 hour then set proper time
    if(timeFormat == 12 && timeHour > 12) timeHour = timeHour - 12;
    if(timeFormat == 12 && timeHour == 0) timeHour = 12;

   
    // Print time on serial monitor
    PrintTime();

    // Extract individual digits
    digit1  = (timeMinute / 1)  % 10;
    digit2  = (timeMinute / 10) % 10;
    digit3  = (timeHour / 1)  % 10;
    digit4  = (timeHour / 10) % 10;

    // Activate "Slot Machine Effect" every 60 seconds
    if(second(t) == 0)
    {
        SlotMachine();      
    }

    NixieDisplay(digit1, digit2, digit3, digit4);
}

void SlotMachine()
{
    for(int i = 0; i < 45; i++)
    {   
        if(i < 30) digit1 = digit1 + 1;        
        if(digit1 >= 10) digit1 = 0;
        
        if(i >= 5 && i < 35) digit2 = digit2 + 1;  
        if(digit2 >= 10) digit2 = 0;  

        if(i >= 10 && i < 40) digit3 = digit3 + 1;  
        if(digit3 >= 10) digit3 = 0; 

        if(i >= 15) digit4 = digit4 + 1;  
        if(digit4 >= 10) digit4 = 0; 
        
        NixieDisplay(digit1, digit2, digit3, digit4);
        delay(100);       
    }
}

void NixieDisplay(int digit1, int digit2, int digit3, int digit4)
{
    // Translate values to cathodes numbers connected to outputs of the HV5812 
    int tube1 = nixie1[digit1];
    int tube2 = nixie2[digit2];
    int tube3 = nixie3[digit3];
    int tube4 = nixie4[digit4];

    // Clear bit array
    for (int i = 39; i >= 0; i--)
    {
        nixieDisplayArray[i] = 1;      
    }

    // Set bits corresponding to the nixie tubes cathodes
    nixieDisplayArray[tube1] = 0;
    nixieDisplayArray[tube2] = 0;
    nixieDisplayArray[tube3] = 0;
    nixieDisplayArray[tube4] = 0; 

    // Send bit array to the nixie drivers 
    for (int i = 39; i >= 0; i--)
    {
        digitalWrite(DIN, nixieDisplayArray[i]);
        digitalWrite(CLK, HIGH);
        delay (1);
        digitalWrite(CLK, LOW);
        delay (1);      
    }

    // Turn on the outputs
    digitalWrite(STR, HIGH);
    delay (1);
    digitalWrite(STR, LOW);       
}

void PrintTime()
{
    Serial.print("Current Time: ");
    if(hour(t) < 10)   Serial.print("0");
    Serial.print(hour(t));
    Serial.print(":");
    if(minute(t) < 10) Serial.print("0");
    Serial.print(minute(t));  
    Serial.print(":");
    if(second(t) < 10) Serial.print("0");
    Serial.println(second(t));      
}

void BlinkDot()
{
    dotState = !dotState;
    digitalWrite(DOT, dotState);    
}
