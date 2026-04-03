#include "EMMC42V5.h"
#include <stdlib.h>

// ────────────────────────────────────────────
//  内部：发送帧（先清 RX，再发）
// ────────────────────────────────────────────
static void emm_send(EMMC42V5_t *m, uint8_t *data, uint16_t len)
{
    uint8_t dummy;
    while (HAL_UART_Receive(m->huart, &dummy, 1, 0) == HAL_OK) {}
    HAL_UART_Transmit(m->huart, data, len, 100);
    HAL_Delay(1);
}

// ────────────────────────────────────────────
//  内部：接收帧（等首字节匹配 id）
// ────────────────────────────────────────────
static void emm_read(EMMC42V5_t *m, uint8_t *buf, uint8_t len)
{
    HAL_Delay(1);
    for (int i = 0; i < EMM_MAX_RETRY; i++) {
        if (HAL_UART_Receive(m->huart, &buf[0], 1, 2) == HAL_OK) {
            if (buf[0] == m->id || buf[0] == 0x00) {
                for (int j = 1; j < len; j++) {
                    HAL_UART_Receive(m->huart, &buf[j], 1, 10);
                }
                return;
            }
        }
        HAL_Delay(1);
    }
}

// ────────────────────────────────────────────
//  解析通用4字节应答
// ────────────────────────────────────────────
static int emm_parse_resp(uint8_t *r)
{
    switch (r[2]) {
        case 0x02: return EMM_COMMAND_SUCCESS;
        case 0xE2: return EMM_COMMAND_FAIL;
        case 0xEE: return EMM_COMMAND_ERROR;
        default:   return EMM_NOT_RESPONSE;
    }
}

// ────────────────────────────────────────────
//  初始化
// ────────────────────────────────────────────
void EMMC42V5_Init(EMMC42V5_t *m, UART_HandleTypeDef *huart, uint8_t id)
{
    m->huart = huart;
    m->id    = id;
    m->dir   = 0;
}

// ────────────────────────────────────────────
//  使能控制
//  地址 + 0xF3 + 0xAB + 使能 + 0x00 + 0x6B
// ────────────────────────────────────────────
int EMMC42V5_Enable(EMMC42V5_t *m, uint8_t is_enable)
{
    uint8_t data[6] = {m->id, 0xF3, 0xAB, is_enable, 0x00, EMM_CHECKSUM};
    emm_send(m, data, 6);
    uint8_t r[4] = {0};
    emm_read(m, r, 4);
    return emm_parse_resp(r);
}

// ────────────────────────────────────────────
//  角度清零
//  地址 + 0x0A + 0x6D + 0x6B
// ────────────────────────────────────────────
int EMMC42V5_AngleReset(EMMC42V5_t *m)
{
    uint8_t data[4] = {m->id, 0x0A, 0x6D, EMM_CHECKSUM};
    emm_send(m, data, 4);
    uint8_t r[4] = {0};
    emm_read(m, r, 4);
    return emm_parse_resp(r);
}

// ────────────────────────────────────────────
//  立即停止
//  地址 + 0xFE + 0x98 + needsync + 0x6B
// ────────────────────────────────────────────
int EMMC42V5_Stop(EMMC42V5_t *m, uint8_t needsync)
{
    uint8_t data[5] = {m->id, 0xFE, 0x98, needsync, EMM_CHECKSUM};
    emm_send(m, data, 5);
    uint8_t r[4] = {0};
    emm_read(m, r, 4);
    return emm_parse_resp(r);
}

// ────────────────────────────────────────────
//  多机同步
//  0x00 + 0xFF + 0x66 + 0x6B
// ────────────────────────────────────────────
int EMMC42V5_Sync(EMMC42V5_t *m)
{
    uint8_t data[4] = {0x00, 0xFF, 0x66, EMM_CHECKSUM};
    emm_send(m, data, 4);
    uint8_t r[4] = {0};
    emm_read(m, r, 4);
    return emm_parse_resp(r);
}

// ────────────────────────────────────────────
//  位置模式（脉冲数）
//  地址 + 0xFD + dir + speed(2) + acce + pulse(4) + abs + sync + 0x6B
// ────────────────────────────────────────────
int EMMC42V5_PulseControl(EMMC42V5_t *m, int64_t pulse_num, int16_t speed,
                           uint8_t acce, uint8_t absolute_mode, uint8_t needsync)
{
    uint8_t direction = (pulse_num < 0) ? 1 : 0;
    if (m->dir) direction = !direction;
    uint64_t p = (uint64_t)llabs(pulse_num);

    uint8_t data[13] = {
        m->id, 0xFD,
        direction,
        (uint8_t)(speed >> 8), (uint8_t)(speed & 0xFF),
        acce,
        (uint8_t)(p >> 24), (uint8_t)(p >> 16), (uint8_t)(p >> 8), (uint8_t)(p),
        absolute_mode, needsync,
        EMM_CHECKSUM
    };
    emm_send(m, data, 13);
    uint8_t r[4] = {0};
    emm_read(m, r, 4);
    return emm_parse_resp(r);
}

// ────────────────────────────────────────────
//  读取输入脉冲数
//  地址 + 0x32 + 0x6B
//  返回：地址 + 0x32 + 符号 + 脉冲数(4字节) + 0x6B
// ────────────────────────────────────────────
int64_t EMMC42V5_ReadInputPulses(EMMC42V5_t *m)
{
    uint8_t data[3] = {m->id, 0x32, EMM_CHECKSUM};
    emm_send(m, data, 3);
    uint8_t r[8] = {0};
    emm_read(m, r, 8);
    if (r[1] != 0x32) return 0;
    int64_t val = ((int64_t)r[3] << 24) | ((int64_t)r[4] << 16)
                | ((int64_t)r[5] <<  8) |  (int64_t)r[6];
    if (r[2] == 0x01) val = -val;
    return val;
}
