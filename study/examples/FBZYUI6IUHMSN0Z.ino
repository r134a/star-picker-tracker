/* This sketch drives an 28BYJ-48 stepper motor via its ULN2003 driver
 *  initial start speed is 271.6 to drive a screw for tracking stars during astro photography.
 The value of adckeyin has been altered to reflect the values delivered by the HobyTronics unit
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   
*/




#include <AccelStepper.h>
#include <LiquidCrystal.h>
#define HALFSTEP 8
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int adc_key_val[5] ={50, 195, 380, 555, 790 };

/*Motor pin definitions when using HobbyTronics Keypad shield
 have been changed to 15 16 17 18. 
*/
#define motorPin1  15    // IN1 on the ULN2003 driver 1
#define motorPin2  16    // IN2 on the ULN2003 driver 1
#define motorPin3  17    // IN3 on the ULN2003 driver 1
#define motorPin4  18    // IN4 on the ULN2003 driver 1

/* 
original code using the authors keypad, the pinouts have to 
be changed when using the hobby tronics keypad, this is the original
outputs allocated
#define motorPin1  3     // IN1 on the ULN2003 driver 1
#define motorPin2  4     // IN2 on the ULN2003 driver 1
#define motorPin3  5     // IN3 on the ULN2003 driver 1
#define motorPin4  6     // IN4 on the ULN2003 driver 1
*/

int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int isRun;
double speeds = 271.6;
int maxspeed = 1245;

AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

void setup() {                    // void means dont expect any return from this command
  lcd.clear();                    // clear the lcd
  lcd.begin(16, 2);               // set up the LCD's number of columns and rows:
  lcd.setCursor(0,0); 
  lcd.print("    Stopped     ");  // display stopped on the LCD
  lcd.setCursor(0,1); 
  lcd.print("Speed ");            // display the value of speed
  lcd.print(speeds);              // display the value of speeds
  lcd.print("   ");
  isRun = 0;

  stepper1.setMaxSpeed(maxspeed); //1245
  stepper1.setSpeed(speeds);      //271.6
}

void loop() {
  adc_key_in = analogRead(0);    // read the value from the sensor 
  key = get_key(adc_key_in);     // convert into key press
 
  if (key >= 0)                  // if keypress is detected
   {
      if (key == 1) {
         speeds += 0.1;
         delay(50);
      }
      if (key == 2 && speeds > 0) {
         speeds -= 0.1;
         delay(50);
      }
      if (key == 0) {
         speeds += 10;
      }
      if (key == 3) {
         speeds -= 10;
      }
      if (speeds>maxspeed) {
         speeds = maxspeed;
      }
      if (speeds<-maxspeed) {
         speeds = -maxspeed;
      }
      if (key == 4) {
        isRun = 1 - isRun;
        lcd.setCursor(0,0); 
        if (isRun == 1) {
          lcd.print("+++ Running +++ ");
        } else {
          lcd.print("    Stopped     "); 
        }
        delay(250);
      }
      lcd.setCursor(0, 1);
      lcd.print("Speed "); 
      lcd.print(speeds);
      lcd.print("       ");
      stepper1.setSpeed(speeds);
      delay(50);
  }
  if (isRun == 1) {
    stepper1.runSpeed();
  }
}

int get_key(unsigned int input)
{
    int k;
    for (k = 0; k < NUM_KEYS; k++)
    {
      if (input < adc_key_val[k])
            return k;
    }
   
    if (k >= NUM_KEYS)
      k = -1;  // No valid key pressed
    return k;
}
