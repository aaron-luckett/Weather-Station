

#include <stdint.h>
#include "SparkFunBME280.h"

#include "Wire.h"
#include "SPI.h"
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <SD.h>



//Global sensor object
BME280 mySensor;


unsigned long previousTime = 0;
unsigned long currentTime = 0;
const int interval = 1000; // 1000 ms between actions
int i = 0;

//Creates the variables used in the program
float Temperature;
float displayTemp;
float maxTemp;
float minTemp;
String displayString;
float displayMinTemp;
float displayMaxTemp;
float Pressure;
float Humidity;
String Hour;
String Minute;
String Second;
int counter = 1;
int counterTwo = 1;
String unit;


File dataFile;

char fileName[16];


const int RIGHT_KEY = 1; // define some symbolic names for the keys
const int UP_KEY = 2;
const int DOWN_KEY = 3;
const int LEFT_KEY = 4;
const int SELECT_KEY = 5;


int backlight = 127;
bool flag = true;

const int chipSelect = 10;

RTC_DS1307 rtc;

LiquidCrystal lcd(8,9,4,5,6,7);


void setup()
{
Serial.begin(9600);

while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
    
    
    
    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // November 12, 2019 at 3pm you would call:
    // rtc.adjust(DateTime(2019, 11, 12, 15, 24, 0));
  }

  FileBuilder();



  //Initializes the BME280 sensor to run using an I2C protocol
  mySensor.settings.commInterface = I2C_MODE;
  mySensor.settings.I2CAddress = 0x77;
  

  //***Operation settings*****************************//
  mySensor.settings.runMode = 3; //  3, Normal mode
  mySensor.settings.tStandby = 0; //  0, 0.5ms
  mySensor.settings.filter = 0; //  0, filter off
  //tempOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.tempOverSample = 1;
  //pressOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    mySensor.settings.pressOverSample = 1;
  //humidOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.humidOverSample = 1;

  // Initialzes the LCD screen & tells the Arduino the dimensions of the screen
  lcd.begin(16, 2);  

  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

  // Gets the sensor to start taking readings
  mySensor.begin();


  Serial.print("Welcome to the hyper advanced weather monitor\n\n");
  Serial.print("Temperature: \tPressure: \tHumidity:\n\n");

 
  SensorDataReader();
  minTemp = Temperature;
  maxTemp = Temperature;
  // variable used to help keep track of time so readings can be made every second
  previousTime = millis();
}

void loop()
{

currentTime = millis(); // record current time
  
 // if statement to check if 1 second interval has elapsed
  keyPad();
  if (currentTime - previousTime >= interval)
  {
    previousTime = currentTime; // update previous time
  //Calls the functions which reads data from the BME280 sensor, formats it, and writes the formatted data to the LCD
  SensorDataReader();
  TimeReader();

  //will check to see if a new minimum or maximum temperature has been recorded
  minTempChecker();
  maxTempChecker();

  //calls the temperature conversion function
  tempConverter();

  //calls the function to write to the LCD monitor 
  LCDWriter();
   i++;
   if (i == 10)
   {
    i = 0;
    //prints the recorded values to the serial monitor
    Serial.print(String(Temperature) + "°C\t\t");         
    Serial.print(String(Pressure) + "mBar\t");
    Serial.print(String(Humidity) + "%\n");
    //calls the fileWriter function
    FileWriter();
   }
  }

  //sets the  LED backlight to the backlight value
  analogWrite(3, backlight);
}

// Function which formats data from the Arduino modules, and writes the formatted data to the LCD
void LCDWriter()
{  
  // sets the LCD to print the next set of data from the top left corner of the screen
  lcd.setCursor(0, 0);
  // prints a formatted Temperature string to 1 decimal place & concatenates '°C' to the end
  lcd.print((String(displayTemp, 1)));
  if(counter != 3){
    //char 223 is used to print the degrees symbol to the LCD monitor if units are not kelvin
    lcd.print(char(223));
  }
  //will print the specified unit after the temperature is displayed
  lcd.print(unit);

  // sets the LCD to print the next set of data from the middle of the top part of the screen
  lcd.setCursor(8, 0);
  // prints a formatted Pressure string to 0 decimal places and concatenates the units 'mBar' to the end
  lcd.print(String(Pressure, 0) + "mBar");
  
  
  // sets the LCD to print the next set of data from the bottom left part of the screen
  lcd.setCursor(0, 1);
  if(counterTwo == 1){
    // prints a formatted Humidity string to 0 decimal places and concatenates a % symbol to the end
    lcd.print(String(Humidity, 0) + "%       ");
  
    lcd.setCursor(8, 1);
    //prints the time to the LCD monitor
    lcd.print(Hour + ":" + Minute + ":" + Second); 
   } else {
    //prints the maximum and minimum temperatures
    lcd.print((String(displayMinTemp, 1)) + "/" + (String(displayMaxTemp, 1)) + "         ");
   }
}

