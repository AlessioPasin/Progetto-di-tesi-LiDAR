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

int mod_misurazione = '0';    // Permette di avviare la modalità di misurazione del dispositivo
int s = 0;                    // Variabile per la lettura dei registri di configurazione

const int CalibPeriod = 10 ;    //Numero di cicli di calibrazione

const int ClockFreq = 16;        //Frequenza del clock in MHz (Noi in realtà avremmo 16MHz)

const double velLuce = 299792458.0 ; //Velocità espressa in m/s

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



  digitalWrite(TDC_EN, HIGH);             //Abilitiamo il TDC
  digitalWrite(ARD_BUFF_EN, LOW);        //Abilitiamo il BUFFER


  
  Serial.println("Scrivo i registri di configurazione:");       //[Registro, Autoincremento, valore]
  writeTDC(0x00, 0, 0x81);        //Definiamo i parametri del primo registro come: 01000000 (Pag: 25 datasheet)
  writeTDC(0x01, 0, 0x40);        //Definiamo i parametri del secondo registro come: 01000001 (Pag: 26 datasheet)
  // writeTDC(0x08, 0, 0x03);        
  // writeTDC(0x09, 0, 0x04);        
  
  delay(1500);
}



void loop() {

  if(s == 0){
    
    Serial.println("=====CONFIG 1======");
    int config1 = readTDC(0x00, 1, 1);        //[Registro, Autoincremento, Lunghezza]
    Serial.print("Valore della configurazione 1:");
    Serial.println(config1, BIN);
    
    
    Serial.println("=====CONFIG 2======");
    int config2 = readTDC(0x01, 1, 1);
    Serial.print("Valore della configurazione 2:");
    Serial.println(config2, BIN);
    
    
    Serial.println("=====INT_STATUS======");
    unsigned int intStatus = readTDC(0x02, 1, 1);
    Serial.print("Valore dell'INT status:");
    Serial.println(intStatus, BIN);
    
    
    Serial.println("=====INT_MASK======");
    unsigned int intMask = readTDC(0x03, 1, 1);
    Serial.print("Valore dell'INT mask:");
    Serial.println(intMask, BIN);
    
    s++;
  }
  
  
  digitalWrite(13, LOW);
  analogWrite(ARD_V_REF, 15);    //Impostiamo una tensione di riferimento da 0.1V
  mod_misurazione=Serial.read();                  //Acquisiamo il comando di avvio misurazione da tastiera. Premere 0 = Nessuna misurazione. Premere 1 = Avvio misurazione
  
  if (mod_misurazione == '1')         //Entriamo nel loop per effettuare la misurazione
  {
    Serial.println("<ESEGUO LA MISURAZIONE>");
    digitalWrite(TDC_START, HIGH);
    delay(1);
    digitalWrite(TDC_START, LOW);
    delay(1);
    // digitalWrite(TDC_START, HIGH);
    // delay(1);
    // digitalWrite(TDC_START, LOW);
    // delay(13);
  }
  
  
  delay(500);
  digitalWrite(13, HIGH);
  
  
  Serial.println("=====CALIBRATION1======");
  unsigned int calib1 = readTDC(0x1B, 1, 3);
  Serial.print("Valore del calibration 1:");
  Serial.println(calib1, BIN);
  
  Serial.println("=====CALIBRATION2======");
  unsigned int calib2 = readTDC(0x1C, 1, 3);
  Serial.print("Valore del calibration 2:");
  Serial.println(calib2, BIN);
  
  
  
  Serial.println("=====TIME 1======");
  unsigned int time1 = readTDC(0x10, 1, 3);
  Serial.print("Valore del registro TIME 1:");
  Serial.println(time1, BIN);
  
  // Serial.println("=====CLOCK COUNT 1======");
  // unsigned int clockCount1 = readTDC(0x11, 1, 3);
  // Serial.print("Valore del registro CLOCK COUNT 1:");
  // Serial.println(clockCount1, BIN);
  
  
  
  // Serial.println("=====TIME 2======");
  // unsigned int time2 = readTDC(0x12, 1, 3);
  // Serial.print("Valore del registro TIME 2:");
  // Serial.println(time2, BIN);
  
  // Serial.println("=====CLOCK COUNT 2======");
  // unsigned int clockCount2 = readTDC(0x13, 1, 3);
  // Serial.print("Valore del registro CLOCK COUNT 2:");
  // Serial.println(clockCount2, BIN);
  
  
  if (mod_misurazione == '1')
  {
    
    long double calCount = (double(calib2 - calib1))/(CalibPeriod - 1) ;
    // Serial.print("CalCount1:");
    // printDouble(calCount,10000);
    
    // Serial.println(ClockFreq *pow(10.0,6.0));
    long double normLSB = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount));
    // long double normLSBnano = ((1.0/double((ClockFreq *pow(10,6)))) * (1.0/calCount)*pow(10,9));
    // Serial.print("normLSBnano1:");
    // printDouble(normLSBnano,100);
    
    double TOF1 = time1 * normLSB;
    double TOF1nano = time1 * normLSB * pow(10,9);      //CORRETTO: Tempo espresso in nanosecondi [ns]
    Serial.print("TOF1nano [ns]:");
    printDouble(TOF1nano,100);
    
    double distanza = (velLuce * TOF1)/2 ; 
    Serial.print("Distanza1 [m]:");
    printDouble(distanza,100);
    mod_misurazione = '0';    //Facciamo in modo da eseguire un unica chiamata alla misurazione
}



Serial.println("================================================");
delay(9000);
}