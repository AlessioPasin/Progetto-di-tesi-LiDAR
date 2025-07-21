#include <Arduino.h>
#include <SPI.h>
#include <math.h>

//Definiamo alcune costanti che verranno utilizzate in seguito:

const double velLuce = 299792458.0 ; //Velocità espressa in m/s

//Definiamo dei valori pari a quelli che dovremmo ottenere dai registri del TDC. Si considerano gli esempi forniti dal datasheet:

int Calib1byte[3] = {0b00000000, 0b00001000, 0b00111110} ;    //LSB

int Calib2byte[3] = {0b00000000, 0b01010010, 0b10000001} ;    //LSB

int Time1byte[3] = {0b00000000, 0b00010000, 0b01001111} ;    //LSB




int Calib1byte2[3] = {0b00000000, 0b00001001, 0b00001011} ;    //LSB

int Calib2byte2[3] = {0b00000000, 0b01011010, 0b01011101} ;    //LSB

int Time1byte2[3] = {0b00000000, 0b00001000, 0b01100011} ;    //LSB

int Time2byte2[3] = {0b00000000, 0b00000000, 0b11001001} ;    //LSB

int ClockCount1byte2[3] = {0b00000000, 0b00000001, 0b00111110} ;    //LSB

const int CalibPeriod = 10 ;    //Numero di cicli di calibrazione

const int ClockFreq = 8;        //Frequenza del clock in MHz (Noi in realtà avremmo 16MHz)


//===============================================================================================================
//DUMMY FUNCTIONs, solo per creare la funzione di riferimento:

unsigned int readTDC(int valore[], int length)
{
  if (length < 1 || length > 3) return 0;
  
  // Legge i dati da MSB a LSB
  unsigned int value = 0b000000000000000000000000;
  for (int i = 0; i < length; i++) {
      value = (value << (8)) | valore[i];
    }
  return value;
}



void printDouble( double val, unsigned int precision){  //Funzione di stampa dei valori double

   Serial.print (int(val));  //prints the int part
    Serial.print("."); // print the decimal point
    unsigned int frac;
    if(val >= 0)
      frac = (val - int(val)) * precision;
    else
       frac = (int(val)- val ) * precision;
    int frac1 = frac;
    while( frac1 /= 10 )
        precision /= 10;
    precision /= 10;
    while(  precision /= 10)
        Serial.print("0");

    Serial.println(frac,DEC) ;
} 









void setup() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  Serial.begin(115200);  
  Serial.println("Comunicazione inizializzata.");
  pinMode(13, OUTPUT);


  Serial.print("CALIB 1:");
  unsigned int calibrazione1 = readTDC(Calib1byte, 3);
  Serial.println(calibrazione1);

  Serial.print("CALIB 2:");
  unsigned int calibrazione2 = readTDC(Calib2byte, 3);
  Serial.println(calibrazione2);
  
  Serial.print("TIME 1:");
  unsigned int time1 = readTDC(Time1byte, 3);
  Serial.println(time1);
  
  
  
  /*=====PRIMA MODALITA' DI FUNZIONAMENTO======*/
  
  long double calCount = (double(calibrazione2 - calibrazione1))/(CalibPeriod - 1) ;
  Serial.print("CalCount1:");
  printDouble(calCount,10000);
  
  // Serial.println(ClockFreq *pow(10.0,6.0));
  long double normLSB = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount));
  long double normLSBnano = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount)*pow(10,9));
  Serial.print("normLSBnano1:");
  printDouble(normLSBnano,100);
  
  double TOF1 = time1 * normLSB;
  double TOF1nano = time1 * normLSB * pow(10,9);      //CORRETTO: Tempo espresso in nanosecondi [ns]
  Serial.print("TOF1nano [us]:");
  printDouble(TOF1nano,100);
  
  double distanza = (velLuce * TOF1)/2 ; 
  Serial.print("Distanza1 [m]:");
  printDouble(distanza,100);
  
  
  
  
  Serial.println("====================== MODALITA' DUE ==========================");
  
  
  
  Serial.print("CALIB 1:");
  unsigned int calibrazione12 = readTDC(Calib1byte2, 3);
  Serial.println(calibrazione12);

  Serial.print("CALIB 2:");
  unsigned int calibrazione22 = readTDC(Calib2byte2, 3);
  Serial.println(calibrazione22);
  
  Serial.print("TIME 1:");
  unsigned int time12 = readTDC(Time1byte2, 3);
  Serial.println(time12);
  
  Serial.print("TIME 2:");
  unsigned int time22 = readTDC(Time2byte2, 3);
  Serial.println(time22);
  
  Serial.print("CLOCK_COUNT1:");
  unsigned int Clockcount1 = readTDC(ClockCount1byte2, 3);
  Serial.println(Clockcount1);


  Serial.println("================================================");   //Abbiamo costruito correttamente i valori di riferimento





  /*=====SECONDA MODALITA' DI FUNZIONAMENTO======*/

  long double calCount2 = (double(calibrazione22 - calibrazione12))/(CalibPeriod - 1) ;
  Serial.print("CalCount2:");
  printDouble(calCount2,10000);
  
  // Serial.println(ClockFreq *pow(10.0,6.0));
  long double normLSB2 = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount2));
  long double normLSBnano2 = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount2)*pow(10,12));
  Serial.print("normLSBpico:");
  printDouble(normLSBnano2,100);
  
  double TOF12 = (time12 * normLSB2) + (Clockcount1 * ((1.0/double((ClockFreq *pow(10,6)))))) - (time22 * normLSB2);
  double TOF12nano = (TOF12 * pow(10,6));      //CORRETTO: Tempo espresso in nanosecondi [ns]
  Serial.print("TOF2micro [us]:");
  printDouble(TOF12nano,100);
  
  double distanza2 = (velLuce * TOF12)/2 ;
  Serial.print("Distanza2 [m]:");
  printDouble(distanza2,100);

  delay(1500);
}



void loop() {

  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}