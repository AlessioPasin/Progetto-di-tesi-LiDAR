#include <Arduino.h>
#include <SPI.h>

//Definiamo alcune costanti che verranno utilizzate in seguito:

const int CS = 44;    //Pin di selezione del TDC. Viene utilizzato nella comunicazione SPI

const byte WRITE = 1;
const byte READ = 0;
int a = 0;




//===========================================================================================================



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


//===============================================================================================================






void setup() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
  Serial.begin(115200);

  pinMode(CS, OUTPUT);
  pinMode(38, OUTPUT);

  digitalWrite(38, HIGH); //Abilitiamo il TDC
  
  Serial.println("Scrivo i registri di configurazione:");       //[Registro, Autoincremento, valore]
  writeTDC(0x00, 0, 0x40);        //Definiamo i parametri del primo registro come: 01000000 (Pag: 25 datasheet)
  writeTDC(0x01, 0, 0x40);        //Definiamo i parametri del secondo registro come: 01000001 (Pag: 26 datasheet)
  // writeTDC(0x08, 0, 0x03);        
  // writeTDC(0x09, 0, 0x04);        
  Serial.println("Comunicazione inizializzata.");
  
  delay(1500);
}



void loop() {

  digitalWrite(13, LOW);
    
  Serial.println("=====CONFIG 1======");
  unsigned int calibrazione = readTDC(0x00, 1, 1);        //[Registro, Autoincremento, Lunghezza]
  Serial.println("=====CONFIG 2======");
  unsigned int calibrazione2 = readTDC(0x01, 1, 1);
  Serial.println("=====INT_STATUS======");
  unsigned int calibrazione3 = readTDC(0x02, 1, 1);
  Serial.println("=====INT_MASK======");
  unsigned int calibrazione4 = readTDC(0x03, 1, 1);
  Serial.println("=====CALIBRATION1======");
  unsigned int calibrazione5 = readTDC(0x1B, 1, 3);
  Serial.println("=====CALIBRATION2======");
  unsigned int calibrazione6 = readTDC(0x1C, 1, 3);
  Serial.println("=====STOP_MASK H======");
  unsigned int calibrazione7 = readTDC(0x08, 1, 1);
  Serial.println("=====STOP_MASK L======");
  unsigned int calibrazione8 = readTDC(0x09, 1, 1);
  

  Serial.println("================================================");
  digitalWrite(13, HIGH);
  
  
  
  // Serial.print(a);
  // if (a == 0)
  // {
  //   digitalWrite(34, HIGH);
  //   delay(100);
  //   digitalWrite(34, LOW);
  //   delay(100);
  //   a++;
  // }
  

  delay(9000);

}