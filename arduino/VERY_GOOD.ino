#include "BluetoothSerial.h"
#include "ELMduino.h"
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
//#include "timeObj.h"

BluetoothSerial SerialBT; 
#define ELM_PORT SerialBT
#define I2C_ADDRESS 0x3C

#define RST_PIN -1

esp_spp_sec_t sec_mask = ESP_SPP_SEC_NONE; // or ESP_SPP_SEC_ENCRYPT|ESP_SPP_SEC_AUTHENTICATE to request pincode confirmation
esp_spp_role_t role = ESP_SPP_ROLE_MASTER; // or ESP_SPP_ROLE_MASTER
uint8_t address[6] = { 0x00, 0x10, 0xCC, 0x4F, 0x36, 0x03 };

SSD1306AsciiWire oled;
//unsigned long global_millis=0;
//unsigned long startMillis;  
unsigned long fanMillis;
unsigned long tempwarnMillis;
unsigned long throttleMillis;
//unsigned long egrMillis;
//unsigned long loadMillis;
const bool DEBUG        = false;
//const int  TIMEOUT      = 2500;
const int  FANTIMEOUT   = 30000;
const int TEMPWARNIMEOUT = 1500;
const int THROTTLETIMEOUT = 3000;
//const int  EGRTIMEOUT   = 500;
//const int  LOADTIMEOUT   = 3000;

const bool HALT_ON_FAIL = false;
const int egrpin        = 16;
const int fanpin        = 17;
//const int  vtecpin_display =14;
const int vtecpin =13;

int i=0;
const int freq = 5000;
const int resolution = 8;
int dutyCycle=0;
int globaldutyCycle=0;
String egr_onoff = "OFF";
String fan_onoff = "OFF";
bool egr = false;
bool fan= false;
bool vtech = false;
bool egr_old = false;
bool fan_old= false;
bool tempflag =false;
bool tempwarnflag=false;

bool throttleflag=false;
bool throttleonflag=false;

//timeObj  egr_timer; 
//bool egr_timer_flag = false;
bool connected1;
ELM327 myELM327;

typedef enum { ENG_RPM,
               ENG_TEMP, ENG_SPEED, ENG_THROTTLE } obd_pid_states;
obd_pid_states obd_state = ENG_RPM;

float rpm = 0;
//float load = 0;
//float prevload = 0;
//float prevkph=0;
//float loadchange = 0;
//float kphchange=0;
float temp =0;
float kph = 0;
float temp_old =0;  
//float m_temp = 0;
float throttle = 0;
float prevthrottle = 0;
float throttlediff =0;
//float rpm_old =0;

//unsigned int rpm_couter ;

int ENG_STATE[] = {

  0, 0, 0, 0
}; 

void setup()
{
 // Serial.begin(38400);
  Wire.begin();
  Wire.setClock(400000L);

#if RST_PIN >= 0
  oled.begin(&SH1106_128x64, I2C_ADDRESS, RST_PIN);
#else 
  oled.begin(&SH1106_128x64, I2C_ADDRESS);
#endif 

  oled.setFont(  fixed_bold10x15);
oled.clear();
 oled.println("Attempting \n to connect\n to ELM327...");
 ////////////////////////////////////////////////////////////////

 SerialBT.begin("ESP32", true, true);
 connected1 = SerialBT.connect(address, sec_mask, role);

  if (connected1) {
    
    
  } else {
    while (!SerialBT.connected(10000)) {

    }
  }

  if (!myELM327.begin(ELM_PORT, DEBUG))
  {
    oled.clear();
   oled.println("Couldn't connect to OBD scanner");
delay(1000);
    if (HALT_ON_FAIL)
      while (1);
  }

////////////////////////////////////////////////
//egr_timer.setTime(200);  
oled.clear();

 ledcAttach(egrpin, freq, resolution);
  ledcWrite(egrpin, 0);
//pinMode(egrpin, OUTPUT); 
pinMode(fanpin, OUTPUT); 
//digitalWrite(egrpin, LOW);
digitalWrite(fanpin, LOW);

//pinMode(vtecpin_display, INPUT);   // changed from INPUT to INPUT_PULLUP date 19/10/2024,  was back restored to INPUT 20/10/24


pinMode(vtecpin, OUTPUT); 
//digitalWrite(egrpin, LOW);
digitalWrite(vtecpin, LOW);

}

void loop()
{
  switch (obd_state)
  {
    case ENG_RPM:
    {
      rpm = myELM327.rpm();
   
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        
        obd_state = ENG_THROTTLE;
        if(rpm>0 && rpm <8000)
        ENG_STATE[0]=1;
        //endMillis = millis(); 
    //    global_millis=0;
      }
     else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        
        obd_state = ENG_THROTTLE;
          ENG_STATE[0]=0;
      }  

   break;
    }
