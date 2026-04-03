import time, os, sys
from media.sensor import *
from media.display import *
from media.media import *

DISPLAY_WIDTH  = 640
DISPLAY_HEIGHT = 480

# 测试时可同时显示多组阈值效果，方便标定
# 最终只保留 RED_THRESHOLD 一条
RED_THRESHOLD = (0, 66, 7, 127, 3, 127)

AREA_THRESHOLD = 3000

REF_DIST_MM = 200
REF_BLOB_W  = 120
FOCAL_LEN   = REF_BLOB_W * REF_DIST_MM

def estimate_z(blob_w):
    if blob_w == 0:
        return 9999
    return int(FOCAL_LEN / blob_w)

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

            for blob in blobs:
                cx = blob[5]
                cy = blob[6]
                z  = estimate_z(blob[2])
                # 画红框 + 绿色十字
                img.draw_rectangle(blob[0:4], color=(255, 0, 0), thickness=4)
                img.draw_cross(cx, cy, color=(0, 255, 0), thickness=2)
                # 显示尺寸和距离
                img.draw_string_advanced(
                    blob[0], blob[1] - 30, 22,
                    "w:{} h:{} z:{}mm".format(blob[2], blob[3], z),
                    color=(255, 255, 0))
                print("cx:{} cy:{} w:{} h:{} z:{}mm".format(
                    cx, cy, blob[2], blob[3], z))

            img.draw_string_advanced(0, 0, 30,
                                     "FPS:{:.1f} blobs:{}".format(clock.fps(), len(blobs)),
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