//Reads the Temperature, Pressure & Humidity fron the BME280 Sensor
void SensorDataReader()
{     
  Temperature = mySensor.readTempC();
  Pressure = (mySensor.readFloatPressure())/100; // converts the reading in Pascals to Millibar
  Humidity = mySensor.readFloatHumidity();
}

//will add the correct amount of zeros for the time recorded
String TimeFormatter(String arg)
{
  if (arg.length() == 1)
  {
    arg = "0" + arg;        //will add a zero beforehand if the number is one digit long
  }
  return arg;
}

//will get the current time and then format it correctly
void TimeReader()
{
  DateTime now = rtc.now();
  Hour = String(now.hour());
  Hour = TimeFormatter(Hour);

  Minute = String(now.minute());
  Minute = TimeFormatter(Minute);
  
  Second = String(now.second());
  Second = TimeFormatter(Second);  
}

//will write the date,time, temperature, pressure and humidity to the file created
void FileWriter()
{
    DateTime now = rtc.now();
    dataFile = SD.open(fileName, FILE_WRITE);
    if (dataFile) {
    dataFile.print(now.day(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.year(), DEC);
    dataFile.print(',');
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    dataFile.print(now.second(), DEC);
    dataFile.print(',');
    dataFile.print(Temperature,2);
    dataFile.print(',');
    dataFile.print(Pressure,2);
    dataFile.print(',');
    dataFile.print(Humidity,0);
    dataFile.print("\n");
    dataFile.close();
    }
}

//creates sequential file that the data will be written to 
void FileBuilder()
{

int count = 0;
 do
  {      
    sprintf(fileName, "DATA%04d.TXT", count);
    count++;
  }
  while(SD.exists(fileName));               //if file exists, add one to counter and try again
  }

//function to apply button functionality 
void keyPad()
  {
    int A0Input = analogRead(A0); // temporary storage for analog input
    if (flag == true)
    {
      //Determine which button was pressed and applies their functionality 
      switch (A0Input)                                    
      {
      case 0 ... 100 :
        // RIGHT key pressed;
        counterTwo++;                          //identifies which display format to use on the bottom line
        if(counterTwo > 2){
          counterTwo = 1;
        }
        flag = false;
        return;
      case 101 ... 160:
        // UP key pressed
        backlight = constrain(backlight + 16,0,255);                 //increases brightness of screen
        flag = false;
        return ;
      case 161 ... 340:
        // DOWN key pressed
        backlight = constrain(backlight - 16,0,255);                 //decreases brightness of screen
        flag = false;
        return ;
      case 341 ... 510:
      // LEFT key pressed
        counter++;
        if(counter > 3)                         //identifies which unit of temperature to display in
        {
          counter = 1;
        }
        flag = false;
        return ;
      case 511 ... 750:
     // SELECT key pressed
     return ;
     default:
     return;
     
    }

    
  }
  //if no button is pressed flag is set to true so that buttons are not repeatedly triggered with one push
  if (A0Input > 800)
  {
    flag = true;
    return 0;
  }
}

//function to convert temperature into kelvin
float kel(float temp)
{
  float kelTemp = (temp + 273.15);
  return kelTemp;
}

//function to convert temperature into fahrenheit
float fah(float temp)
{
  float fahTemp = (temp*(9/5)+32);
  return fahTemp;  
}

//Does temperature conversion for selected temperature scale
void tempConverter(){
  switch(counter){
    case 1:                                       //for celcius
    displayTemp = Temperature;
    displayString = (String(displayTemp, 1));
    unit = ("C");
    displayMinTemp = minTemp;
    displayMaxTemp = maxTemp;
    break;
    case 2:                                       //for fahrenheit
    displayTemp = fah(Temperature);
    displayString = (String(displayTemp, 1));
    unit = ("F");
    displayMinTemp = fah(minTemp);
    displayMaxTemp = fah(maxTemp);
    break;
    case 3:                                        //for kelvin
    displayTemp = kel(Temperature);
    displayString = (String(displayTemp, 1));
    unit = ("K");
    displayMinTemp = kel(minTemp);
    displayMaxTemp = kel(maxTemp);
    break;
  }
}

// Checks if temperature read is lower than minTemp and sets minTemp to equal temp if temp was lower
void minTempChecker()
 {
  if (minTemp > Temperature)
  {
    minTemp = Temperature;
  }
  return;
 }
 
 // Checks if temperature read is higher than maxTemp and sets maxTemp to equal temp if temp was higher
void maxTempChecker()
 {
  if (maxTemp < Temperature)
  {
    maxTemp = Temperature;
  } 
  return;
 }

  
