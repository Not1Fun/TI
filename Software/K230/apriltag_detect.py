import time, math, os, gc
from media.sensor import *
from media.display import *
from media.media import *
from machine import UART

# ────────────────────────────────────────────
#  串口初始化
# ────────────────────────────────────────────
uart = UART(UART.UART1, 115200)

# ────────────────────────────────────────────
#  常量定义
# ────────────────────────────────────────────
SENSOR_WIDTH   = 400
SENSOR_HEIGHT  = 240

FRAME_HEADER        = 0xAA
FRAME_TAIL          = 0xBB
TYPE_CARGO          = 0x01

CARGO_NAME          = {0: 'A', 1: 'B', 2: 'C'}
DECISION_MARGIN_MIN = 5     # 置信度阈值，根据 raw 打印出的 margin 实际值调整
CONFIRM_FRAMES      = 3     # 连续确认帧数，防抖

# ────────────────────────────────────────────
#  串口发送
#  协议：AA 01 id cx_H cx_L cy_H cy_L BB
# ────────────────────────────────────────────
def send_cargo(tag_id, cx, cy):
    buf = bytes([
        FRAME_HEADER,
        TYPE_CARGO,
        tag_id & 0xFF,
        (cx >> 8) & 0xFF, cx & 0xFF,
        (cy >> 8) & 0xFF, cy & 0xFF,
        FRAME_TAIL
    ])
    uart.write(buf)

# ────────────────────────────────────────────
#  调试绘图（正式比赛注释掉整个函数体即可）
# ────────────────────────────────────────────
def draw_debug(img, tag, tid, cx, cy, fps):
    # 通过 margin 的 tag 加蓝色边框高亮
    img.draw_rectangle(tag.rect(), color=(0, 0, 255), thickness=6)
    img.draw_string_advanced(cx - 10, cy - 60, 28,
                             "{}  PASS".format(CARGO_NAME.get(tid, '?')),
                             color=(0, 0, 255))
    # FPS
    img.draw_string_advanced(0, 0, 30,
                             "FPS:{:.1f}".format(fps),
                             color=(255, 255, 255))

# ────────────────────────────────────────────
#  主函数
# ────────────────────────────────────────────
def main():
    sensor = Sensor()
    sensor.reset()
    sensor.set_framesize(width=SENSOR_WIDTH, height=SENSOR_HEIGHT)
    sensor.set_pixformat(Sensor.RGB565)

    Display.init(Display.ST7701, to_ide=True)
    MediaManager.init()
    sensor.run()

    clock      = time.clock()
    confirm_count = {}
    last_sent_id  = -1

    try:
        while True:
            clock.tick()
            img = sensor.snapshot()

            tags = img.find_apriltags(families=image.TAG36H11)

            # 调试：所有检测到的 tag 都画框，不管 margin（正式比赛注释掉）
            for tag in tags:
                img.draw_rectangle(tag.rect(), color=(0, 255, 0), thickness=3)
                img.draw_cross(int(tag.cx()), int(tag.cy()), color=(255, 0, 0), thickness=2)
                img.draw_string_advanced(int(tag.cx()) - 10, int(tag.cy()) - 35, 24,
                                         "id:{} m:{:.0f}".format(tag.id(), tag.decision_margin()),
                                         color=(255, 255, 0))

            # 调试：打印原始 tag 信息（正式比赛注释掉）
            if tags:
                print("raw:", [(t.id(), round(t.decision_margin(), 1)) for t in tags])

            valid_tags = [t for t in tags if t.decision_margin() > DECISION_MARGIN_MIN]

            if valid_tags:
                best = max(valid_tags, key=lambda t: t.decision_margin())
                tid  = best.id()
                cx   = int(best.cx())
                cy   = int(best.cy())

                # 调试绘图：通过 margin 的 tag 额外加蓝色高亮（正式比赛注释掉下一行）
                draw_debug(img, best, tid, cx, cy, clock.fps())

                # 连续确认防抖
                confirm_count[tid] = confirm_count.get(tid, 0) + 1
                for k in list(confirm_count.keys()):
                    if k != tid:
                        confirm_count[k] = 0

                if confirm_count[tid] >= CONFIRM_FRAMES and tid != last_sent_id:
                    send_cargo(tid, cx, cy)
                    last_sent_id = tid
                    print("上报货物:", CARGO_NAME.get(tid, '?'), "cx:", cx, "cy:", cy)

            else:
                confirm_count.clear()
                last_sent_id = -1

            Display.show_image(img)

    except KeyboardInterrupt:
        pass
    finally:
        sensor.stop()
        Display.deinit()
        MediaManager.deinit()

if __name__ == "__main__":
    main()
