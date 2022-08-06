#ifndef DHT11_SENSOR
#define DHT11_SENSOR

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define DHT11_PIN GPIO_NUM_33 //可通过宏定义，修改引脚

#define DHT11_CLR gpio_set_level(DHT11_PIN, 0)
#define DHT11_SET gpio_set_level(DHT11_PIN, 1)
#define DHT11_IN gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT)
#define DHT11_OUT gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT)

uint8_t DHT11Data[4] = {0};
uint8_t Temp, Humi;

// us延时函数，误差不能太大
void DHT11DelayUs(uint32_t nCount)
{
    ets_delay_us(nCount);
}

void DHT11_init()
{
    gpio_pad_select_gpio(DHT11_PIN);
}

void DHT11_Start(void)
{
    DHT11_OUT;               //设置端口方向
    DHT11_CLR;               //拉低端口
    DHT11DelayUs(19 * 1000); //持续最低18ms;

    DHT11_SET;        //释放总线
    DHT11DelayUs(30); //总线由上拉电阻拉高，主机延时30uS;
    DHT11_IN;         //设置端口方向

    while (!gpio_get_level(DHT11_PIN))
        ; // DHT11等待80us低电平响应信号结束

    while (gpio_get_level(DHT11_PIN))
        ; // DHT11   将总线拉高80us
}

uint8_t DHT11_ReadValue(void)
{
    uint8_t i, sbuf = 0;
    for (i = 8; i > 0; i--)
    {
        sbuf <<= 1;
        while (!gpio_get_level(DHT11_PIN))
            ;
        DHT11DelayUs(30); // 延时 30us 后检测数据线是否还是高电平
        if (gpio_get_level(DHT11_PIN))
        {
            sbuf |= 1;
        }
        else
        {
            sbuf |= 0;
        }
        while (gpio_get_level(DHT11_PIN))
            ;
    }
    
    return sbuf;
}

uint8_t DHT11_ReadTemHum(uint8_t *buf)
{
    uint8_t check;

    buf[0] = DHT11_ReadValue();
    buf[1] = DHT11_ReadValue();
    buf[2] = DHT11_ReadValue();
    buf[3] = DHT11_ReadValue();

    check = DHT11_ReadValue();

    if (check == buf[0] + buf[1] + buf[2] + buf[3])
        return 1;
    else
        return 0;
}

#endif