/*********************************************************************
  Modbus RTU Slave pour ESP32
  Mapping exact pour ton superviseur NiceGUI
  - État global sur 1 octet (registre 100, LSB)
  - 16 entrées analogiques × 2 bits (registres 101 + 102)
  - Zone configuration en lecture/écriture (à partir du registre 200)
*********************************************************************/

#include "includes.h"
#include "ModbusRTUSlave.h"

// ================== CONFIGURATION (à changer par ESP32) ==================
#define SLAVE_ID          1          // ← CHANGE CE NUMÉRO pour chaque ESP32 (1 à 20)

#define RS485_RX_PIN      16         // GPIO16 → RO du MAX485
#define RS485_TX_PIN      17         // GPIO17 → DI du MAX485
// #define RS485_DE_PIN      18         // GPIO18 → DE + RE du MAX485 (tous les deux reliés)

#define BAUDRATE          9600
#define GLOBAL_STATE_REG  0
#define ANALOG_START_REG  1
#define CONFIG_START_REG  3
#define CONFIG_NUM_REGS   8          // nombre de registres de configuration

// Variables Modbus (holding registers)
uint16_t globalState = 0;                    // 0=END, 1=GO, 2=ARMED, 3=TEST
uint16_t analogRegs[2] = {0, 0};             // registre 101 + 102 (32 bits)
uint16_t configRegs[CONFIG_NUM_REGS] = {0};  // zone libre en lecture/écriture

const uint8_t numHoldingRegisters = 11;
uint16_t holdingRegisters[numHoldingRegisters] = {3, 0xFFFA, 0xAA55, 1, 2, 3, 4, 5, 6, 7, 8};

// Création de l'objet Modbus
// ModbusRTUSlave modbus(Serial2, RS485_DE_PIN);
ModbusRTUSlave modbus(Serial2);

void modbus_init (void) 
{
  SERIAL_DEBUG("Start Modbus RTU Slave ID =" + String(SLAVE_ID));

  // Configuration UART2 pour RS485
  Serial2.begin(BAUDRATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

  // Initialisation Modbus
  modbus.begin(SLAVE_ID, BAUDRATE, SERIAL_8N1);
  modbus.configureHoldingRegisters(holdingRegisters, numHoldingRegisters); 

//   // Mise en place des registres avec valeurs initiales
//   modbus.holdingRegisterWrite(GLOBAL_STATE_REG, globalState);
//   modbus.holdingRegisterWrite(ANALOG_START_REG,     analogRegs[0]);
//   modbus.holdingRegisterWrite(ANALOG_START_REG + 1, analogRegs[1]);

//   for (int i = 0; i < CONFIG_NUM_REGS; i++) {
//     modbus.holdingRegisterWrite(CONFIG_START_REG + i, configRegs[i]);
//   }

  Serial.println("Modbus Slave ready !");
}

void modbus_refresh (void) 
{
  modbus.poll();   // ← indispensable, traite toutes les demandes du maître
// Serial2.println("test !");
  // ================== EXEMPLE : mise à jour des valeurs ==================
  // Tu peux remplacer ces lignes par tes capteurs, boutons, etc.

//   // Exemple état global (change selon ton code)
//   globalState = 2;                    // 2 = ARMED par exemple
//   modbus.holdingRegisterWrite(GLOBAL_STATE_REG, globalState);

//   // Exemple : mise à jour des 16 entrées analogiques (2 bits chacune)
//   uint32_t bits32 = 0;
//   // Exemple fictif : entrée 0 = OK (11), entrée 1 = KO (10), etc.
//   bits32 |= (0b11 << 0);   // entrée 1  → OK
//   bits32 |= (0b10 << 2);   // entrée 2  → KO
//   bits32 |= (0b01 << 4);   // entrée 3  → MOYEN
//   // ... continue jusqu'à 16

//   analogRegs[0] = bits32 & 0xFFFF;          // bits bas
//   analogRegs[1] = (bits32 >> 16) & 0xFFFF;  // bits haut

//   modbus.holdingRegisterWrite(ANALOG_START_REG,     analogRegs[0]);
//   modbus.holdingRegisterWrite(ANALOG_START_REG + 1, analogRegs[1]);

  // Tu peux aussi lire la config envoyée par le maître :
  // uint16_t nouvelleValeur = modbus.holdingRegisterRead(CONFIG_START_REG);

//   delay(10);   // petite pause (pas obligatoire)
}

// static uint16_t lastConfig = 0;
// uint16_t current = modbus.holdingRegisterRead(CONFIG_START_REG);
// if (current != lastConfig) {
//   lastConfig = current;
//   Serial.printf("Nouvelle config reçue : 0x%04X\n", current);
//   // fais ce que tu veux ici
// }