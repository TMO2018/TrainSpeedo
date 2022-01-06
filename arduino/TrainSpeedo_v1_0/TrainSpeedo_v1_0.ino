int Version = 2;

/*
  Trainspeed calculator.
  
  This uses 2 inputs from IR LED / IR receivers and measures the speed of a train over a certain track length
  
  Note on the inputs :   The IR receivers will switch the corresponding inputs to GND when they detect IR light
                           so these must be triggered on  "HIGH"  value (meaning a train is blocking the path between
                           IR LED (sender) and IR  transistor (receiver).   However, the push buttons will switch on
                           negative  (connectin to ground) because of having the pullup resistors enabled.  This
                           avoids having to solder in  pull-down resistors on all these channels.
   
--COMMUNICATION
  D0 =  serial comm
  D1 =  serial comm

--INPUTS.
  D2 =  IR sensor 1  (int 0)     -- PULLUP !!
  D3 =  IR sensor 2  (int 1)     -- PULLUP !!
  D4 =  Start measuring (armed)  -- PULLUP !!
  D5 =  Reset to zero            -- PULLUP !!
  D6 =  Settings menu            -- PULLUP !!
  A0 =  Setting UP               -- PULLUP !!
  A1 =  Setting DOWN             -- PULLUP !!
  
--OUTPUTS 
  D7 =  StandBy / Ready LED (blue)
  D8 =  Busy LED  (blinking)
  D9 =  IR LED 1
  D10 = IR LED 2
  D11 = LCD RS
  D12 = LCD ENable
  D13=  Power LED  (red)
  A2  = LCD D4
  A3  = LCD D5
  A4  = LCD D6
  A5  = LCD D7  
  
 */

/* speed conversion formulas  

-- a train in scale 1:X  takes  Y seconds  to run   Z   cm of track
=> his scale speed is    Z / Y       cm/sec
=> he runs               Z/(100*y)   m/sec
1 m/sec corresponds to 3600 m per hour 
this equals 3,6 km per hour
==>  his scale speed is then     Z *  3,6  /  (100 * y)   km/h
1 cm on scale 1:X   would be     X cm in the real world
==>   The train's converted  "real" speed is  then expressed by the formula  
         X * Z * 3,6 / (100 * Y)    km/h 

An example  =    train in scale 1:87  does   3 meters (300 cm)  in  20 seconds
==>  his speed is   (87 * 300 *  3,6 / (100 * 20 )  =  93960 / 2000 = 47  km/h 

On the Arduino, we measure the time difference in milliseconds.   (time / 1000)
Conclusion: the formula =  
      SpeedKmh =  3.6 * Scale * Length / (100 * Delta/1000 )

The example  :  
     SpeedKmh = 3.6 * 87 * 300 / (100 * 20000 / 1000 ) = 93960 / 2000 = 47 km/h
*/

//  Libraries and  declarations
#include <LiquidCrystal.h>
#include <EEPROM.h>


// GLOBAL PROGRAM VARIABLES
int Length = 4;              // default 1 meter = 4x10 cm  -- will read from eeprom
int Scale  =  160;             // default N scale  1:160      -- will read from eeprom
int SpeedKmh = 0;             // calculated speed in km per hour
int Conversion = 0;           // conversion factor based on EEprom values
int Trigger_RL = 0;           // 0=no motion   1=Left triggered     2=right triggered
int MenuStep = 0;             // Menu step 0=none  1= scale  2 = Speed
unsigned long Start  = 0;     // milliseconds at start
unsigned long Finish = 0;     // milliseconds at end
unsigned long Delta= 0;       // timed milliseconds
unsigned long RealKmh = 0;
boolean Running = false;      // measurement ongoing
boolean Armed   = false;      // not ready
boolean Sensor1On = false;    // enable or disable IR sensor 1 (left)
boolean Sensor2On = false;    // enable or disable IR sensor 2 (right)
boolean MenuMode = false;      // enable or disable menu mode

// TMO
int Taste_DOWN = A1;
int Taste_UP   = A0;

