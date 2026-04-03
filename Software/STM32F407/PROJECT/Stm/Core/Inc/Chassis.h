#ifndef CHASSIS_H
#define CHASSIS_H

#include "EMMC42V5.h"

// 修改为 CubeMX 中配置的电机串口句柄
#define MOTOR_UART  huart1

void Chassis_Enable              (uint8_t enable);
void Chassis_Incremental_Position(float position_x, float position_y, float position_z);

#endif
