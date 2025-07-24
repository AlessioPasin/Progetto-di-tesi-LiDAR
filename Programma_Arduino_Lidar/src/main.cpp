#include <Arduino.h>
#include <SPI.h>

//Definiamo alcune costanti che verranno utilizzate in seguito:

const int CS = 44;                //Pin di selezione del TDC. Viene utilizzato nella comunicazione SPI
const int TDC_EN = 38;            //Pin di ENABLE del TDC. Da inizializzare dopo l'accensione
const int ARD_TRIGG = 42;         //Pin di TRIGGER del TDC. Da inizializzare dopo l'accensione
const int TDC_START = 34;         //Pin di invio comando di partenza della misurazione
const int ARD_BUFF_EN = 24;       //Pin di ENABLE del BUFFER
const int ARD_V_REF = DAC0;       //Pin di generazione della tensione di riferimento
const int TDC_INTB = 48;       //Pin di generazione della tensione di riferimento

const int WRITE = 1;
const int READ = 0;

const int CalibPeriod = 10 ;    //Numero di cicli di calibrazione
const int ClockFreq = 16;        //Frequenza del clock in MHz (Noi in realtà avremmo 16MHz)
const double velLuce = 299792458.0 ; //Velocità espressa in m/s

int s = 0;                    // Variabile per la lettura dei registri di configurazione
int mod = 0;                    // Variabile di selezione del case del programma

unsigned int calib1 = 0b000000000000000000000000;        //Acquisiamo i valori
unsigned int calib2 = 0b000000000000000000000000;
unsigned int time1 = 0b000000000000000000000000;
unsigned int Clockcount1 = 0b000000000000000000000000;
unsigned int time2 = 0b000000000000000000000000;
unsigned int Clockcount2 = 0b000000000000000000000000;
unsigned int time3 = 0b000000000000000000000000;
long double calCount;
long double normLSB;
double TOF1;
double TOF2;
double TOF3;
double TOF1nano;
double TOF2nano;
double TOF3nano;
double distanza1;
double distanza2;
long double normLSBnano;
//===========================================================================================================




void writeTDC(byte thisRegister, byte autoIncrement, byte thisValue)  //Si crea la funzione per l'invio dei dati dall'arduino al TDC:
{
  unsigned int sendDetails = (autoIncrement << 7) | (WRITE << 6) | (thisRegister) ; //Si crea il byte da 8 bit per il "Command field"
  // Serial.println(sendDetails, BIN);
  
  digitalWrite(CS, LOW);              //Si inizializza la fase di scrittura al dispositivo

  SPI.transfer(sendDetails);          //Si inviano le specifiche della comunicazione
  SPI.transfer(thisValue);            //Si invia il valore da inserire nel registro
  
  digitalWrite(CS, HIGH);             //Si termina la fase di scrittura al dispositivo
}




unsigned int readTDC(byte thisRegister, byte autoIncrement, int length)   //Si crea la funzione per la lettura dei dati con autoincremento dal TDC all'Arduino:
{
  if (length < 1 || length > 3) return 0;

  digitalWrite(CS, LOW);

  // Comando: [Auto-Inc][R/W][Addr(6 bit)]
  unsigned int command = (autoIncrement << 7) | (READ << 6) | (thisRegister);
  // Serial.println(command, BIN);
  SPI.transfer(command);


  // Legge i dati da MSB a LSB
  unsigned int value = 0b000000000000000000000000;
  for (int i = 0; i < length; i++) {
    value = (value << 8) | SPI.transfer(0x00);
    // Serial.println(value, BIN);
  }
  digitalWrite(CS, HIGH);
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


//===============================================================================================================






void setup() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  Serial.begin(115200);
  Serial.println("Comunicazione inizializzata.");


  pinMode(CS, OUTPUT);                //Inizializziamo tutti i pin secondo la loro funzione
  pinMode(TDC_EN, OUTPUT);
  pinMode(ARD_TRIGG, INPUT);
  pinMode(TDC_START, OUTPUT);
  pinMode(ARD_BUFF_EN, OUTPUT);
  pinMode(ARD_V_REF, OUTPUT);
  pinMode(TDC_INTB, INPUT);
  
  
  
  digitalWrite(TDC_EN, LOW);               //Abilitiamo il TDC
  digitalWrite(ARD_BUFF_EN, HIGH);          //Imponiamo l'uscita in alta impedenza
  analogWrite(ARD_V_REF, 15);    //Impostiamo una tensione di riferimento da 0.1V
  
  delay(1500);
}



