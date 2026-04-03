import time, os, sys
from media.sensor import *
from media.display import *
from media.media import *
from machine import UART

# ────────────────────────────────────────────
#  串口初始化
# ────────────────────────────────────────────
uart = UART(UART.UART1, 115200)

# ────────────────────────────────────────────
#  显示参数
# ────────────────────────────────────────────
DISPLAY_WIDTH  = 640
DISPLAY_HEIGHT = 480

# ────────────────────────────────────────────
#  红色 LAB 阈值（需在实际场地光照下重新标定）
#  格式：(L_min, L_max, A_min, A_max, B_min, B_max)
# ────────────────────────────────────────────
RED_THRESHOLD = (0, 66, 7, 127, 3, 127)

# 色块最小面积，过滤噪声
AREA_THRESHOLD = 3000

# ────────────────────────────────────────────
#  z 轴距离估算参数
#  原理：z = FOCAL_LEN * REAL_WIDTH_PX_AT_REF / blob.w
#  标定方法：将摄像头置于红框正上方 REF_DIST_MM 处，
#            记录此时 blob.w 的像素值填入 REF_BLOB_W
# ────────────────────────────────────────────
REF_DIST_MM  = 200    # 标定距离（mm），摄像头距红框
REF_BLOB_W   = 120    # 标定距离下红框色块的像素宽度（需实测填入）
FOCAL_LEN    = REF_BLOB_W * REF_DIST_MM   # = 像素焦距 × 实际宽度

def estimate_z(blob_w):
    if blob_w == 0:
        return 9999
    return int(FOCAL_LEN / blob_w)

# ────────────────────────────────────────────
#  串口发送
#  协议：AA 02 cx_H cx_L cy_H cy_L z_H z_L BB
# ────────────────────────────────────────────
FRAME_HEADER  = 0xAA
FRAME_TAIL    = 0xBB
TYPE_RED      = 0x02

def send_red(cx, cy, z):
    buf = bytes([
        FRAME_HEADER,
        TYPE_RED,
        (cx >> 8) & 0xFF, cx & 0xFF,
        (cy >> 8) & 0xFF, cy & 0xFF,
        (z  >> 8) & 0xFF, z  & 0xFF,
        FRAME_TAIL
    ])
    uart.write(buf)

# ────────────────────────────────────────────
#  主函数
# ────────────────────────────────────────────
def main():
    sensor = Sensor()
    sensor.reset()
    sensor.set_framesize(width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT)
    sensor.set_pixformat(Sensor.RGB565)

    Display.init(Display.ST7701, to_ide=True)
    MediaManager.init()
    sensor.run()

    clock = time.clock()

    try:
        while True:
            clock.tick()
            img = sensor.snapshot()

            blobs = img.find_blobs([RED_THRESHOLD],
                                   area_threshold=AREA_THRESHOLD,
                                   merge=True)

            if blobs:
                # 取面积最大的色块（最可能是目标红框）
                best = max(blobs, key=lambda b: b[2] * b[3])
                cx = best[5]
                cy = best[6]
                z  = estimate_z(best[2])

                img.draw_rectangle(best[0:4], color=(255, 0, 0), thickness=4)
                img.draw_cross(cx, cy, color=(0, 255, 0), thickness=2)
                img.draw_string_advanced(best[0], best[1] - 30, 24,
                                         "z:{}mm".format(z),
                                         color=(255, 255, 0))

                send_red(cx, cy, z)
                print("红框 cx:{} cy:{} z:{}mm".format(cx, cy, z))

            img.draw_string_advanced(0, 0, 30,
                                     "FPS:{:.1f}".format(clock.fps()),
                                     color=(255, 255, 255))
            Display.show_image(img)

    except KeyboardInterrupt:
        pass
    finally:
        sensor.stop()
        Display.deinit()
        MediaManager.deinit()

if __name__ == "__main__":
    main()
