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
#define MODBUS_STATE_DECO   0
#define MODBUS_STATE_END    1
#define MODBUS_STATE_GO     2
#define MODBUS_STATE_ARMED  3
#define MODBUS_STATE_TEST   4
#define MODBUS_STATE_PROG   5

#define MODBUS_ANALOG_NB_VALUE    4
#define MODBUS_ANALOG_ALL_OK      0xFFFF
#define MODBUS_ANALOG_ALL_MOYEN   0xAAAA
#define MODBUS_ANALOG_ALL_KO      0x5555
#define MODBUS_ANALOG_ALL_ABSENT  0x0000

#define MODBUS_REFRESH_TIME       3000 // ms

uint16_t global_state = 0;                   // registre 0 : 0=DECO, 1=END, 2=GO, 3=ARMED, 4=TEST 5=PROG
uint16_t modbus_analog_register[2] = {0, 0};             // registre 1 + 2 (32 bits) : pour les 16 entrées valeur 0b00 "ABSENT", 0b01: "KO", 0b10: "MOYEN", 0b11: "OK" 
uint16_t configRegs[CONFIG_NUM_REGS] = {0};  // zone libre en lecture/écriture

const uint8_t numHoldingRegisters = 11;
uint16_t holdingRegisters[numHoldingRegisters] = {0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};

static uint64_t modbus_test_time;
static uint8_t test_analog_cnt;
const uint16_t test_analog[MODBUS_ANALOG_NB_VALUE]={MODBUS_ANALOG_ALL_ABSENT, MODBUS_ANALOG_ALL_KO, MODBUS_ANALOG_ALL_MOYEN, MODBUS_ANALOG_ALL_OK};

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

void modbus_app_test(void)
{
    if (TempsSup(modbus_test_time,MODBUS_REFRESH_TIME))
    {
        modbus_test_time = millis();
        global_state ++;
        if (global_state > MODBUS_STATE_PROG)
        {
            global_state = MODBUS_STATE_END;
        }
        test_analog_cnt ++;
        if (test_analog_cnt >= MODBUS_ANALOG_NB_VALUE)
        {
            test_analog_cnt = 0;
        }
        modbus_analog_register[0] = test_analog[test_analog_cnt];
        modbus_analog_register[1] = test_analog[test_analog_cnt];
        holdingRegisters[0] = global_state;
        holdingRegisters[1] = modbus_analog_register[0];
        holdingRegisters[2] = modbus_analog_register[1];
        holdingRegisters[3] ++;
        holdingRegisters[4] ++;
        holdingRegisters[5] ++;
        holdingRegisters[6] ++;
        holdingRegisters[7] ++;
        holdingRegisters[8] ++;
        holdingRegisters[9] ++;
        holdingRegisters[10] ++;
        holdingRegisters[11] ++;

    }   
}

void modbus_refresh (void) 
{
    if (Micro.Phase == MICRO_WAIT)
    {
        global_state = MODBUS_STATE_TEST;
    }
    else if (Micro.Phase == MICRO_TEST)
    {
        global_state = MODBUS_STATE_TEST;
    }    
    else if (Micro.Phase == MICRO_ARM)
    {
        global_state = MODBUS_STATE_ARMED;
    }    
    else if (Micro.Phase == MICRO_FEU)
    {
        if (Micro.State == GO)
        {
            global_state = MODBUS_STATE_GO;
        }
        else if (Micro.State == STOP)
        {
            global_state = MODBUS_STATE_GO; // TODO estce ok ?
        }
        else if (Micro.State == END)
        {
            global_state = MODBUS_STATE_END;
        }
    }
    holdingRegisters[0] = global_state;
    holdingRegisters[1] = modbus_analog_register[0];
    holdingRegisters[2] = modbus_analog_register[1];

    modbus.poll();   // ← indispensable, traite toutes les demandes du maître

    // modbus_app_test();
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