case ENG_THROTTLE:
    {
      throttle = myELM327.throttle();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
     
        obd_state = ENG_SPEED;
        if(throttle > 0 && throttle <101)
               ENG_STATE[3]=1;
              
           if(throttle!=prevthrottle)
           {
            throttlediff = throttle -prevthrottle;
           prevthrottle = throttle;   
           }
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        //myELM327.printError();
        obd_state = ENG_SPEED;
        //ENG_STATE[0]=0;
        ENG_STATE[3]=0;
      }  

      break;  
    }
    case ENG_SPEED:
    {
      kph = myELM327.kph();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        
        obd_state = ENG_TEMP;
        if(kph>-1 && kph <180)
        ENG_STATE[2]=1;

        
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
//        myELM327.printError();
        obd_state = ENG_TEMP;
         ENG_STATE[2]=0;
       
      }

///////////////////////////////

if((rpm==0) && (throttle==0) && (kph==0)  && (dutyCycle > 0))        // If not connected to obd adapter or data is not receiving from obd

{
    closeEGR();
    egr=false;

}
/////////////////////////////
     break;
    }
    ////////////////////////// Throttle ///////////////////////////
 
 case ENG_TEMP:
    {
      temp = myELM327.engineCoolantTemp();
      
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
     
        obd_state = ENG_RPM;
        if(temp > -5 && temp <115)
               ENG_STATE[1]=1;
       
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        //myELM327.printError();
        obd_state = ENG_RPM;
        //ENG_STATE[0]=0;
        ENG_STATE[1]=0;
      }  

      break;  
    }
////////////////////////////////////////////////

     } //Switch

///////////////////// EGR OFF/////////////////

 if (  ((ENG_STATE[0]==1) && ((rpm > 0 ) && ( rpm < 1401 )))  || ((ENG_STATE[1]==1) && ((temp > 0 ) && ( temp < 46 ))) || ((ENG_STATE[2]==1) && ((kph > 0 ) && ( kph < 25 )))  )
        {

    closeEGR();
    egr=false;

        } 

////////////////////////////////////LOAD CHANGE/////////////////////////////

 

//if (((loadchange > 8) || (loadchange < -5 )   || ((ENG_STATE[3]==1)&&(load<35))  )   && (loadchangeflag == false) )           //////////  load diff above 9  and below -9 (deccelaration )   
if ((ENG_STATE[3]==1) && (throttleflag==false))     
{
  if  ((throttle>0) && (throttle<12)) 
    {
      throttleMillis = millis();
      closeEGR();
      throttleflag=true;
      throttleonflag = false;
    //  ENG_STATE[3]==0;
    }
  //loadchangeflag = true;
}

if ((ENG_STATE[3]==1) &&  (throttleflag==true))               //////// btw -15 and 15
{
  
   if ((millis()-throttleMillis) > THROTTLETIMEOUT)
    {  
     
     if ((throttle > 11) && (throttle <101))
      {
      throttleflag = false;
      throttleonflag = true;
      throttleMillis=0;

      }
     
     }
  
}

////////////////////////Load less than 30 ////////////////////////////

/*
if ((ENG_STATE[3]==1) && (loadlessflag==false))  
{
  if((load>0) && (load<30))
  {
 
  closeEGR();
  egr=false;
  loadlessflag = true;
  loadMillis = millis();

  }
  
}

if ((ENG_STATE[3]==1) && (loadlessflag==true) && ((millis()-loadMillis) > LOADTIMEOUT))
{
      //  loadchangeflag = false;
    
    loadMillis =0;

     if((load>29) && (load<100))
     loadlessflag=false;

       
}
*/

///////////////////Decline//////////////////




/////////////////////////////////////////////////////////FAN CONTROL RELAY //////////////////////////

if (ENG_STATE[1]==1 && ((temp > 93)&&(temp <110)) && fan == false) 
{
 
  fan = true;
  digitalWrite(fanpin, HIGH);
  fanMillis = millis();
 
}

