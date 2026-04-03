#include "EMMC42V5.hpp"
#include "Chassis.hpp"
#include "uart.hpp"

//串口
#define RX1 18
#define TX1 19

#define RX2 17
#define TX2 16
uint8_t buffer[10];
void setup() {
  // put your setup code here, to run once:
  motor_ser.begin(115200, SERIAL_8N1, RX1, TX1);
  Serial2.begin(115200, SERIAL_8N1, RX2, TX2);  
  Serial.begin(115200);
  delay(1000);

  // Uart_Scan_QR();
   Chassis_Enable(true);
  // motor_back_left.enable(1);
  // motor_back_left.angle_reset();
  // motor_back_left.pulse_control(3200,250);


  delay(2000);  
  Chassis_Incremental_Position(300,0,0);
  // delay(1000);
  // Chassis_Incremental_Position(300,0,0);  
  // delay(1000);
  // Chassis_Incremental_Position(300,0,0);  
  // delay(1000);  
  // Chassis_Incremental_Position(0,84,0);
  // delay(1000);  
  // Chassis_Incremental_Position(-300,0,0);  
  // delay(1000);  
  // Chassis_Incremental_Position(300,0,0);  

}

void loop() {
  // 检查串口是否有数据

  // if (Serial2.available() > 0) {
  //   // 读取一个字节的数据
  //   char incomingByte = Serial2.read();
  //   // 打印接收到的数据
  //   Serial.print("Received: ");
  //   Serial.println(incomingByte);

  // }
  // put your main code here, to run repeatedly:

}
