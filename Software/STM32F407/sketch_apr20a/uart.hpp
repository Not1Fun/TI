#ifndef UART_HPP_
#define UART_HPP_
#include<math.h>

#define HeaderShoot1 0x0A
#define ReceiveShoot 0x0B

    /**
     * @brief 发送通讯传输函数
     *
     * @param data:用于向tx发送的消息数组
     * @param length:向tx发送的消息数组的长度
     * @param need_delay:是否需要延时
     *
     * @return None
     */
    void Serial_Send(uint8_t *data, int length)
    {
        for (int i = 0; i < length; i++)
        {
            Serial.write(data[i]);
        }
        delay(5);
    }

uint8_t Buffer[10];
int a=13;
void Uart_Scan_QR()
{
   Serial.write(a);
   for(int i=0;i<2000;i++)
    {
        if (Serial2.available() > 0) {
    // 读取一个字节的数据
        char incomingByte = Serial2.read();
    // 打印接收到的数据
        Serial.println(incomingByte);
        break;
    }

  }



//    Buffer[0]=Serial.read();
//    Serial.println(Buffer[0]);
//    while(Buffer[0]!=ReceiveShoot)
//    {
//       delay(10);
//       Serial.write(HeaderShoot1);
//       Buffer[0]=Serial.read();
//    }

//    Buffer[1]=Serial.read();   
//    Serial.println(Buffer[1]);
}





#endif