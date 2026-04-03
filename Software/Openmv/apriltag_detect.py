import sensor, image, time
from machine import UART

# ────────────────────────────────────────────
#  串口初始化（UART3，TX=P4，RX=P5）
# ────────────────────────────────────────────
uart = UART(3, 115200)

# ────────────────────────────────────────────
#  摄像头初始化
# ────────────────────────────────────────────
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)      # 320×240，帧率快
sensor.set_auto_gain(True)             # 自动增益，应对光照变化
sensor.set_auto_whitebal(True)         # 自动白平衡
sensor.set_auto_exposure(True)
sensor.skip_frames(time=1000)          # 等待自动曝光稳定

# ────────────────────────────────────────────
#  常量定义
# ────────────────────────────────────────────
FRAME_HEADER = 0xAA
FRAME_TAIL   = 0xBB
TYPE_CARGO   = 0x01    # 货物识别帧
TYPE_NOCARGO  = 0x00   # 未检测到货物

# Tag ID → 货物名称映射
CARGO_NAME = {0: 'A', 1: 'B', 2: 'C'}

# 置信度阈值，过滤低质量识别结果
DECISION_MARGIN_MIN = 5   # 根据实际 margin 打印值调整，正式打印纸质Tag后可适当提高

# 连续确认帧数（同一ID连续出现N帧才上报，防抖）
CONFIRM_FRAMES = 3

# ────────────────────────────────────────────
#  发送函数
#  协议：AA 01 id cx_H cx_L cy_H cy_L BB
# ────────────────────────────────────────────
def send_cargo(tag_id, cx, cy):
    buf = bytes([
        FRAME_HEADER,
        TYPE_CARGO,
        tag_id & 0xFF,
        (cx >> 8) & 0xFF,
        cx & 0xFF,
        (cy >> 8) & 0xFF,
        cy & 0xFF,
        FRAME_TAIL
    ])
    uart.write(buf)

def send_no_cargo():
    buf = bytes([FRAME_HEADER, TYPE_NOCARGO, 0xFF, 0, 0, 0, 0, FRAME_TAIL])
    uart.write(buf)

# ────────────────────────────────────────────
#  主循环
# ────────────────────────────────────────────
confirm_count  = {}   # {tag_id: 连续出现帧数}
last_sent_id   = -1   # 上一次上报的ID，避免重复上报同一货物

while True:
    img = sensor.snapshot()

    tags = img.find_apriltags(families=16)  # TAG36H11=16，v4.8.x 无 image.TAG36H11 常量

    # 过滤低置信度（调试时先打印 margin 实际值，根据结果调整 DECISION_MARGIN_MIN）
    if tags:
        print("raw tags:", [(t.id, round(t.decision_margin, 1)) for t in tags])
    valid_tags = [t for t in tags if t.decision_margin > DECISION_MARGIN_MIN]

    if valid_tags:
        # 若多个Tag同帧出现，取置信度最高的
        best = max(valid_tags, key=lambda t: t.decision_margin)
        tid  = best.id
        cx   = int(best.cx)
        cy   = int(best.cy)

        # 画框调试（烧录正式版可注释掉）
        img.draw_rectangle(best.rect(), color=(0, 255, 0))
        img.draw_string(cx - 10, cy - 10,
                        "{}({})".format(CARGO_NAME.get(tid, '?'), tid),
                        color=(255, 0, 0), scale=1.5)

        # 连续确认防抖
        confirm_count[tid] = confirm_count.get(tid, 0) + 1
        # 其他ID计数清零
        for k in list(confirm_count.keys()):
            if k != tid:
                confirm_count[k] = 0

        if confirm_count[tid] >= CONFIRM_FRAMES and tid != last_sent_id:
            send_cargo(tid, cx, cy)
            last_sent_id = tid
            print("上报货物:", CARGO_NAME.get(tid, '?'), "cx:", cx, "cy:", cy)

    else:
        # 视野内无货物，计数全部清零
        confirm_count.clear()
        last_sent_id = -1

    time.sleep_ms(30)   # ~33fps