// initialize the library with the numbers of the interface pins
LiquidCrystal LCD(12, 11, A2, A3, A4, A5);
           //     RS  EN  D4  D5  D6  D7 


void setup() {
  pinMode(13,OUTPUT);

  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);
  pinMode(A5,OUTPUT);
  
  digitalWrite(13,HIGH);
  Serial.begin(9600);
  Serial.println("Initializing");
  Serial.print("Version  3.");Serial.println(Version);
  // set up the LCD's number of columns and rows: 
  Serial.println("Setup LCD");
  digitalWrite(13,LOW);
  LCD.begin(16, 2);
  LCD.display();
  LCD.clear();
 digitalWrite(13,HIGH);
 // Print a message to the LCD.
  LCD.clear();LCD.print("Train Speedo");
  delay(1000);
  // read scale and track length from EEPROM
  Serial.println("Retrieve EEPROM data");
  Length = EEPROM.read(1);  // max 255 = 2550 cm = 25,5 meters
  Serial.print("Measuring track length in dm = ");Serial.println(Length);
  Scale  = EEPROM.read(2);
  Serial.print("Measuring scale = 1:");Serial.println(Scale);
  Serial.println("Establishing Calculating conversion factor");
  Serial.println("dm/ms -> km/h  =   3,6 x distance(dm) x scale(S) / Time(mSec) ");
  RealKmh=3.6 *Scale * 20 / 200;
  SpeedKmh=int(RealKmh);
  Serial.print("36 x "); Serial.print(Scale); Serial.println(" x 20 / 2000");
  Serial.println(RealKmh);
  Serial.println(SpeedKmh);
  //   
  LCD.clear();LCD.print("Initializing");delay(500);
  Serial.println("Setting Output channels");
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);

  Serial.println("Setting Output channels and enabling internal Pull Up resistors");
  pinMode(2 ,INPUT_PULLUP);
  pinMode(3 ,INPUT_PULLUP);
  pinMode(4 ,INPUT_PULLUP);
  pinMode(5 ,INPUT_PULLUP);
  pinMode(6 ,INPUT_PULLUP);
  pinMode(A0,INPUT_PULLUP);
  pinMode(A1,INPUT_PULLUP);
  Serial.println("Attaching interrupts");
//  attachInterrupt(0, LeftDetect, RISING);
//  attachInterrupt(1, RightDetect, RISING);
  Serial.println("Setting variables");
  Armed=false;
  Running=false;
  Sensor1On=false;
  Sensor2On=false;
  MenuMode=false;
  LCD.clear();LCD.print("Starting");delay(500);
  digitalWrite( 7,LOW);
  digitalWrite( 8,LOW);
  digitalWrite( 9,LOW);
  digitalWrite(10,LOW);
  digitalWrite(13,HIGH);
  LCD.clear();
  LCD.setCursor(0,0);LCD.write("System Ready");
  LCD.setCursor(0,1);LCD.write("Press ** GO ** ");
  Serial.println("Initialisation OK");
  Serial.println("Press  START  button to initiate measurement ");
  
}

////////////////////////////////////////////////////////////////////////////////////////////
//  MAIN ROUTINE
////////////////////////////////////////////////////////////////////////////////////////////


// the loop routine runs over and over again forever:
void loop() 

{

//  if system is armed, check for passage at either IR detectors
if (Armed) // run this only when a measurement is requested 
  {digitalWrite(7,HIGH);digitalWrite(8,LOW);
   if (digitalRead( 2))   // Sensor 1
    {LeftStart();goto EndLoop;}
   if (digitalRead( 3))    // Sensor 2 
    {RightStart();goto EndLoop;}   }  

//  if system is measuring, check for passage at either IR detectors
if (Running) // run this only when a measurement is ongoing
  {digitalWrite(7,LOW);digitalWrite(8,HIGH);
   if (digitalRead( 2) && Sensor1On) {LeftStop();goto EndLoop;}    // Sensor 1
   if (digitalRead( 3) && Sensor2On) {RightStop();goto EndLoop;} } // Sensor 2 
 
// reset and request  buttons   
if (!digitalRead( 5)) {ResetSystem();goto EndLoop;}                 // Reset
if (!digitalRead( 4) && !Armed ) {ArmSystem(); goto EndLoop;}       // Activate
  
//  menu for setting distance and scale  
if (!digitalRead( 6))    // MENU Select
  {Serial.println("D6  to ground");
   MenuMode=true;
   ProcessMenu();
   goto EndLoop;}
   
if (!digitalRead(A0)) {Serial.println("A0  to ground");}   // MENU +
if (!digitalRead(A1)) {Serial.println("A1  to ground");}   // MENU -



EndLoop:   // resume from start
DoNothing();

}

