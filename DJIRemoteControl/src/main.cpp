#include <Arduino.h>
#include <Keypad.h>
#include <IRremote.h>

//按键和执行命令的定义，从之前项目拷贝完成，根据实际需要可单独定义为头文件



uint16_t sAddress = 0x0; //红外线接收标识的地址！！！为防止恶意干扰，实际应用场景应重新定义此值，并在
//定义遥控器按键
const unsigned long K_CHDOWN = 0x45; //频道+
const unsigned long K_CH = 0x46;     //频道
const unsigned long K_CHUP = 0x47;   //频道+

const unsigned long K_PRE = 0x44;        //上一曲
const unsigned long K_NEXT = 0x40;       //下一曲
const unsigned long K_PLAYNPAUSE = 0x43; //播放暂停

const unsigned long K_VOLDOWN = 0x07; //音量减
const unsigned long K_VOLUP = 0x15;   //音量加
const unsigned long K_EQ = 0x09;      //EQ

const unsigned long K_0 = 0x16;   //0
const unsigned long K_100 = 0x19; //100+
const unsigned long K_200 = 0x0D; //200+

const unsigned long K_1 = 0x0C;
const unsigned long K_2 = 0x18;
const unsigned long K_3 = 0x5E;

const unsigned long K_4 = 0x08;
const unsigned long K_5 = 0x1C;
const unsigned long K_6 = 0x5A;

const unsigned long K_7 = 0x42;
const unsigned long K_8 = 0x52;
const unsigned long K_9 = 0x4A;

//定义键位
const unsigned long TTCMD_TAKEOFF = K_7; //起飞
const unsigned long TTCMD_LAND = K_9;    //降落

const unsigned long TTCMD_UP = K_CHUP;     //上升
const unsigned long TTCMD_DOWN = K_CHDOWN; //下降

const unsigned long TTCMD_LEFT = K_PRE;         //向左飞
const unsigned long TTCMD_RIGHT = K_PLAYNPAUSE; //向右飞

const unsigned long TTCMD_FORWARD = K_CH; //向左飞
const unsigned long TTCMD_BACK = K_NEXT;  //向右飞

const unsigned long TTCMD_CCW = K_VOLDOWN; //逆时针旋转
const unsigned long TTCMD_CW = K_EQ;       //顺时针旋转

const unsigned long TTCMD_FLIP_F = K_2; //向前翻转
const unsigned long TTCMD_FLIP_B = K_5; //向后翻转
const unsigned long TTCMD_FLIP_L = K_4; //向左翻转
const unsigned long TTCMD_FLIP_R = K_6; //向右翻转

uint8_t SEND_PIN = 13;    //发射红外线的引脚

uint8_t sRepeats = 1;    //重复发送命令的次数

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {8, 7, 6, 5}; //定义行接口的引脚connect to the row pinouts of the kpd
byte colPins[COLS] = {12, 11, 10, 9};   //定义列接口的引脚connect to the column pinouts of the kpd

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String msg;

void setup()
{
    Serial.begin(9600);
    IrSender.begin(SEND_PIN, true); //初始化红外线发射
    Serial.println("Ready to send...");
    //msg = "";
}

