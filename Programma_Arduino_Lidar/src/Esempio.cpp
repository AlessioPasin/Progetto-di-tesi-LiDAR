#include <SPI.h>

// Pin del Chip Select (CS) del dispositivo SPI
const int CS_PIN = 10;

// Flags
const uint8_t READ_FLAG  = 0x80;  // ipotetico bit per lettura
const uint8_t AUTO_INC_FLAG = 0x40; // ipotetico bit per autoincremento

void setup() {
  Serial.begin(9600);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH); // Deseleziona il dispositivo

  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  Serial.println("SPI inizializzata.");
}

void loop() {
  // Esempio di scrittura e lettura su registro da 24 bit
  writeRegister(0x20, 0x123456, 3, true);
  uint32_t value = readRegister(0x20, 3, true);
  Serial.print("Valore letto: 0x");
  Serial.println(value, HEX);

  delay(1000);
}

/**
 * Scrive un valore su un registro del dispositivo SPI
 * @param regAddress indirizzo del registro
 * @param value valore da scrivere
 * @param numBytes numero di byte (1, 2 o 3)
 * @param autoIncrement true per attivare autoincremento
 */
void writeRegister(uint8_t regAddress, uint32_t value, uint8_t numBytes, bool autoIncrement) {
  if (numBytes < 1 || numBytes > 3) return;

  digitalWrite(CS_PIN, LOW);

  // Comando: [Auto-Inc][R/W][Addr(6 bit)]
  uint8_t command = (autoIncrement << 7) | (1 << 6) | (regAddress & 0x3F);
  SPI.transfer(command);

  // Invia i dati da MSB a LSB
  for (int i = numBytes - 1; i >= 0; i--) {
    SPI.transfer((value >> (8 * i)) & 0xFF);
  }

  digitalWrite(CS_PIN, HIGH);
}




/**
 * Legge un valore da un registro del dispositivo SPI
 * @param regAddress indirizzo del registro
 * @param numBytes numero di byte da leggere (1, 2 o 3)
 * @param autoIncrement true per attivare autoincremento
 * @return valore letto (in un uint32_t, i byte superiori saranno 0 se numBytes < 4)
 */
uint32_t readRegister(uint8_t regAddress, uint8_t numBytes, bool autoIncrement) {
  if (numBytes < 1 || numBytes > 3) return 0;

  digitalWrite(CS_PIN, LOW);

  // Comando: [Auto-Inc][R/W][Addr(6 bit)]
  uint8_t command = (autoIncrement << 7) | (0 << 6) | (regAddress & 0x3F);
  SPI.transfer(command);

  // Legge i dati da MSB a LSB
  uint32_t value = 0;
  for (int i = 0; i < numBytes; i++) {
    value = (value << 8) | SPI.transfer(0x00);
  }

  digitalWrite(CS_PIN, HIGH);
  return value;
}