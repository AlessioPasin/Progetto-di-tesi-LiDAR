#include <Arduino.h>
#include <SPI.h>

//Definiamo alcune costanti che verranno utilizzate in seguito:

const int CS = 44;    //Pin di selezione del TDC. Viene utilizzato nella comunicazione SPI

const byte WRITE = 1;
const byte READ = 0;


//===========================================================================================================



//Si crea la funzione per l'invio dei dati dall'arduino al TDC:

void writeTDC(byte autoIncrement,byte thisRegister, byte thisValue)
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
    SPI.transfer(0x00);
    // Serial.println(value, BIN);
    Serial.println(SPI.transfer(0x00));
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

  digitalWrite(38, HIGH); //Abilitiamo il TDC
  
  Serial.println("Scrivo i registri di configurazione:");
  writeTDC(0, 0x00, 0x40);        //Definiamo i parametri del primo registro come: 01000000 (Pag: 25 datasheet)
  writeTDC(0, 0x01, 0x40);        //Definiamo i parametri del secondo registro come: 01000001 (Pag: 26 datasheet)
  Serial.println("Comunicazione inizializzata.");
  
  delay(1500);
}



void loop() {

  digitalWrite(13, LOW);
    
  Serial.println("Sto per leggere:");
  unsigned int calibrazione = readTDC(0x1B, 1, 3);

  Serial.print("Il valore della calibrazione è:");
  Serial.println(calibrazione);
  Serial.println("================================================");
  digitalWrite(13, HIGH);

  delay(9000);

}