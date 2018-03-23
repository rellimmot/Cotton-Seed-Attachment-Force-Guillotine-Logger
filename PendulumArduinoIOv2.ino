#include <Wire.h>             //I2C
#include <LCD.h>              //LCD
#include <LiquidCrystal_I2C.h>//I2C LCD for the PCF8574T chip
//START LCD Constant Initializations
#define I2C_ADDR    0x27      // Define I2C Address for the PCF8574T 
#define BACKLIGHT_PIN  3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
#define LED_OFF  1
#define LED_ON  0
//END LCD Constant Initializations

//START Declare Constants and Variables
const int pendulumPin1 = A0; //Analog Pin A0 to Pendulum pin 1
const int pendulumPin4 = A1; //Analog Pin A1 to Pendulum pin 4
const int triggerPin = 2;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
// the following two long variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;   // the last time the output pin was toggled
long debounceDelay = 50;     // the debounce time; increase if the output flickers
int pendulumMax0 = 0;        //Max value hold for Pendulum pin 1
int pendulumMax1 = 1;        //Max value hold for Pendulum pin 4
int numReadings = 10;        //Number of reading to average out for each reading (reduce noise)
int readingSub[10];
int indexSub = 0;
int readingMain[10];
int indexMain = 0;
int counters = 0;
int readingThreshold = 1023;
int readingResult = 0;
int readingMinMin = 1023;
//END Declare Constants and Variables

//START Declare Objects
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
//END Declare Objects

void setup() {//START SETUP: RUNS ONCE AT START
  pinMode(triggerPin, INPUT); //initialize triggerPin as input
  pinMode(13, OUTPUT); //initialize LED pin 13 as output
  digitalWrite(13, HIGH);//initialize LED as on
  Serial.begin(38400);//Initialize Serial COM @ 38400 baud
  lcd.begin (16,2);  //Initialize the LCD, 16x2: 16 columns, 2 rows
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE); //Turn on the LCD backlight
  lcd.setBacklight(LED_ON);                    //Turn on the LCD backlight
//  lcdIntro();  //Intro screen. See lcdIntro() function to customize.
  lcdLayout();  //Sets layout, runs once so it doesn't have to redraw with every results frame.
  pinMode(triggerPin, INPUT); //initialize triggerPin as input
  pinMode(13, OUTPUT); //initialize LED pin 13 as output
  digitalWrite(13, HIGH);
//  Serial.println("END void setup()");
  digitalWrite(13, LOW);//setup complete, set LED LOW
}//END SETUP
int x = 1;


