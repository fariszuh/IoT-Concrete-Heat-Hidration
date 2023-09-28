#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <AverageThermocouple.h>
#include <MAX6675_Thermocouple.h>
#include <SmoothThermocouple.h>
#include <Thermocouple.h>
#include <SPI.h>
#include <SD.h>

#include <LiquidCrystal_I2C.h>
//---INISIALISASI THERMOCOUPLE 1-----------
int pinSO1 = 32;
int pinCS1 = 4;
int pinSCK1 = 33;
float suhuRumus1=0.0;
#define SMOOTHING_FACTOR 5
Thermocouple* thermocouple1 = NULL;
//---INISIALISASI THERMOCOUPLE 2-----------
int pinSO2 = 14;
int pinCS2 = 27;
int pinSCK2 = 26;
float suhuRumus2=0.0;
Thermocouple* thermocouple2 = NULL;
//---INISIALISASI DIGITAL CALIPER--------
int i;
int sign;
long value;
float result=0;
int clockpin = 15;  
int datapin = 34;
unsigned long tempmicros;

//---INISIALISASI RTC---------------------
#define DS3231_I2C_ADDRESS 0x68

byte decToBcd(byte val){
  return( (val/10*16) + (val%10) );
}
byte bcdToDec(byte val){
  return( (val/16*10) + (val%16) );
}
int jedaMenit = 1; //jeda waktu antar pengukuran dlm menit
int jedaDetik = 10; //jeda waktu antar pengukuran dlm menit
//---INISIALISASI LCD I2C------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long millis_tampilan;
byte symKetinggian[] = {
  B00100,
  B01010,
  B10101,
  B00100,
  B00100,
  B10101,
  B01010,
  B00100
};

byte symSuhu[] = {
  B01101,
  B01100,
  B01110,
  B01100,
  B01100,
  B10010,
  B10010,
  B01100
};

//---INISIALISASI WIFI ESP32 & HTTP GET----
const char* host = "hidrasibeton.ewatery.tech";
const int httpPort = 80;
const char* ssid     = "apoci";
const char* password = "apoci123";
long last_millis_wifi;
String wifi="OFF";
bool mode;
WiFiClient client;

//---DATALOG MICRO SD----------------------
const int CS_uSD = 2; //chip select
File myFile;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  //lcd.init();
  lcd.begin();
  lcd.createChar(0, symKetinggian);
  lcd.createChar(1, symSuhu);
  
  //---CONNECTING TO WIFI HOTSPOT---------
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.setCursor (0,0);
  lcd.print("Connecting.....");
  Serial.print("Connecting....");
  lcd.setCursor (0,1);
  lcd.print(ssid);
  //tunggu connect wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.setCursor (0,0);
  lcd.print("Connecting.....");
  lcd.setCursor (0,1);
  lcd.print("SUCCESS!");
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  tampilPengukuran();
  
  //---SMOOTHING FOR THERMOCOUPLE1---------
  Thermocouple* originThermocouple1 = new MAX6675_Thermocouple(pinSCK1, pinCS1, pinSO1);
  thermocouple1 = new SmoothThermocouple(originThermocouple1, SMOOTHING_FACTOR);
  //---SMOOTHING FOR THERMOCOUPLE2---------
  Thermocouple* originThermocouple2 = new MAX6675_Thermocouple(pinSCK2, pinCS2, pinSO2);
  thermocouple2 = new SmoothThermocouple(originThermocouple2, SMOOTHING_FACTOR);
  
  //---DS3231------ FORMAT SETTING: seconds, minutes, hours, day, date, month, year
  //setDS3231time(50,02,15,06,15,07,22); //UNCOMMENT INI UNTUK PERTAMA KALI SETTING TIME DS1307

/*
  //---SD CARD-----
  SD.begin();
  if (SD.begin())
  {
    Serial.println("SD card is present & ready");
  } 
  else
  {
    Serial.println("SD card missing or failure");
    return;
    //while(1); //halt program
  }
  //SECTION clear out old data file
  if (SD.exists("data_pengukuran.txt")) 
  {
    Serial.println("Removing data_pengukuran.txt");
    SD.remove("data_pengukuran.txt");
    Serial.println("Done removing");
  }
  //SECTION write data
  //write csv headers to file:
   
   myFile = SD.open("data_pengukuran.txt", FILE_WRITE);  
   if (myFile) // it opened OK
    {
    Serial.println("Writing headers to data_pengukuran.txt");
    myFile.println("Year, Month, Day, Hour, Minute, Second, Suhu1, Suhu2, Panjang Caliper");
    myFile.close(); 
    Serial.println("Headers written");
    }
  else 
    Serial.println("Error opening data_pengukuran.txt");
    */
  pinMode(clockpin, INPUT);
  pinMode(datapin, INPUT);
  pinMode(CS_uSD, OUTPUT);
  Serial.println("Mulai baca caliper dan thermocouple");
}

void loop () {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
  //jika jeda untuk detik, condition: detik%jedaDetik==0 saja
  //jika jeda untuk menit, condition: detik%60==0 && menit%jedaMenit==0
  //jika jeda untuk jam, condition: detik%60==0 && menit%60==0 && jam%jedaJam==0
  
  
  if (second%jedaDetik == 0){
    Serial.print(hour);
    Serial.print(minute);
    Serial.print(second);
    //------BACA THERMOCOUPLE 1&2-----
    const double suhu1 = thermocouple1->readCelsius(); 
    const double suhu2 = thermocouple2->readCelsius(); 
    //------BACA CALIPER SAJA-----
    while (digitalRead(clockpin)==HIGH) {} //if clock is LOW wait until it turns to HIGH
    tempmicros=micros();
    while (digitalRead(clockpin)==LOW) {} //wait for the end of the HIGH pulse
    if ((micros()-tempmicros)>500) { //if the HIGH pulse was longer than 500 micros we are at the start of a new bit sequence
      decode(); //decode the bit sequence
    }
    Serial.print("Pembacaan: caliper = ");  
    Serial.print(result,2); //print result with 2 decimals
    Serial.print("mm ; TE1 =  ");
    suhuRumus1 = 1*suhu1 - 1.35; //linearisasi thermocouple
    Serial.print(suhuRumus1);
    Serial.print(" C ; TE2 =  ");  
    suhuRumus2 = 1*suhu2 - 1.35; //linearisasi thermocouple
    Serial.print(suhuRumus2);
    /*
    myFile = SD.open("data_pengukuran.txt", FILE_WRITE);     
        // if the file opened okay, write to it:
        if (myFile) 
        {
          Serial.println("Writing to txt");
          myFile.println(year);
          myFile.println(month); 
          myFile.println(dayOfMonth); 
          myFile.println(hour); 
          myFile.println(minute); 
          myFile.println(second); 
          myFile.println(suhuRumus1); 
          myFile.println(suhuRumus2); 
          myFile.println(result); 
          myFile.close();
        } 
        else 
        {
          Serial.println("error opening txt");
        }
      */
    //writeToMicroSD(year,month,dayOfMonth,hour,minute,second,suhuRumus1,suhuRumus2,result);
    String protokol = "/tes.php?timeSampling="+
        String(year)+String(month)+String(dayOfMonth)+String(hour)+String(minute)+String(second)+
        "&suhu="+String(suhuRumus1)+
        "&panjang="+String(result);
    Serial.println( "Send Data To Server:"+send(protokol));
  }
  if (millis()-millis_tampilan<5000){
      tampilPengukuran();}
  else  if (millis()-millis_tampilan<8000){
      splashScreen();}
  else{
      millis_tampilan=millis();
    }
}