/////////////////////////////////////////////////////////////////////////////////
//  Set all variables and arm the system
/////////////////////////////////////////////////////////////////////////////////
void ArmSystem()
{ Serial.println("D4  to ground");
  Serial.println("System armed");
  Trigger_RL  = 0;
  Armed       = true;  
  Running     = false;
  Serial.println("Setting IR LEDs");
  digitalWrite(9,HIGH);
  digitalWrite(10,HIGH);  
  Serial.println("Setting Status LEDs");
  digitalWrite(13,HIGH);  // power
  digitalWrite( 7,HIGH);  // armed
  digitalWrite( 8,LOW );  // busy}
  Serial.println("Setting sensors");
  Sensor1On = true;
  Sensor2On = true;
  Start  = 0;
  Finish = 0;
  Delta  = 0;
  LCD.clear();
  LCD.setCursor(0,0);LCD.write("*** STARTED ***");
  LCD.setCursor(0,1);LCD.write("Wait for Signal");
 }
 
/////////////////////////////////////////////////////////////////////////////////
//  Reset system
/////////////////////////////////////////////////////////////////////////////////
void ResetSystem()
{ Serial.println("System Reset");
  digitalWrite(13,HIGH);  // power
  digitalWrite( 7,HIGH );  // armed
  digitalWrite( 8,HIGH );  // busy
  LCD.clear();
  LCD.setCursor(0,0);LCD.write("** R E S E T **");
  LCD.setCursor(0,1);LCD.write("Please hold....");
  Trigger_RL  = 0;
  Armed       = false;  
  Running     = false;
  Serial.println("Resetting IR LEDs");
  digitalWrite(9,LOW);
  digitalWrite(10,LOW);
  Serial.println("Setting Status LEDs");
  Serial.println("Setting sensors");
  Sensor1On = false;
  Sensor2On = false;
  Start  = 0;
  Finish = 0;
  Delta  = 0;
  delay(2000);
  LCD.clear();
  LCD.setCursor(0,0);LCD.write("System Ready");
  LCD.setCursor(0,1);LCD.write("Press ** GO ** ");
  digitalWrite(13,HIGH);  // power
  digitalWrite( 7,LOW );  // armed
  digitalWrite( 8,LOW );  // busy
  // soft reset
  asm volatile ("  jmp 0");
  
}


/////////////////////////////////////////////////////////////////////////////////
//  LeftStart   interrupt routine   on pin  D2  - IRQ 0
/////////////////////////////////////////////////////////////////////////////////
void LeftStart()
{ // left sensor linked to  IR  LED 1
  Serial.println("D2  to ground");
  Serial.println("LeftStart triggered");
  Trigger_RL=1;
  Serial.println("Disabling left sensor 1");
  Sensor1On = false;  
  Sensor2On = true;  
  Armed=false;
  Running=true;
  digitalWrite(7,LOW);  
  digitalWrite(8,HIGH);
  
  digitalWrite(9,LOW);  
  digitalWrite(10,HIGH);
  
  Start  = millis();
  Finish = 0;
  Delta  = 0;
  LCD.clear();
  LCD.setCursor(0,0);LCD.write("STARTED LEFT");
  LCD.setCursor(0,1);LCD.write("Measuring....");

}


