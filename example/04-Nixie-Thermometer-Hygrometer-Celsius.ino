// Nixie Clock Thermometer Hygrometer Arduino Shield by Marcin Saj https://nixietester.com
// https://github.com/marcinsaj/Nixie-Clock-Thermometer-Hygrometer-Arduino-Shield
//
// Thermometer (Celsius) and Hygrometer Example
// 
// This example demonstrates how to use DHT22 sensor and display temperature & humidity on nixie tubes. 
// The control is carried out using two HV5812 drivers: http://ww1.microchip.com/downloads/en/DeviceDoc/20005629A.pdf 
// The HV5812 is a 20-channel serial-input display driver. It combines a 20-bit CMOS shift register, 
// data latches and control circuitry with high-voltage MOSFET outputs.
//
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
//
// Hardware:
// 4 Nixie Tubes - counting from the right: IN-15A (+, -, %), IN-12B (digits and dot), 2 x IN-12A or 2 x IN-12B 
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

#include "DHT.h"

#define DHTPIN A3           // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22       // Sensor type

DHT dht(DHTPIN, DHTTYPE);   // Initialize DHT sensor
          
#define EN    A2            // Nixie Power Supply Module: "0" - ON, "1" - OFF                  
#define DIN   12            // HV5812 serial data input             
#define CLK   A1            // HV5812 data shift register clock input      
#define STR   A0            // HV5812 output enable input
#define DOT   4             // Nixie tube dot symbol 


// Bit array for 4 nixie tubes, 10 bits for each tube
boolean nixieDisplayArray[40];

// Assignment of the connected cathodes to the position in the 40 bit array
// Each cathode of nixie tubes is connected to the corresponding output of the HV5812 driver
 
int nixie1[]={26, 27, 28, 29, 20, 21, 22, 23, 24, 25};    
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9
           //  μ,  n,  %,  Π,  k,  M,  m,  +,  -,  P
                     
int nixie2[]={33, 32, 31, 30, 39, 38, 37, 36, 35, 34};
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9

int nixie3[]={ 8,  7,  6,  5,  4,  3,  2,  1,  0,  9};
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9

int nixie4[]={11, 12, 13, 14, 15, 16, 17, 18, 19, 10};
           //  0,  1,  2,  3,  4,  5,  6,  7,  8,  9

// Temperature and humidity symbols (nixie1 array cathode numbers)
#define plusTEMP      7   // +
#define minusTEMP     8   // -
#define HUM           2   // %

int digit1 = 0;
int digit2 = 0;
int digit3 = 0;
int digit4 = 0;

float tempDHT = 0;              // Temperature form sensor  
float humDHT = 0;               // Humidity from sensor  
               
bool flagDisplay = 1;           // "1" - display temperature, "0" - display humidity
int loopCount = 0;

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
  
    delay(1000);

    dht.begin();                // DHT sensor start
}

void loop() 
{

    // Wait a few seconds between measurements.
    delay(2000);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    humDHT = dht.readHumidity();
    // Read temperature as Celsius (the default)
    tempDHT = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humDHT) || isnan(tempDHT)) 
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    // Compute heat index in Celsius
    float hic = dht.computeHeatIndex(tempDHT, humDHT);

    
    // Change display values between temperature and humidity every 10 seconds
    // 2s (default delay between measurements) * loopCount(5) = 10s 
    if (loopCount >= 5)
    {
        loopCount = 0; 
        // Update flag value every 10s
        flagDisplay = !flagDisplay;     
        // Slot machine effect
        SlotMachine();     
    }

    // Change display values (temperature or humidity) every 10 seconds
    if (flagDisplay == 1) DisplayTemperature();
    else DisplayHumidity();

    loopCount++;
}

void DisplayTemperature()
{
    int value = 100 * tempDHT;    

    if(tempDHT >= 0)               
    {                               // If the temperature is > 0°C   
        digit1 = plusTEMP;          // Set "+" symbol            
    }
    else 
    {                               // If the temperature is below 0°C
        value = -value;             // Convert a negative value to a positive for easier conversion for the nixie display  
        digit1 = minusTEMP;         // Set "-" nixie tube symbol
    }
       
    // Extract individual digits
    digit2 = (value / 10) % 10;
    digit3 = (value / 100) % 10;
    digit4 = (value / 1000) % 10;

    // Display on nixie tubes
    NixieDisplay(digit1, digit2, digit3, digit4);
}

void DisplayHumidity()
{
    int value = 100 * humDHT; 
    
    digit1 = HUM;                   // Set "%" symbol for humidity
    
    // Extract individual digits
    digit2 = (value / 10) % 10;
    digit3 = (value / 100) % 10;
    digit4 = (value / 1000) % 10;

    // Display on nixie tubes
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
        delay(30);       
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

    // Turn on NPS - Nixie Power Supply Module
    digitalWrite(EN, LOW);      
}
