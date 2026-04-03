#include "Chassis.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include <stdlib.h>

#define CHASSIS_LONG    75
#define CHASSIS_WHITE   102
#define CHASSIS_PI      3.14159265f
#define WHEEL_RADIUS    37.5f

extern UART_HandleTypeDef MOTOR_UART;

static EMMC42V5_t motor_fl;   // 左前
static EMMC42V5_t motor_fr;   // 右前
static EMMC42V5_t motor_bl;   // 左后
static EMMC42V5_t motor_br;   // 右后

static uint8_t chassis_inited = 0;

static void chassis_init(void)
{
    if (chassis_inited) return;
    EMMC42V5_Init(&motor_fl, &MOTOR_UART, 1);
    EMMC42V5_Init(&motor_fr, &MOTOR_UART, 2);
    EMMC42V5_Init(&motor_bl, &MOTOR_UART, 3);
    EMMC42V5_Init(&motor_br, &MOTOR_UART, 4);
    chassis_inited = 1;
}

static float fand_max(float a, float b, float c, float d)
{
    a = fabsf(a); b = fabsf(b); c = fabsf(c); d = fabsf(d);
    float m1 = a > b ? a : b;
    float m2 = c > d ? c : d;
    return m1 > m2 ? m1 : m2;
}

// ────────────────────────────────────────────
//  底盘使能 + 角度清零
// ────────────────────────────────────────────
void Chassis_Enable(uint8_t enable)
{
    chassis_init();
    EMMC42V5_Enable(&motor_fl, enable);
    EMMC42V5_Enable(&motor_fr, enable);
    EMMC42V5_Enable(&motor_bl, enable);
    EMMC42V5_Enable(&motor_br, enable);

    EMMC42V5_AngleReset(&motor_fl);
    EMMC42V5_AngleReset(&motor_fr);
    EMMC42V5_AngleReset(&motor_bl);
    EMMC42V5_AngleReset(&motor_br);
}

// ────────────────────────────────────────────
//  增量位置控制
//  position_x / position_y：平移（mm）
//  position_z：旋转（度）
// ────────────────────────────────────────────
void Chassis_Incremental_Position(float position_x, float position_y, float position_z)
{
    chassis_init();
    Chassis_Enable(1);

    int16_t speed = 250;
    uint8_t acc   = 100;

    float pulse_x = position_x * 3200.0f / (CHASSIS_PI * WHEEL_RADIUS * 2.0f);
    float pulse_y = -(position_y * 3200.0f / (CHASSIS_PI * WHEEL_RADIUS * 2.0f));
    float pulse_z = -((3200.0f * position_z
                       * sqrtf((float)(CHASSIS_LONG * CHASSIS_LONG + CHASSIS_WHITE * CHASSIS_WHITE))
                       * sqrtf(2.0f))
                      / (360.0f * WHEEL_RADIUS * 2.0f));

    float fl = -(-pulse_y - pulse_x - pulse_z);
    float fr = -(-pulse_y + pulse_x - pulse_z);
    float bl =  (-pulse_y + pulse_x + pulse_z);
    float br =  (-pulse_y - pulse_x + pulse_z);

    EMMC42V5_PulseControl(&motor_fl, (int64_t)fl, speed, acc, 0, 1);
    EMMC42V5_PulseControl(&motor_fr, (int64_t)fr, speed, acc, 0, 1);
    EMMC42V5_PulseControl(&motor_bl, (int64_t)bl, speed, acc, 0, 1);
    EMMC42V5_PulseControl(&motor_br, (int64_t)br, speed, acc, 0, 1);
    EMMC42V5_Sync(&motor_fl);

    float max_pulse = fand_max(fl, fr, bl, br);
    uint32_t wait_ms = (uint32_t)(fabsf(max_pulse) / 3200.0f / speed * 60.0f * 1000.0f
                                  + (256 - acc) * 0.05f * speed);
    HAL_Delay(wait_ms);
}