/////////////////////////////////////////////////////////////////////////////////
//  RightStart   interrupt routine   on pin  D3  - IRQ 1
/////////////////////////////////////////////////////////////////////////////////
void RightStart()
{ // right sensor linked to  IR  LED 1
  Serial.println("D3  to ground");
  Serial.println("RightStart triggered");
  Trigger_RL=2;
  Serial.println("Disabling right sensor 2");
  Sensor1On = true;    
  Sensor2On = false;  
  Armed=false;
  Running=true;
  digitalWrite(7,LOW);  
  digitalWrite(8,HIGH);
  digitalWrite(10,LOW);  
  digitalWrite(9,HIGH);
  
  Start  = millis();
  Finish = 0;
  Delta  = 0;
  LCD.clear();
  LCD.setCursor(0,0);LCD.write("STARTED RIGHT");
  LCD.setCursor(0,1);LCD.write("Measuring....");
}


/////////////////////////////////////////////////////////////////////////////////
//  RightStop   interrupt routine   on pin  D3  - IRQ 1
/////////////////////////////////////////////////////////////////////////////////
void RightStop()
{ // right sensor linked to  IR  LED 2
  LCD.setCursor(0,1);LCD.write("STOPPED");
  Serial.println("D3  to ground");
  Serial.println("RightStop triggered");
  Finish = millis();
  CalculateSpeed();
}

/////////////////////////////////////////////////////////////////////////////////
//  LeftStop   interrupt routine   on pin  D2  - IRQ 0
/////////////////////////////////////////////////////////////////////////////////
void LeftStop()
{ // right sensor linked to  IR  LED 1
  LCD.setCursor(0,1);LCD.write("STOPPED");
  Serial.println("D2  to ground");
  Serial.println("LeftStop triggered");
  Finish = millis();
  CalculateSpeed();
}


/////////////////////////////////////////////////////////////////////////////////
//  CalculateSpeed()
/////////////////////////////////////////////////////////////////////////////////
void CalculateSpeed()
{
  unsigned long  snel=0;
  
  digitalWrite(13,LOW);  
  Serial.println("Disabling both sensors");
  Sensor1On = false;    
  Sensor2On = false;  
  Serial.println("Setting system variables");
  Trigger_RL=0;
  Armed=false;
  Running=false;
  digitalWrite( 7,LOW);
  digitalWrite( 8,LOW);  
  digitalWrite( 9,LOW);  
  digitalWrite(10,LOW);  
  Serial.println("Calculating");
  Delta    = Finish-Start;      //elaopsed times in milliseconds.
  RealKmh  = 360 * Length;       Serial.print("Step 1 - 3,6 * Length   : ");Serial.println(RealKmh);
  RealKmh  = RealKmh * Scale;    Serial.print("Step 2 - speed * Scale  : ");Serial.println(RealKmh);
  RealKmh  = RealKmh / Delta;    Serial.print("Step 3 - speed /Delta   : ");Serial.println(RealKmh);
  SpeedKmh = int(RealKmh);
  snel= 360 * Length * Scale / Delta ;  Serial.print ("Alternate : ");Serial.println(int(snel));
  
  Serial.println("Displaying");
  LCD.clear();
  LCD.setCursor(0,0);LCD.write("Time:");
  LCD.setCursor(7,0);LCD.print(Delta);
  LCD.setCursor(0,1);LCD.write("km/h=");
  LCD.setCursor(7,1);LCD.print(SpeedKmh);
  Serial.print("Length = ");Serial.print(Length);Serial.println(" dm");
  Serial.print("Time   = ");Serial.print(Delta);Serial.println(" mSec");
  Serial.print("Scale  = ");Serial.print(Scale);Serial.println(" factor");
  Serial.print("Km/h   = ");Serial.print(SpeedKmh);Serial.println(" km/h");
  digitalWrite(13,HIGH);  
}

