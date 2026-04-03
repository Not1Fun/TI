import time, math
from media.sensor import *
from media.display import *
from media.media import *

sensor = Sensor()
sensor.reset()
sensor.set_framesize(width=400, height=240)
sensor.set_pixformat(Sensor.RGB565)

Display.init(Display.ST7701, width=640, height=480, to_ide=True)
MediaManager.init()
sensor.run()

# K230 image 模块有这些常量，可以直接用
FAMILIES = [
    ("TAG16H5",  image.TAG16H5),
    ("TAG25H7",  image.TAG25H7),
    ("TAG25H9",  image.TAG25H9),
    ("TAG36H10", image.TAG36H10),
    ("TAG36H11", image.TAG36H11),
]

clock = time.clock()

try:
    while True:
        clock.tick()
        img = sensor.snapshot()

        found = False
        for name, val in FAMILIES:
            tags = img.find_apriltags(families=val)
            if tags:
                for t in tags:
                    # K230 上全是方法，加 ()
                    print("族:{} ID:{} margin:{:.1f} cx:{} cy:{}".format(
                        name, t.id(), t.decision_margin(), int(t.cx()), int(t.cy())))
                    img.draw_rectangle(t.rect(), color=(255, 0, 0), thickness=4)
                    img.draw_cross(int(t.cx()), int(t.cy()), color=(0, 255, 0), thickness=2)
                found = True
                break

        if not found:
            print("未检测到 Tag  FPS:{:.1f}".format(clock.fps()))

        Display.show_image(img, x=120, y=120)

finally:
    sensor.stop()
    Display.deinit()
    MediaManager.deinit()
