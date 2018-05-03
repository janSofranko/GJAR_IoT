//include stuff



//
#define DHTPIN ?


//
#define EEPROM_DATA 0

//globals
float temperature;
float humidity;


void setup() 
{
  pinMode(DHTPIN, INPUT);


}

void loop() 
{


}

void GetSensorData()
{
  
}
//-------------------- nechcem nic mazat a prepisovat tak to dam pod tu schemu co ste urobili---------------------------------------------
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

extern "C" {
#include "user_interface.h"
}

#define DHT22_INTERVAL   2000
#define LDR_INTERVAL     2000

#define tmpT_INTERVAL    3 // z kolkych hodnot sa pocita priemer teploty
#define dispAVG_INTERVAL  3 // po kolkych meraniach sa posiela priemer teploty

#define DHTPIN 4
#define DHTTYPE DHT22
//------------------------------------------------
struct 
{
  float hum;              // vlhkost
  float temp;             // teplota v C
  float tempF;            // teplot v F
  byte FailedRead:1;      // priznak chyby nacitania
  byte unused:7;          // zatial nepouzite
}sDHT22;


struct
{
  byte  HUM:1;
  byte  LDR:1;
  byte  unused:6;
}sCMD;

float tmpT[tmpT_INTERVAL];  // tmpT hodnoty, z kt. sa bude pocitat priemer
float avgT; // priemerna teplota;
int itmpT;
int dispAVG; //zobrazi priemernu teplotu ked disAVG bude rovne nule

float tmpH[tmpT_INTERVAL];  // tmpH hodnoty, z kt. sa bude pocitat priemer
float avgH; // priemerna vlhkost;

//----------------------------------------------------------
DHT dht(DHTPIN, DHTTYPE);
//---------------------------------------------------------
os_timer_t timDHT22;
os_timer_t timLDR;
//-----------------------------------------------------------
// Funkcia, kt sa spusti ked nastane timer pre DHT22
void getDHT22Data(void *pArg)
{
  sCMD.HUM = true;      // nastavi flag pre citanie Humanity
}
//----------------------------------------------------------------
//funkcia, kt sa spusti ked nastane timer pre LDR
void getLDRdata(void *pArg)
{
  sCMD.LDR = true;      // nastavi flag pre citanie LDR
}
//-----------------------------------------------------------------
void clear_sCMD(void)
{
  memset(&sCMD,0,sizeof(sCMD));    // nastavi sCMD na nulu
}


//------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  dht.begin();

  os_timer_setfn(&timDHT22, getDHT22Data, NULL);     // spustenie a nastavernie Timera pre snimanie teploty a spol.
  os_timer_arm(&timDHT22,DHT22_INTERVAL, true);       

  os_timer_setfn(&timLDR, getLDRdata, NULL);     // spustenie a nastavernie Timera LDR.
  os_timer_arm(&timLDR, LDR_INTERVAL , true);      

  clear_sCMD();   // vynuluje sCMD

 memset(tmpT,0,tmpT_INTERVAL*sizeof(tmpT)); //vynulovanie namerane hodnoty teploty
 itmpT = 0; // vynulovanie indexu nameranej teploty

 memset(tmpH,0,tmpT_INTERVAL*sizeof(tmpH)); //vynulovanie namerane hodnoty vlhkosti


  Serial.println("----START----");
 

}

void loop() {

    if(sCMD.HUM)          // nacitavanie DHT22
    {
       ReadDHT22(true);
       sCMD.HUM=false;
    }
    
    if(sCMD.LDR)          // nacitavanie LDR
    {
       readLDRdata(true);
       sCMD.LDR=false;       
    }
}
//-------------------------------------------------------------------
void ReadDHT22(bool debug)
{
  int i;
  sDHT22.hum = dht.readHumidity();                 // nacita data zo senzora
  sDHT22.temp = dht.readTemperature();
  sDHT22.tempF = dht.readTemperature(true);

  if (isnan(sDHT22.hum) || isnan(sDHT22.temp))      // ak je chyba citania
  {
    sDHT22.FailedRead = true;
  }
  else
  {
    sDHT22.FailedRead = false;           // ak nie, tak je ten flag false

   tmpT[itmpT] = sDHT22.temp; //ulozi teplotu do pola teplot 
   tmpH[itmpT] = sDHT22.hum; //ulozi vlhkost do pola vlhkosti
   itmpT++;
   if (itmpT>(tmpT_INTERVAL-1)) itmpT = 0;     // ak je index pola teplot vacsi ako 2 vynuluje ho

   
   avgT = 0; //vynuluje sa priemerna teplota
   avgH = 0; //vznuluje sa priemerna vlhkost
  for (i=0;i<tmpT_INTERVAL;i++) // spocitame vsetky ulozene teploty
  {
   avgT = avgT+tmpT[i];
   avgH = avgH+tmpH[i];
    }
    avgT = avgT/tmpT_INTERVAL; //priemerna teplota je podiel spocitanych teplot s ich poctom
    avgH = avgH/tmpT_INTERVAL;
    
  }

  if(debug)   // ak sa ma vypisovat
  {
   // Serial.print("DHT22: ");
    if(sDHT22.FailedRead)
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
    /*
      Serial.print("Hum: "); 
      Serial.print(sDHT22.hum);
      Serial.print(" %  ");
      Serial.print("Temp: "); 
      Serial.print(sDHT22.temp);
      Serial.print(" °C ~ ");
      Serial.print(sDHT22.tempF);
      Serial.println(" °F");
*/

      if(dispAVG==0) // ak dispAVG je nula zobrazi priemerne hodnoty
      {
      Serial.print("DHT:  ");  
      Serial.print("AVG Hum: "); 
      Serial.print(avgH);
      Serial.print(" %  ");  
      Serial.print("AVG Temp: "); 
      Serial.print(avgT);
      Serial.println(" °C ~ ");
      
      dispAVG++;
      }
      else // ak dispAVG je vacsie ako 0 tak ho inkrementuje 
      {
        dispAVG++;
        if(dispAVG>(dispAVG_INTERVAL-1)) dispAVG = 0; // ak dispAVG dosiahne hodnotu pri ktorej sa ma zobrazit priemerna hodnota dispAVG vynuluje
      }

      
    }
  }
}

//--------------------------------------------------------------------------------
void readLDRdata(bool debug)
{
   int sensorValue = analogRead(A0);   // read the input on analog pin 0
   float voltage = sensorValue * (5.0 / 1023.0);   // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)
  if(debug)
    {
     Serial.print("LDR: ");
     Serial.print(voltage);   // print out the value you read
    Serial.println(" V");
    }
  
}