/////////////////////////////////////////////////////////////////////////////////
//  ProcessMenu
/////////////////////////////////////////////////////////////////////////////////
void ProcessMenu()
{ 
//  at first  push, enter menu mode 1  (set scale)
//  at second push, enter menu mode 2  (set length)
//  at third  push, enter menu mode 3  (save or cancel)
//  at fourth push, enter menu mode 0  (stop)

int tmpScale =  Scale;
int tmpLength = Length;
boolean Pressed = false;
boolean Accept = false; 

MenuMode=true;
MenuStep=1;

        Serial.print("Select Scale :");Serial.println(tmpScale);
        LCD.clear();
        LCD.setCursor(0,0);LCD.write("Scale : ");
        LCD.setCursor(10,0);LCD.print(tmpScale);
        delay(500);
        // check for buttons and act 
        Pressed=false;
        do {if ( !digitalRead(6))  {Pressed=true;delay(500);}
            if ( !digitalRead(Taste_DOWN)) 
                     {delay(200);tmpScale=tmpScale+1;
                      if(tmpScale>240) {tmpScale=240;}
                      Serial.println(tmpScale);
                      LCD.setCursor(10,0);LCD.write("     ");LCD.setCursor(10,0);LCD.print(tmpScale);}
            if ( !digitalRead(Taste_UP)) 
                     {delay(200);tmpScale=tmpScale-1;
                      if(tmpScale<1)   {tmpScale=1;}
                      Serial.println(tmpScale);
                      LCD.setCursor(10,0);LCD.write("     ");LCD.setCursor(10,0);LCD.print(tmpScale);}
           } while (!Pressed);


        Serial.print("Select Length :");Serial.println(tmpLength);
        LCD.setCursor(0,1);LCD.write("Length : ");
        LCD.setCursor(10,1);LCD.print(tmpLength);
        // check for buttons and act 
        delay(500);
        Pressed=false;
        do {if ( !digitalRead(6))  {Pressed=true;delay(500);}
            if ( !digitalRead(Taste_DOWN)) 
             {delay(200);tmpLength=tmpLength+1;
              if(tmpLength>200) {tmpLength=200;}
              Serial.println(tmpLength);
              LCD.setCursor(10,1);LCD.write("     ");LCD.setCursor(10,1);LCD.print(tmpLength);}
            if ( !digitalRead(Taste_UP)) 
              {delay(200);tmpLength=tmpLength-1;
               if(tmpLength<1)   {tmpLength=1;}
               Serial.println(tmpLength);
               LCD.setCursor(10,1);LCD.write("     ");LCD.setCursor(10,1);LCD.print(tmpLength);}
           } while (!Pressed);
           delay(500);

        Serial.print("Confirm or Cancel :");Serial.println("Y/N");
        Accept=true;
        LCD.clear();
        LCD.setCursor(1,0);LCD.write("Action : ");
        LCD.setCursor(1,10);LCD.write("ACCEPT");
        // check for buttons and act 
        delay(500);
        Pressed=false;
        Serial.println("Accept");
        do {if ( !digitalRead(6))  {Pressed=true;delay(500);}
            if ( !digitalRead(Taste_DOWN)) {Accept=!Accept;delay(200);
                   if (Accept)
                       {LCD.setCursor(1,10);LCD.write("ACCEPT");Serial.println("Accept");}
                   else
                       {LCD.setCursor(1,10);LCD.write("REJECT");Serial.println("Reject");}}
            if ( !digitalRead(Taste_UP)) {Accept=!Accept;delay(200);
                   if (Accept)
                       {LCD.setCursor(1,10);LCD.write("ACCEPT");Serial.println("Accept");}
                   else
                       {LCD.setCursor(1,10);LCD.write("REJECT");Serial.println("Reject");}}
           } while (!Pressed);
        if (Accept) {// save to EEPROM and update memory.
          Serial.println("Writing values to memory and udpating ");
          Length=tmpLength;EEPROM.write(1,int(Length));
          Scale=tmpScale;EEPROM.write(2,Scale);}
          LCD.clear();
          LCD.setCursor(0,0);LCD.write("System Ready");
          LCD.setCursor(0,1);LCD.write("Press ** GO **");
          Serial.println("Done saving values.  ready for next action");
     
}


/////////////////////////////////////////////////////////////////////////////////
//  DoNothing
/////////////////////////////////////////////////////////////////////////////////
void DoNothing()
{ /// absolutely NUTTIN'
}
