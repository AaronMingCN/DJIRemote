#include <Arduino.h>
#include <IRremote.h>

//#include <RMTT_Effect.h>
//#include <RMTT_Fonts.h>
#include <RMTT_Libs.h>
//#include <RMTT_Matrix.h>
#include <RMTT_Protocol.h>
#include <RMTT_RGB.h>
//#include <RMTT_Shell.h>
//#include <RMTT_TOF.h>

#define TTSpeed (100)       //预定义速度
#define TTMoveDistance (50) //移动的距离
#define TTRotateAngle (45)  //定义旋转角度

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

//定义TT状态
enum TT_STAT
{
    TT_INIT,
    TT_WAITINGCMD, //等待命令
    TT_PROCCMD,    //正在处理命令
    TT_PROCERR     //处理命令错误
};

//定义红外线接收器
int Recv_Pin = 13; //红外接收的引脚

int TT_Battery = 0;    //定义TT的电量
int BLastReadTime = 0; //最后一次读取的时间

bool inited = false; //是否初始化完成

//定义TT通信模块
RMTT_Protocol protocol;
//定义TT的LED灯
RMTT_RGB tt_rgb;
//定义TT当前状态

TT_STAT TS = TT_INIT;
//修改该当前状态
void ChangeTTStat(TT_STAT AStat)
{
    if (AStat != TS) //如果状态改变
    {
        switch (AStat)
        {
        case TT_WAITINGCMD:
            if (TT_Battery > 50)
                tt_rgb.SetRGB(0, 255, 0); //绿灯为等待状态
            else
                tt_rgb.SetRGB(255, 0, 255); //电量不足灯为等待状态紫色
            break;
        case TT_PROCCMD:
            tt_rgb.SetRGB(255, 255, 0); //黄灯执行状态
            break;
        case TT_PROCERR:
            tt_rgb.SetRGB(255, 0, 0); //红灯错误
        default:
            break;
        }
        TS = AStat;
    }
}

//等待命令执行结果s
bool WaitCMDRes(uint32_t timeout)
{
    bool result = true;
    String Res;
    ChangeTTStat(TT_PROCCMD);
    Res = protocol.getTelloResponseString(timeout); //读取命令返回,5秒超时

    if (Res != String("ETT ok")) //如果没有收到执行成功的返回指令，闪一下红灯
    {
        result = false;
        ChangeTTStat(TT_PROCERR);
        delay(100);
    }

    ChangeTTStat(TT_WAITINGCMD);
    return result;
}

void WaitTTInit() //等待设备初始化，连接到无人机
{
    ChangeTTStat(TT_PROCCMD);
    while (protocol.getTelloMsgString("[TELLO] command", 20000) != String("ETT ok"))
    {
    }
    protocol.getTelloResponseString(1000);
    ChangeTTStat(TT_WAITINGCMD);
}

//处理一个命令
void TTProcCMD(unsigned long ACMD)
{
    if (TS == TT_WAITINGCMD)
        switch (ACMD)
        {
        case TTCMD_TAKEOFF:
            protocol.TakeOff();
            WaitCMDRes(10000);
            break;
        case TTCMD_LAND:
            protocol.Land();
            WaitCMDRes(10000);
            break;
        case TTCMD_UP:
            protocol.Up(TTMoveDistance);
            WaitCMDRes(5000);
            break;
        case TTCMD_DOWN:
            protocol.Down(TTMoveDistance);
            WaitCMDRes(5000);
            break;
        case TTCMD_LEFT:
            protocol.Left(TTMoveDistance);
            WaitCMDRes(5000);
            break;
        case TTCMD_RIGHT:
            protocol.Right(TTMoveDistance);
            WaitCMDRes(5000);
            break;
        case TTCMD_FORWARD:
            protocol.Forward(TTMoveDistance);
            WaitCMDRes(5000);
            break;
        case TTCMD_BACK:
            protocol.Back(TTMoveDistance);
            WaitCMDRes(5000);
            break;
        case TTCMD_CCW:
            protocol.CCW(TTRotateAngle);
            WaitCMDRes(5000);
            break;
        case TTCMD_CW:
            protocol.CW(TTRotateAngle);
            WaitCMDRes(5000);
            break;
        case TTCMD_FLIP_F:
            protocol.Flip('f');
            WaitCMDRes(5000);
            break;
        case TTCMD_FLIP_B:
            protocol.Flip('b');
            WaitCMDRes(5000);
            break;
        case TTCMD_FLIP_L:
            protocol.Flip('l');
            WaitCMDRes(5000);
            break;
        case TTCMD_FLIP_R:
            protocol.Flip('r');
            WaitCMDRes(5000);
            break;
        default:
            break;
        }
}

void TTReadBattery()
{
    int BatV;
    unsigned long mis = millis();
    if ((mis - BLastReadTime) > 13000) //执行时间间隔
    {
        if (TS == TT_WAITINGCMD)
        {
            ChangeTTStat(TT_PROCCMD);
            protocol.ReadBattery();
            BatV = protocol.getTelloResponseInt(5000);
            protocol.Stop();                       //发送悬停命令，防止15秒超时降落
            protocol.getTelloResponseString(2000); // 等待命令执行，超时2秒
            if (BatV != 0)
            {
                TT_Battery = BatV;
                BLastReadTime = mis;
            }
            ChangeTTStat(TT_WAITINGCMD);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    //初始化红外线接收器
    pinMode(Recv_Pin, INPUT);
    IrReceiver.begin(Recv_Pin, ENABLE_LED_FEEDBACK);
    //初始化LED
    tt_rgb.Init();
    led_effect_init();

    //初始化和无人机的通信
    Serial1.begin(1000000, SERIAL_8N1, 23, 18);
    WaitTTInit(); //等待TT初始化完成
    //设置初始速度
    protocol.SetSpeed(TTSpeed); //初始速度
    inited = true;
}

unsigned long IRCMD = 0; //定义红外线接收的命令变量
void loop()
{
    if (inited) //是否已经初始化完成
    {
        //TTReadBattery();
        //读取红外线接收数据
        if (IrReceiver.decode())
        {
            IRCMD = IrReceiver.decodedIRData.command;
            IrReceiver.disableIRIn();
            TTProcCMD(IRCMD);
            IrReceiver.enableIRIn();
            IrReceiver.resume();
        }
    }
}