void loop() {//START LOOP: RUNS CONTINUOUSLY
int valuemain = analogRead(A0);         //takes reading from A0 (reading)
float sensorAlign = analogRead(A1);     //takes reading from A1 (alignment)
    sensorAlign = (sensorAlign/10);     //dull alignment reading for clearer use

//////////////////////TRIGGER///////////////////////////   
int trigger = digitalRead(triggerPin);
if (trigger != lastButtonState) {// START TRIGGER ARGUMENTS
  lastDebounceTime = millis();
}
if ((millis() - lastDebounceTime) > debounceDelay) {    //switch debounce for button/trigger is built in to prevent accidental resends
  if (trigger != buttonState) {                         
    buttonState = trigger;
    if (buttonState == HIGH) {
      if (readingMinMin < 880) {  //if 'Read:' is below 880, allow a result to be sent to serial.
        lcd.noBacklight();
        digitalWrite(13, HIGH);
                            Serial.print(x); x=x+1; //send reading number (x) to serial, increase reading number (x) by 1
        Serial.print(",");  Serial.print("=timestamp()"); //send the custom excel timestamp formula to serial
        Serial.print(",");  Serial.println(readingMinMin);//send the reading to serial
          readingMinMin = 1023; //Resets MINMIN result
          delay(50);
        digitalWrite(13, LOW);
        lcd.backlight();
      }
    }
  }
}//END TRIGGER ARGUMENTS
lastButtonState = trigger;
///////////////////////TRIGGER//////////////////////////


if (valuemain < readingThreshold){

////READING BEGIN: Pendulum Actuated; returns minimum of averaged subvalues
////                                  over the course of reading.

int valuesub = analogRead(A0);

readingSub[indexSub] = valuesub;

indexSub++;
if (indexSub >= numReadings) indexSub = 0;
float total = 0;
int sensorMax = 0;
int sensorMin = 1023;

for (int i = 0; i < numReadings; i++){
  total += readingSub[i];
  if (readingSub[i] > sensorMax) sensorMax = readingSub[i];     //this is all for averaging (for noise reduction)
  if (readingSub[i] < sensorMin) sensorMin = readingSub[i];     //takes 10 readings, averages them together, result is sensorMIN
  if (sensorMin < 1) sensorMin = readingThreshold;
}
int sensorAvg = total / numReadings;

if(readingMinMin > sensorMin) readingMinMin = sensorMin;        //this keeps lowest MIN result (readingMinMin

int sensorMinMin = min(readingMinMin, readingSub[indexSub]);
//SUBREADINGS END: Output = sensorMin, sensorMax, sensorAvg


//  Serial.print("\tMIN:\t");
//  Serial.print(sensorMin);
//  Serial.print("\tMAX:\t");
//  Serial.print(sensorMax);
//  Serial.print("\tAVG:\t");
//  Serial.print(sensorAvg);  // 1 decimal        //for debugging, enable all these for more verbose serial monitor output
//  Serial.print("\tMINMIN:\t");
//  Serial.print(sensorMinMin);
//  Serial.print("\tTRIG\t");
//  Serial.print(trigger);
//  Serial.print("\tCNT:\t");
//  Serial.println(counters);

  
  lcd.setCursor(5,0);
   if (sensorAvg > 99)     lcd.print(sensorAvg);
   else                    lcd.print(sensorAvg); lcd.print(" ");   
  lcd.setCursor(14,0);
                           lcd.print(trigger);                          //all this sends results to LCD
  lcd.setCursor(5,1);
   if (sensorMinMin > 99)  lcd.print(sensorMinMin);
   else                    lcd.print(sensorMinMin); lcd.print(" ");
  lcd.setCursor(14,1); 
   if (sensorAlign > 10)   lcd.print(sensorAlign,0);
   else                    lcd.print(sensorAlign,0); lcd.print(" ");
  counters = (counters + 1);
}

//  delay(500);
}//END LOOP

void lcdIntro(){
  lcd.backlight();
  lcd.clear();
  delay(10);
  lcd.setCursor(3,1); lcd.print("2015 USDA");
  delay(700);
//  lcd.noBacklight();
  lcd.setCursor(3,0); lcd.print("2015 USDA");
  lcd.setCursor(2,1); lcd.print("    ARS   ");
  delay(700);
//  lcd.backlight();
  lcd.clear();
  lcd.setCursor(2,0); lcd.print("    ARS   ");
  delay(700);
  lcd.clear();
//  lcd.noBacklight();
    // scroll 16 positions (display length + string length) to the left
    // to move it back to center:
//  for (int positionCounter = 0; positionCounter < 12; positionCounter++) {
//    // scroll one position left:
//    lcd.scrollDisplayLeft();
//    // wait a bit:
//    delay(250);
//  }
}

void lcdLayout(){
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0); lcd.print("Posn:");
  lcd.setCursor(9,0); lcd.print("Trig:");           //all this sets layout of LCD screen
  lcd.setCursor(0,1); lcd.print("Read:");
  lcd.setCursor(9,1); lcd.print("Algn:");  
}

void lcdResult(){
//  lcd.setCursor(0,5); lcd.print(sensorAvg);
//  lcd.setCursor(0,9); lcd.print(trigger);
//  lcd.setCursor(1,5); lcd.print(sensorMinMin);
//  lcd.setCursor(1,9); lcd.print(sensorAlign);  
}

