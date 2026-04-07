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

struModbus Modbus;

static uint64_t modbus_test_time;
static uint8_t test_analog_cnt;
const uint16_t test_analog[MODBUS_ANALOG_NB_VALUE]={MODBUS_ANALOG_ALL_ABSENT, MODBUS_ANALOG_ALL_KO, MODBUS_ANALOG_ALL_MOYEN, MODBUS_ANALOG_ALL_OK};

// Création de l'objet Modbus
// ModbusRTUSlave modbus(Serial2, RS485_DE_PIN);
ModbusRTUSlave modbus(Serial2);

void modbus_init (void) 
{
    SERIAL_DEBUG("Start Modbus RTU Slave ID = " + String(SLAVE_ID));

    // Configuration UART2 pour RS485
    Serial2.begin(BAUDRATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

    // Initialisation Modbus
    modbus.begin(SLAVE_ID, BAUDRATE, SERIAL_8N1);
    modbus.configureHoldingRegisters(Modbus.holdingRegisters, DEF_HOLD_REG); 

    display.println("(id=" + String(SLAVE_ID) + ")");

    Serial.println("Modbus Slave ready !");
}

void modbus_app_test(void)
{
    if (TempsSup(modbus_test_time,MODBUS_REFRESH_TIME))
    {
        modbus_test_time = millis();
        SERIAL_DEBUG(Modbus.holdingRegisters[0]);
        SERIAL_DEBUG(Modbus.holdingRegisters[1]);
        SERIAL_DEBUG(Modbus.holdingRegisters[2]);
        SERIAL_DEBUG(Modbus.holdingRegisters[3]);
        SERIAL_DEBUG(Modbus.holdingRegisters[4]);
        // global_state ++;
        // if (global_state > MODBUS_STATE_PROG)
        // {
        //     global_state = MODBUS_STATE_END;
        // }
        // test_analog_cnt ++;
        // if (test_analog_cnt >= MODBUS_ANALOG_NB_VALUE)
        // {
        //     test_analog_cnt = 0;
        // }
        // Modbus.state.analog[0] = test_analog[test_analog_cnt];
        // Modbus.state.analog[1] = test_analog[test_analog_cnt];
        // holdingRegisters[0] = global_state;
        // holdingRegisters[1] = Modbus.state.analog[0];
        // holdingRegisters[2] = Modbus.state.analog[1];
        // holdingRegisters[3] ++;
        // holdingRegisters[4] ++;
        // holdingRegisters[5] ++;
        // holdingRegisters[6] ++;
        // holdingRegisters[7] ++;
        // holdingRegisters[8] ++;
        // holdingRegisters[9] ++;
        // holdingRegisters[10] ++;
        // holdingRegisters[11] ++;
    }   
}

void modbus_refresh (void) 
{
    if (Micro.Phase == MICRO_WAIT)
    {
        Modbus.state.global_state = MODBUS_STATE_TEST;
    }
    else if (Micro.Phase == MICRO_TEST)
    {
        Modbus.state.global_state = MODBUS_STATE_TEST;
    }    
    else if (Micro.Phase == MICRO_ARM)
    {
        Modbus.state.global_state = MODBUS_STATE_ARMED;
    }    
    else if (Micro.Phase == MICRO_FEU)
    {
        if (Micro.State == GO)
        {
            Modbus.state.global_state = MODBUS_STATE_GO;
        }
        else if (Micro.State == STOP)
        {
            Modbus.state.global_state = MODBUS_STATE_GO; // TODO estce ok ?
        }
        else if (Micro.State == END)
        {
            Modbus.state.global_state = MODBUS_STATE_END;
        }
    }
    modbus_app_test();
    modbus.poll();   // ← indispensable, traite toutes les demandes du maître
}
