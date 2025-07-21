#include <Arduino.h>
#include <SPI.h>

//Definiamo alcune costanti che verranno utilizzate in seguito:

const int CS = 44;                //Pin di selezione del TDC. Viene utilizzato nella comunicazione SPI
const int TDC_EN = 38;            //Pin di ENABLE del TDC. Da inizializzare dopo l'accensione
const int ARD_TRIGG = 42;         //Pin di TRIGGER del TDC. Da inizializzare dopo l'accensione
const int TDC_START = 34;         //Pin di invio comando di partenza della misurazione
const int ARD_BUFF_EN = 24;       //Pin di ENABLE del BUFFER
const int ARD_V_REF = DAC0;       //Pin di generazione della tensione di riferimento

const int WRITE = 1;
const int READ = 0;



//===========================================================================================================




void writeTDC(byte thisRegister, byte autoIncrement, byte thisValue)  //Si crea la funzione per l'invio dei dati dall'arduino al TDC:
{
  unsigned int sendDetails = (autoIncrement << 7) | (WRITE << 6) | (thisRegister) ; //Si crea il byte da 8 bit per il "Command field"
  Serial.println(sendDetails, BIN);
  
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
  Serial.println(command, BIN);
  SPI.transfer(command);


  // Legge i dati da MSB a LSB
  unsigned int value = 0b000000000000000000000000;
  for (int i = 0; i < length; i++) {
    value = (value << 8) | SPI.transfer(0x00);
    Serial.println(value, BIN);
  }

  digitalWrite(CS, HIGH);
  return value;
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



  digitalWrite(TDC_EN, HIGH); //Abilitiamo il TDC
  digitalWrite(ARD_BUFF_EN, HIGH); //Abilitiamo il TDC


  
  Serial.println("Scrivo i registri di configurazione:");       //[Registro, Autoincremento, valore]
  writeTDC(0x00, 0, 0x40);        //Definiamo i parametri del primo registro come: 01000000 (Pag: 25 datasheet)
  writeTDC(0x01, 0, 0x40);        //Definiamo i parametri del secondo registro come: 01000001 (Pag: 26 datasheet)
  // writeTDC(0x08, 0, 0x03);        
  // writeTDC(0x09, 0, 0x04);        
  
  delay(1500);
}



void loop() {

  digitalWrite(13, LOW);
    
  Serial.println("=====CONFIG 1======");
  int calibrazione = readTDC(0x00, 1, 1);        //[Registro, Autoincremento, Lunghezza]
  Serial.print("Valore della configurazione 1:");
  Serial.println(calibrazione, BIN);
  
  
  Serial.println("=====CONFIG 2======");
  int calibrazione2 = readTDC(0x01, 1, 1);
  Serial.print("Valore della configurazione 2:");
  Serial.println(calibrazione2, BIN);
  
  
  Serial.println("=====INT_STATUS======");
  unsigned int calibrazione3 = readTDC(0x02, 1, 1);
  Serial.print("Valore dell'INT status:");
  Serial.println(calibrazione3, BIN);
  
  
  Serial.println("=====INT_MASK======");
  unsigned int calibrazione4 = readTDC(0x03, 1, 1);
  Serial.print("Valore dell'INT mask:");
  Serial.println(calibrazione4, BIN);


  // Serial.println("=====CALIBRATION1======");
  // unsigned int calibrazione5 = readTDC(0x1B, 1, 3);
  // Serial.println(calibrazione5, BIN);
  // Serial.println("=====CALIBRATION2======");
  // unsigned int calibrazione6 = readTDC(0x1C, 1, 3);
  // Serial.println("=====STOP_MASK H======");
  // unsigned int calibrazione7 = readTDC(0x08, 1, 1);
  // Serial.println("=====STOP_MASK L======");
  // unsigned int calibrazione8 = readTDC(0x09, 1, 1);
  

  Serial.println("================================================");
  digitalWrite(13, HIGH);
  delay(9000);

}