void loop()
{
    KeyState K_A_State = IDLE; //   注意！！！！！！！！！！！！！
    KeyState K_B_State = IDLE; //   此处定义为A、B、C、D键的状态，
    KeyState K_C_State = IDLE; //   A、B、C、D键用于组合其他按键协同工作，
    KeyState K_D_State = IDLE; //   以完成更多的指令. 

    if (kpd.getKeys()) //扫描所有按键状态
    {
        for (int i = 0; i < LIST_MAX; i++) //扫描按键列表
        {
            if (kpd.key[i].stateChanged) //如果按键状态发生了改变
            {
                switch (kpd.key[i].kstate)
                { //通过COM口反馈按键状态
                case PRESSED:
                    msg = " PRESSED."; //按下
                    break;
                case HOLD:
                    msg = " HOLD."; //长按
                    break;
                case RELEASED:
                    msg = " RELEASED."; //释放
                    break;
                case IDLE:
                    msg = " IDLE."; //空闲
                }
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.println(msg);

                if ((kpd.key[i].kchar == '*') && (kpd.key[i].kstate == PRESSED)) //起飞
                {
                    IrSender.sendNEC(sAddress, TTCMD_TAKEOFF, sRepeats);
                    delay(100); //防止连续发送导致解码错误，delay（100）
                }
                else if ((kpd.key[i].kchar == '#') && (kpd.key[i].kstate == PRESSED)) //降落
                {
                    IrSender.sendNEC(sAddress, TTCMD_LAND, sRepeats);
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '2') && (kpd.key[i].kstate == PRESSED)) //A没被长安则前进，否则向前翻滚
                {
                    if (K_A_State != HOLD)
                        IrSender.sendNEC(sAddress, TTCMD_FORWARD, sRepeats); //A没被长按，前进
                    else
                        IrSender.sendNEC(sAddress, TTCMD_FLIP_F, sRepeats); //A被长按，向前翻滚

                    delay(100);
                }
                else if ((kpd.key[i].kchar == '5') && (kpd.key[i].kstate == PRESSED)) //后退
                {
                    if (K_A_State != HOLD)
                        IrSender.sendNEC(sAddress, TTCMD_BACK, sRepeats); //A没被长按，后退
                    else
                        IrSender.sendNEC(sAddress, TTCMD_FLIP_B, sRepeats); //A没被长按，向后翻滚
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '4') && (kpd.key[i].kstate == PRESSED)) //左移
                {
                    if (K_A_State != HOLD)
                        IrSender.sendNEC(sAddress, TTCMD_LEFT, sRepeats); //如果A没被长按，左移
                    else
                        IrSender.sendNEC(sAddress, TTCMD_FLIP_L, sRepeats); //如果A被长按，向左翻滚
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '6') && (kpd.key[i].kstate == PRESSED)) //右移
                {
                    if (K_A_State != HOLD)
                        IrSender.sendNEC(sAddress, TTCMD_RIGHT, sRepeats); //如果A没被长按，右移
                    else
                        IrSender.sendNEC(sAddress, TTCMD_FLIP_R, sRepeats); //如果A被长按，右翻滚
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '1') && (kpd.key[i].kstate == PRESSED)) //逆时针旋转
                {
                    IrSender.sendNEC(sAddress, TTCMD_CCW, sRepeats);
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '3') && (kpd.key[i].kstate == PRESSED)) //顺时针旋转
                {
                    IrSender.sendNEC(sAddress, TTCMD_CW, sRepeats);
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '3') && (kpd.key[i].kstate == PRESSED)) //顺时针旋转
                {
                    IrSender.sendNEC(sAddress, TTCMD_CW, sRepeats);
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '8') && (kpd.key[i].kstate == PRESSED)) //上升高度
                {
                    IrSender.sendNEC(sAddress, TTCMD_UP, sRepeats);
                    delay(100);
                }
                else if ((kpd.key[i].kchar == '0') && (kpd.key[i].kstate == PRESSED)) //上升高度
                {
                    IrSender.sendNEC(sAddress, TTCMD_DOWN, sRepeats);
                    delay(100);
                }
                //更新最后一次按键A的状态
            }
            if ((kpd.key[i].kchar == 'A')) //更新最后一次按键A的状态
            {
                K_A_State = kpd.key[i].kstate;
            }
            else if ((kpd.key[i].kchar == 'B')) //更新最后一次按键B的状态
            {
                K_B_State = kpd.key[i].kstate;
            }
            else if ((kpd.key[i].kchar == 'C')) //更新最后一次按键C的状态
            {
                K_C_State = kpd.key[i].kstate;
            }
            else if ((kpd.key[i].kchar == 'D')) //更新最后一次按键D的状态
            {
                K_D_State = kpd.key[i].kstate;
            }
        }
    }
} // End loop