if (ENG_STATE[1]==1 && ((temp>-10) && (temp < 95)) && fan == true)
{ 
  if ((millis()-fanMillis) > FANTIMEOUT) {   // run fan atleast for 30-S even if temp instatnly fall below 93
  
   fan = false;
fanMillis = 0;
digitalWrite(fanpin, LOW);
 
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//if (millis() - startMillis <= TIMEOUT)
//{
///////////////
if ( (ENG_STATE[0]==1) && (ENG_STATE[1]==1) && (ENG_STATE[2]==1) && (ENG_STATE[3]==1) )    //  if all values/data available
{

ENG_STATE[0]=0;
//ENG_STATE[1]=0;
ENG_STATE[2]=0;
//ENG_STATE[3]=0;
//if ( ((temp>50) && (temp<115))  && ((load>20)&&(load<85)) &&  ((rpm>1000)&&(rpm<3200)) &&  ((kph>-1)&&(kph<180)) && (egr==false)  )                  // load value to be checked with rpm vtec
if (((rpm>3200) &&  (rpm < 6500)) && ((kph > 0) && (kph <180)) && (vtech == false))
{
vtech = true;
closeEGR();
digitalWrite(vtecpin, HIGH);
egr = false;
}
if (((rpm>0) &&  (rpm < 3201)) && (vtech == true))
{

vtech = false;
digitalWrite(vtecpin, LOW);

}

///////////////////


///////////////////////
if  ( ((rpm>1400) && (rpm<3201))  &&   ((temp>45) && (temp<115)) &&   ((kph > 24) && (kph <180)) && (vtech==false) && (throttleonflag)  )                  // load value to be checked with rpm vtec
{
       


dutyCycle = 128;  // default dutycycle  50% = 128

 if ((kph>24) && (kph<106))
 dutyCycle = map(kph,25,105,128,255);    // 128 = 50%    255 = 100%

if ((kph>105) && (kph < 181))
dutyCycle = 255;
//ledcWrite(egrpin, dutyCycle);

if (egr==false)
{i=77;                                // 77 = 30%  30% is effective starting/opening of EGR voltage
 while (i!=(dutyCycle+1))
{
  ledcWrite(egrpin, dutyCycle);
 delay(1);
 i++;
 }
}
else
ledcWrite(egrpin, dutyCycle);

egr = true;

}
}
/*
for (i=77;i<=dutyCycle;i++)          // EGR valve opening effective from 30%     30% equal 77;
 {
  ledcWrite(egrpin, i);
 delay(1);
 }
egr = true;
}
}
*/
//}
/////////////////////////////Temp Warning flashing after one second///////////////////////////

if (ENG_STATE[1]==1 ) 
{
  if (((temp > 98)&&(temp <110)) && (tempflag==false))
{
  tempflag = true;
 tempwarnflag = true;
  tempwarnMillis = millis();
}
}

  if ( (tempflag==true) && (millis()-tempwarnMillis) > TEMPWARNIMEOUT)     
  { 

  if (ENG_STATE[1]==1  ) 
   {
      
      if  ((temp > 98)&&(temp <110))
      {
      tempwarnMillis = millis();
      if (tempwarnflag)
      tempwarnflag = false;
      else
      tempwarnflag =true;
      oled_referesh();
      }

      if((temp >0)&&(temp <99))
      {
        tempwarnMillis = 0;
        tempflag = false;
        tempwarnflag = false;
        oled_referesh();
      }
    }
  }



///////////////////////////////////////////////////////////////

 //////////////////////////////////// call oled after status change of egr fan ///////////////////////
if (dutyCycle!=globaldutyCycle)
{ 
 
 oled_referesh();
globaldutyCycle = dutyCycle ;
}

if (egr_old!=egr)
{
oled_referesh();
  egr_old =egr;
}

if (fan_old!=fan)
{
oled_referesh();
  fan_old =fan;
}

if ((ENG_STATE[1]==1) && ((temp!=0.0) && (temp_old!=temp)))
{
oled_referesh();
temp_old =temp;
}



}  /// for loop  ///




void oled_referesh(){

oled.clear();
//temp_old = temp;
oled.set2X();
if (tempwarnflag==false)
 oled.print(throttlediff);
 else
 oled.print("Warning....");
  oled.println("C");
oled.set1X();

oled.print("EGR %= ");
oled.println(map(dutyCycle,128,255,50,100));
if (vtech == true)
oled.println("VTEC = ON");
else
{
if(fan)
fan_onoff= "ON";
else
fan_onoff="OFF";

oled.print("FAN = ");
oled.println(fan_onoff);
}



}


void reconnectbt()

{

  SerialBT.begin("ESP32", true, true);
//  Serial.println("The device started in master mode, make sure remote BT device is on!");
 //SerialBT.setPin(pin);
  connected1 = SerialBT.connect(address, sec_mask, role);

  if (connected1) {
  //  Serial.println("Connected to ELM327 Bluetooth Succesfully!");


    
  } else {
    while (!SerialBT.connected(10000)) {
//Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
    }
  }
}


void closeEGR()
{
while (dutyCycle !=-1)
{

         ledcWrite(egrpin, dutyCycle);
          delay(1);
  dutyCycle--;
}
dutyCycle=0;

}
