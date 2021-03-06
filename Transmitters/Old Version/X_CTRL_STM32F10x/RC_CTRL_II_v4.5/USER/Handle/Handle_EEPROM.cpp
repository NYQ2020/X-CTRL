#include "FileGroup.h"
#include "EEPROM.h"

#define StartAddr 0x10
#define SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

using namespace EEPROM_Chs;

static uint32_t GetCheckSum(uint8_t* Array, uint32_t size)
{
    uint32_t sum = 0;
    for(uint32_t i = 0; i < size; i++)
    {
        sum += Array[i];
    }
    return sum;
}

typedef struct {
    Joystick_t JSL;
    Joystick_t JSR;
    RC_Info_t RC_Info;
	NRF_Config_TypeDef NRF_Cfg;
    bool SoundState;
    bool PassBackState;
    bool BluetoothState;
    bool HandshakeState;
    bool PowerSavingState;
    uint8_t BluetoothConnect;
} SystemConfig_t;

typedef struct {
    SystemConfig_t EEPROM_Info;
    uint32_t CheckSum;
} EEPROM_DataPack_t;

static uint16_t EEDP_Buffer[(sizeof(EEPROM_DataPack_t) / 2) + (sizeof(EEPROM_DataPack_t) % 2)];
static EEPROM_DataPack_t* EEDP = (EEPROM_DataPack_t*)EEDP_Buffer;

bool EEPROM_Handle(EEPROM_Chs_t chs)
{
    bool retval = false;
    if(chs == ReadData)
    {
        for(uint16_t i = 0; i < SIZE(EEDP_Buffer); i++)
        {
            EEPROM.read(StartAddr + i, &(EEDP_Buffer[i]));
        }
        if(EEDP->CheckSum && GetCheckSum((uint8_t*)EEDP_Buffer, sizeof(SystemConfig_t)) == EEDP->CheckSum)
        {
            JS_L = EEDP->EEPROM_Info.JSL;
            JS_R = EEDP->EEPROM_Info.JSR;
            CTRL.Info = EEDP->EEPROM_Info.RC_Info;
            NRF_Cfg = EEDP->EEPROM_Info.NRF_Cfg;
            State_BuzzSound = EEDP->EEPROM_Info.SoundState;
            State_PassBack = EEDP->EEPROM_Info.PassBackState;
            State_Handshake = EEDP->EEPROM_Info.HandshakeState;
            State_Bluetooth = EEDP->EEPROM_Info.BluetoothState;
            State_PowerSaving = EEDP->EEPROM_Info.PowerSavingState;
            Bluetooth_ConnectObject = EEDP->EEPROM_Info.BluetoothConnect;
            retval = true;
        }
    }
    else if(chs == SaveData)
    {
        EEDP->EEPROM_Info.JSL = JS_L;
        EEDP->EEPROM_Info.JSR = JS_R;
        EEDP->EEPROM_Info.RC_Info = CTRL.Info;  
		EEDP->EEPROM_Info.NRF_Cfg = NRF_Cfg;
        EEDP->EEPROM_Info.SoundState = State_BuzzSound;
        EEDP->EEPROM_Info.PassBackState = State_PassBack;
        EEDP->EEPROM_Info.HandshakeState = State_Handshake;
        EEDP->EEPROM_Info.BluetoothState = State_Bluetooth;
        EEDP->EEPROM_Info.PowerSavingState = State_PowerSaving;
        EEDP->EEPROM_Info.BluetoothConnect = Bluetooth_ConnectObject;

        EEDP->CheckSum = GetCheckSum((uint8_t*)EEDP_Buffer, sizeof(SystemConfig_t));
        for(uint16_t i = 0; i < SIZE(EEDP_Buffer); i++)
        {
            EEPROM.update(StartAddr + i, EEDP_Buffer[i]);
        }
        retval = EEPROM_Handle(ReadData);
    }
    else if(chs == ClearData)
    {
        for(uint16_t i = 0; i < SIZE(EEDP_Buffer); i++)
        {
            EEPROM.write(StartAddr + i, 0);
        }
    }
    return retval;
}
