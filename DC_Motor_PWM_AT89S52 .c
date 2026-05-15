#include <reg51.h>

/*==================================================
      DC MOTOR SPEED CONTROL USING PWM
               AT89S52 MICROCONTROLLER
====================================================*/

/*--------------- MOTOR CONTROL PINS ---------------*/
sbit ENA = P1^0;   // Enable pin for PWM output
sbit IN1 = P1^1;   // Motor direction pin 1
sbit IN2 = P1^2;   // Motor direction pin 2

/*---------------- SWITCH INPUT --------------------*/
sbit SW  = P3^0;   // Push button for speed control

/*---------------- I2C LCD PINS --------------------*/
sbit SDA = P2^0;   // I2C Data line
sbit SCL = P2^1;   // I2C Clock line

#define LCD_ADDR 0x4E   // I2C LCD address

/*--------------- GLOBAL VARIABLES -----------------*/
unsigned char duty = 0;     // PWM duty cycle
unsigned char count = 0;    // PWM counter
unsigned char mode  = 0;    // Speed mode selector

/*==================================================
                    DELAY FUNCTION
====================================================*/
void delay_ms(unsigned int d)
{
    unsigned int i, j;

    for(i = 0; i < d; i++)
        for(j = 0; j < 1275; j++);
}

/*==================================================
                PWM USING TIMER0 ISR
====================================================*/
void timer0_ISR(void) interrupt 1
{
    count++;

    if(count < duty)
        ENA = 1;
    else
        ENA = 0;

    if(count >= 100)
        count = 0;
}

/*==================================================
                 TIMER0 INITIALIZATION
====================================================*/
void timer0_init()
{
    TMOD = 0x02;
    TH0  = 200;

    IE   = 0x82;
    TR0  = 1;
}

/*==================================================
                    I2C FUNCTIONS
====================================================*/
void I2C_Start()
{
    SDA = 1;
    SCL = 1;

    SDA = 0;
    SCL = 0;
}

void I2C_Stop()
{
    SDA = 0;
    SCL = 1;
    SDA = 1;
}

void I2C_Write(unsigned char dat)
{
    unsigned char i;

    for(i = 0; i < 8; i++)
    {
        SDA = (dat & 0x80);

        SCL = 1;
        SCL = 0;

        dat <<= 1;
    }

    SDA = 1;

    SCL = 1;
    SCL = 0;
}

/*==================================================
                  LCD LOW LEVEL DRIVER
====================================================*/
void LCD_Send(unsigned char dat)
{
    I2C_Start();

    I2C_Write(LCD_ADDR);

    I2C_Write(dat | 0x0C);
    delay_ms(2);

    I2C_Write(dat | 0x08);
    delay_ms(2);

    I2C_Stop();
}

/*==================================================
              LCD COMMAND & DATA FUNCTIONS
====================================================*/
void LCD_Command(unsigned char cmd)
{
    LCD_Send(cmd & 0xF0);
    LCD_Send((cmd << 4) & 0xF0);
}

void LCD_Data(unsigned char dat)
{
    LCD_Send((dat & 0xF0) | 0x09);
    LCD_Send(((dat << 4) & 0xF0) | 0x09);
}

/*==================================================
                  LCD INITIALIZATION
====================================================*/
void LCD_Init()
{
    delay_ms(50);

    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);

    delay_ms(10);
}

/*==================================================
                DISPLAY STRING ON LCD
====================================================*/
void LCD_String(char *str)
{
    while(*str)
        LCD_Data(*str++);
}

/*==================================================
                DISPLAY MOTOR SPEED
====================================================*/
void LCD_Speed(unsigned char val)
{
    LCD_Command(0xC0);

    if(val == 0)
    {
        LCD_String("OFF   ");
        return;
    }

    if(val == 100)
    {
        LCD_String("100%");
        return;
    }

    if(val < 10)
        LCD_Data('0');

    LCD_Data((val / 10) + '0');
    LCD_Data((val % 10) + '0');
    LCD_Data('%');
}

/*==================================================
                     MAIN PROGRAM
====================================================*/
void main()
{
    IN1 = 1;
    IN2 = 0;

    timer0_init();
    LCD_Init();

    LCD_Command(0x80);
    LCD_String("Motor Speed:");

    while(1)
    {
        LCD_Speed(duty);

        if(SW == 0)
        {
            delay_ms(200);

            mode++;

            if(mode > 5)
                mode = 0;

            switch(mode)
            {
                case 0: duty = 0;   break;
                case 1: duty = 20;  break;
                case 2: duty = 40;  break;
                case 3: duty = 60;  break;
                case 4: duty = 80;  break;
                case 5: duty = 100; break;
            }

            while(SW == 0);
        }
    }
}
