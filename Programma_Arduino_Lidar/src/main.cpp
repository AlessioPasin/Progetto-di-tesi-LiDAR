#include <Arduino.h>
#include <SPI.h>
#include <math.h>

//Definiamo alcune costanti che verranno utilizzate in seguito:

const int CS = 44;    //Pin di selezione del TDC. Viene utilizzato nella comunicazione SPI

const byte WRITE = 1;
const byte READ = 0;
int a = 0;



//Definiamo dei valori pari a quelli che dovremmo ottenere dai registri del TDC. Si considerano gli esempi forniti dal datasheet:

const int Calib1byte1 = 0b00000000 ;    //MSB
const int Calib1byte2 = 0b01010010 ;
const int Calib1byte3 = 0b10000001 ;    //LSB

const int Calib2byte1 = 0b00000000 ;    //MSB
const int Calib2byte2 = 0b00001000 ;
const int Calib2byte3 = 0b00111110 ;    //LSB

const int CalibPeriod = 10 ;    //Numero di cicli di calibrazione

const int ClockFreq = 8;        //Frequenza del clock in MHz (Noi in realtà avremmo 16MHz)

const int Time1byte1 = 0b00000000 ;     //MSB
const int Time1byte2 = 0b00010000 ;
const int Time1byte3 = 0b01001111 ;     //LSB





//===========================================================================================================
/*


//Si crea la funzione per l'invio dei dati dall'arduino al TDC:

void writeTDC(byte thisRegister, byte autoIncrement, byte thisValue)
{
  unsigned int sendDetails = (autoIncrement << 7) | (WRITE << 6) | (thisRegister & 0x3F) ; //Si crea il byte da 8 bit per il "Command field"
  Serial.println(sendDetails, BIN);
  
  digitalWrite(CS, LOW);              //Si inizializza la fase di scrittura al dispositivo

  SPI.transfer(sendDetails);          //Si inviano le specifiche della comunicazione
  SPI.transfer(thisValue);            //Si invia il valore da inserire nel registro
  
  digitalWrite(CS, HIGH);             //Si termina la fase di scrittura al dispositivo
}



//Si crea la funzione per la lettura dei dati con autoincremento dal TDC all'Arduino:


unsigned int readTDC(byte thisRegister, byte autoIncrement, int length)
{
  if (length < 1 || length > 3) return 0;

  digitalWrite(CS, LOW);

  // Comando: [Auto-Inc][R/W][Addr(6 bit)]
  unsigned int command = (autoIncrement << 7) | (READ << 6) | (thisRegister & 0x3F);
  Serial.println(command, BIN);
  SPI.transfer(command);



  // Legge i dati da MSB a LSB
  unsigned int value;
  for (int i = 0; i < length; i++) {
    // value = (value << 8) | SPI.transfer(0x00);
    // Serial.println(value, BIN);
    Serial.print("Trasferito: ");
    Serial.println(SPI.transfer(0x00), BIN);
  }

  digitalWrite(CS, HIGH);
  return value;
}

*/
//===============================================================================================================
//DUMMY FUNCTION, solo per creare la funzione di riferimento:

unsigned int readTDCcalib1(int length)
{
  if (length < 1 || length > 3) return 0;
  
  // Legge i dati da MSB a LSB
  unsigned int value;
  value = (Calib1byte1 << 16) | (Calib1byte2 << 8) | (Calib1byte3);
  return value;
}

unsigned int readTDCcalib2(int length)
{
  if (length < 1 || length > 3) return 0;
  
  // Legge i dati da MSB a LSB
  unsigned int value;
  value = (Calib2byte1 << 16) | (Calib2byte2 << 8) | (Calib2byte3);
  return value;
}

unsigned int readTDCTime1(int length)
{
  if (length < 1 || length > 3) return 0;
  
  // Legge i dati da MSB a LSB
  unsigned int value;
  value = (Time1byte1 << 16) | (Time1byte2 << 8) | (Time1byte3);
  return value;
}


void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

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

  Serial.println("=====CALIB 1======");
  unsigned int calibrazione1 = readTDCcalib2(3);        //[Registro, Autoincremento, Lunghezza]
  Serial.println(calibrazione1, BIN);
  Serial.println(calibrazione1);
  
  Serial.println("=====CALIB 2======");
  unsigned int calibrazione2 = readTDCcalib1(3);        //[Registro, Autoincremento, Lunghezza]
  Serial.println(calibrazione2, BIN);
  Serial.println(calibrazione2);
  
  Serial.println("=====TIME 1======");
  unsigned int time1 = readTDCTime1(3);        //[Registro, Autoincremento, Lunghezza]
  Serial.println(time1, BIN);
  Serial.println(time1);
  
  Serial.println("================================================");   //Abbiamo costruito correttamente i valori di riferimento


  /*=====PRIMA MODALITA' DI FUNZIONAMENTO======*/

  long double calCount = (double(calibrazione2 - calibrazione1))/(CalibPeriod - 1) ;
  printDouble(calCount,10000);
  
  // Serial.println(ClockFreq *pow(10.0,6.0));
  long double normLSB = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount));
  long double normLSBnano = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount)*pow(10,9));
  printDouble(normLSBnano,100);
  
  double TOF1 = time1 * normLSB;
  double TOF1nano = time1 * normLSB * pow(10,9);      //CORRETTO: Tempo espresso in nanosecondi [ns]
  printDouble(TOF1nano,100);









  delay(1500);
}



void loop() {

  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}