void loop() { //Costruiamo un programma basato sulla funzione switch-case: [0]=Solo lettura registri; [1]=Mod. misurazione 1; [2]=Mod. misurazione 2;
  
  mod=Serial.read(); 
  switch (mod)
{
  
  case '1':
  
  digitalWrite(TDC_EN, HIGH);               //Abilitiamo il TDC
  delay(100);
  Serial.println("=====<MODALITA' DI MISURAZIONE: 1 >======");       
  digitalWrite(ARD_BUFF_EN, LOW);         //Abilitiamo l'uscita del buffer
  Serial.println("Scrivo i registri di configurazione:");
  writeTDC(0x00, 0, 0b10000001);        //Definiamo i parametri del primo registro come: 01000000 (Pag: 25 datasheet)
  writeTDC(0x01, 0, 0b01000000);        //Definiamo i parametri del secondo registro come: 01000001 (Pag: 26 datasheet)
  
  delay(50);
  
  digitalWrite(13, LOW);
  Serial.println("<ESEGUO LA MISURAZIONE>");
  digitalWrite(TDC_START, HIGH);
  delay(1);
  digitalWrite(TDC_START, LOW);  
  delay(500);
  
  
  calib1 = readTDC(0x1B, 1, 3);        //Acquisiamo i valori
  calib2 = readTDC(0x1C, 1, 3);
  time1 = readTDC(0x10, 1, 3);
  
  
  calCount = (double(calib2 - calib1))/(CalibPeriod - 1) ;
  Serial.print("CalCount:");
  printDouble(calCount,10000);
  normLSB = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount));
  normLSBnano = (normLSB*pow(10,11));
  Serial.print("normLSBnano1:");
  printDouble(normLSBnano,100);
  
  TOF1 = time1 * normLSB;
  TOF1nano = time1 * normLSB * pow(10,9);
  Serial.print("TOF1 [ns]:");
  printDouble(TOF1nano,100);
  
  distanza1 = (velLuce * TOF1)/2 ; 
  Serial.print("Distanza1 [m]:");
  printDouble(distanza1,100);
  // digitalWrite(TDC_EN, LOW);               //Abilitiamo il TDC
  // delay(100);
  digitalWrite(13, HIGH);
  Serial.println("================================================");
  break;
  
  
  
  
  
  case '2':
  
  digitalWrite(TDC_EN, HIGH);               //Abilitiamo il TDC
  delay(100);
  Serial.println("=====<MODALITA' DI MISURAZIONE: 2 >======");       
  digitalWrite(ARD_BUFF_EN, LOW);         //Abilitiamo l'uscita del buffer
  Serial.println("Scrivo i registri di configurazione:");
  writeTDC(0x00, 0, 0b10000011);        //Definiamo i parametri del primo registro come: 01000000 (Pag: 25 datasheet)
  writeTDC(0x01, 0, 0b01000010);        //Definiamo i parametri del secondo registro come: 01000001 (Pag: 26 datasheet)
  
  delay(50);
  
  digitalWrite(13, LOW);
  Serial.println("<ESEGUO LA MISURAZIONE>");
  digitalWrite(TDC_START, HIGH);
  delay(1);
  digitalWrite(TDC_START, LOW);  
  delay(1);
  digitalWrite(TDC_START, HIGH);
  delay(1);
  digitalWrite(TDC_START, LOW);  
  delay(500);
  
  
  calib1 = readTDC(0x1B, 1, 3);        //Acquisiamo i valori
  calib2 = readTDC(0x1C, 1, 3);
  time1 = readTDC(0x10, 1, 3);
  Clockcount1 = readTDC(0x11, 1, 3);
  time2 = readTDC(0x12, 1, 3);
  Clockcount2 = readTDC(0x13, 1, 3);
  time3 = readTDC(0x14, 1, 3);
  
  
  calCount = (double(calib2 - calib1))/(CalibPeriod - 1) ;
  Serial.print("CalCount:");
  printDouble(calCount,10000);
  normLSB = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount));
  normLSBnano = (normLSB*pow(10,11));
  Serial.print("normLSBnano:");
  printDouble(normLSBnano,100);

  TOF2 = normLSB*(time1-time2 ) + (Clockcount1 * ((1.0/double((ClockFreq *pow(10,6))))));
  TOF1 = normLSB*(time1-time3 ) + (Clockcount2 * ((1.0/double((ClockFreq *pow(10,6))))));
  TOF3 = time1 * normLSB;

  TOF1nano = (TOF1 * pow(10,9));      //CORRETTO: Tempo espresso in nanosecondi [ns]
  TOF2nano = (TOF2 * pow(10,9));      //CORRETTO: Tempo espresso in nanosecondi [ns]
  TOF3nano = (TOF3 * pow(10,9));      //CORRETTO: Tempo espresso in nanosecondi [ns]
  Serial.print("TOF1 [ns]:");
  printDouble(TOF1nano,100);
  Serial.print("TOF2 [ns]:");
  printDouble(TOF2nano,100);
  Serial.print("TOF3 [ns]:");
  printDouble(TOF3nano,100);
  
  distanza1 = (velLuce * TOF1)/2 ;
  distanza2 = (velLuce * TOF2)/2 ;
  Serial.print("Distanza1 [m]:");
  printDouble(distanza1,100);
  Serial.print("Distanza2 [m]:");
  printDouble(distanza2,100);
  // digitalWrite(TDC_EN, LOW);               //Abilitiamo il TDC
  // delay(100);
  digitalWrite(13, HIGH);
  Serial.println("================================================");
  break;
  
  
  
  default:
  
  digitalWrite(TDC_EN, HIGH);               //Abilitiamo il TDC
  delay(100);
  Serial.println("=====CALIBRATION1======");
  calib1 = readTDC(0x1B, 1, 3);
  Serial.print("Valore del calibration 1:");
  Serial.println(calib1, BIN);
  
  Serial.println("=====CALIBRATION2======");
  calib2 = readTDC(0x1C, 1, 3);
  Serial.print("Valore del calibration 2:");
  Serial.println(calib2, BIN);
  
  Serial.println("=====TIME 1======");
  time1 = readTDC(0x10, 1, 3);
  Serial.print("Valore del registro TIME 1:");
  Serial.println(time1, BIN);
  Serial.println(time1, DEC);
  
  Serial.println("=====CLOCK COUNT 1======");
  Clockcount1 = readTDC(0x11, 1, 3);
  Serial.print("Valore del registro CLOCK COUNT 1:");
  Serial.println(Clockcount1, BIN);
  Serial.println(Clockcount1, DEC);
  
  Serial.println("=====TIME 2======");
  time2 = readTDC(0x12, 1, 3);
  Serial.print("Valore del registro TIME 2:");
  Serial.println(time2, BIN);
  Serial.println(time2, DEC);
  
  Serial.println("=====CLOCK COUNT 2======");
  Clockcount2 = readTDC(0x13, 1, 3);
  Serial.print("Valore del registro CLOCK COUNT 2:");
  Serial.println(Clockcount2, BIN);

  Serial.println("=====TIME 3======");
  time3 = readTDC(0x14, 1, 3);
  Serial.print("Valore del registro TIME 2:");
  Serial.println(time3, BIN);


  digitalWrite(TDC_EN, LOW);               //Abilitiamo il TDC
  delay(100);
  break;
}

delay(9000);
}