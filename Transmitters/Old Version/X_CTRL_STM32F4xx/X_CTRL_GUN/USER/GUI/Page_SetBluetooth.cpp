#include "FileGroup.h"
#include "DisplayPrivate.h"
#include "ComPrivate.h"

static bool State_Bluetooth_Last;

/*蓝牙连接对象*/
uint8_t Bluetooth_ConnectObject = BC_Type::BC_XFS;

/*实例化HC05蓝牙对象*/
Bluetooth_HC05 hc05(&Bluetooth_SERIAL, BT_State_Pin, BT_EN_Pin, BT_PowerCtrl_Pin);

/*蓝牙可选波特率*/
static const uint32_t UseBaudRate[] = {1200, 2400, 4800, 9600, 14400, 19200, 38400, 43000, 57600, 76800, 115200};

/*蓝牙可选波特率个数*/
#define UseBaudRate_Size __Sizeof(UseBaudRate)

/*蓝牙波特率选择项*/
static uint8_t BaudRateSelect = 0;

/*密码*/
static String Password;

/*主从角色*/
static bool Role;

/*选项最大个数*/
#define ItemSelect_MAX 5

/*选项字符串缓冲*/
static String ItemStr[ItemSelect_MAX];

/*当前选中的选项*/
static int16_t ItemSelect = 0;

/*选项更新标志位*/
static bool ItemSelectUpdating = false;

static bool SaveChanges = false;

/*选项显示起始坐标*/
#define ItemStartY (StatusBar_Height+8)
#define ItemStartX 14


/**
  * @brief  更新选项字符串
  * @param  无
  * @retval 无
  */
static void UpdateItemStr()
{
    for(uint8_t i = 0; i < ItemSelect_MAX; i++)
    {
        if(i == ItemSelect)
            screen.setTextColor(screen.Yellow, screen.Black);
        else
            screen.setTextColor(screen.White, screen.Black);

        screen.setCursor(ItemStartX, ItemStartY + i * (TEXT_HEIGHT_1 + 2));
        screen.print(ItemStr[i]);
    }
}

/**
  * @brief  AT指令延时回调
  * @param  无
  * @retval 无
  */
static void BluetoothDelayCallback()
{
    Thread_StatusBar();
}

/**
  * @brief  页面初始化事件
  * @param  无
  * @retval 无
  */
static void Setup()
{
    SaveChanges = false;
    State_Bluetooth_Last = CTRL.Bluetooth.Enable;
    CTRL.Bluetooth.Enable = OFF;

    screen.setTextColor(screen.White, screen.Black);
    screen.setCursor(20, StatusBar_Height + 10);
    screen.print("Enter AT Mode...");

    hc05.SetDelayCallback(BluetoothDelayCallback);

    if(hc05.AT_Enter())//进入AT模式
    {
        ItemSelect = 0;
        BaudRateSelect = 0;
        ItemSelectUpdating = false;
        screen.setCursor(20, StatusBar_Height + 20);
        screen.setTextColor(screen.Green, screen.Black);
        screen.print("SUCCESS");

        uint32_t baudRate = 0;
        String name, sbaudRate, password;

        if(!hc05.GetName(&name))
            name = "*";

        if(!hc05.GetBaudRate(&baudRate))
            sbaudRate = "*";
        else
            sbaudRate = String(baudRate);

        if(!hc05.GetPassword(&password))
            password = "*";

        ItemStr[0] = "1.Name:" + name;
        ItemStr[1] = "2.BaudRate:" + sbaudRate;
        ItemStr[2] = "3.Password:" + password;
        ItemStr[3] = "4.Role:" + String(hc05.GetRole() ? "Master" : "Slaver");
        ItemStr[4] = "5.Save & Exit";
        Role = hc05.GetRole();
        ClearPage();
        UpdateItemStr();
    }
    else
    {
        screen.setTextColor(screen.Red, screen.Black);
        screen.print("FAILED");
        PageDelay(1000);
        page.PagePop();
    }
}

/**
  * @brief  页面循环事件
  * @param  无
  * @retval 无
  */
static void Loop()
{
    if(ItemSelectUpdating)
    {
        UpdateItemStr();
        ItemSelectUpdating = false;
    }
}

/**
  * @brief  页面退出事件
  * @param  无
  * @retval 无
  */
static void Exit()
{
    ClearPage();
    screen.setTextColor(screen.White, screen.Black);
    screen.setCursor(20, StatusBar_Height + 10);
    screen.print("Exit AT Mode...");
    if(SaveChanges)
    {
        hc05.SetName(_X_CTRL_NAME);
        hc05.SetBaudRate(UseBaudRate[BaudRateSelect]);
        hc05.SetPassword(Password);
        hc05.SetRole(Role);
    }
    hc05.AT_Exit();

    CTRL.Bluetooth.Enable = State_Bluetooth_Last;
}

/**
  * @brief  页面事件
  * @param  无
  * @retval 无
  */
static void Event(int event, void* param)
{
    if(event == EVENT_ButtonPress)
    {
        if(btOK || btEcd)
        {
            switch(ItemSelect)
            {
            case 1:
                BaudRateSelect = (BaudRateSelect + 1) % UseBaudRate_Size;
                ItemStr[1] = "2.BaudRate:" + String(UseBaudRate[BaudRateSelect]) + "  ";
                ItemSelectUpdating = true;
                break;
            case 2:
                static uint8_t i = 0;
                i++;
                i %= 10;
                Password = String(i * 1000 + i * 100 + i * 10 + i);
                ItemStr[2] = "3.Password:" + Password;
                ItemSelectUpdating = true;
                break;
            case 3:
                Role = ! Role;
                ItemStr[3] = "4.Role:" + String(Role ? "Master" : "Slaver");
                ItemSelectUpdating = true;
                break;
            case 4:
                SaveChanges = true;
                page.PagePop();
                break;
            }
        }
        if(btBACK)
        {
            page.PagePop();
        }

        if(btUP)
        {
            ItemSelect = (ItemSelect + 1) % ItemSelect_MAX;
            ItemSelectUpdating = true;
        }
        if(btDOWN)
        {
            ItemSelect = (ItemSelect + ItemSelect_MAX - 1) % ItemSelect_MAX;
            ItemSelectUpdating = true;
        }
    }
}

/**
  * @brief  蓝牙设置页面注册
  * @param  pageID:为此页面分配的ID号
  * @retval 无
  */
void PageRegister_SetBluetooth(uint8_t pageID)
{
    page.PageRegister(pageID, Setup, Loop, Exit, Event);
}
