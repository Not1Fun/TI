#ifndef EMMC42V5_H
#define EMMC42V5_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define EMM_MAX_RETRY       10
#define EMM_COMMAND_SUCCESS 1
#define EMM_COMMAND_FAIL    0
#define EMM_COMMAND_ERROR   2
#define EMM_NOT_RESPONSE    3
#define EMM_CHECKSUM        0x6B

// 电机句柄
typedef struct {
    UART_HandleTypeDef *huart;
    uint8_t             id;
    uint8_t             dir;   // 反向标志：1=反转
} EMMC42V5_t;

void EMMC42V5_Init        (EMMC42V5_t *m, UART_HandleTypeDef *huart, uint8_t id);

int  EMMC42V5_Enable      (EMMC42V5_t *m, uint8_t is_enable);
int  EMMC42V5_AngleReset  (EMMC42V5_t *m);
int  EMMC42V5_Stop        (EMMC42V5_t *m, uint8_t needsync);
int  EMMC42V5_Sync        (EMMC42V5_t *m);
int  EMMC42V5_PulseControl(EMMC42V5_t *m, int64_t pulse_num, int16_t speed,
                            uint8_t acce, uint8_t absolute_mode, uint8_t needsync);
int64_t EMMC42V5_ReadInputPulses(EMMC42V5_t *m);

#endif
