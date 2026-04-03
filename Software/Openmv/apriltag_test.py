import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_auto_gain(True)
sensor.set_auto_whitebal(True)
sensor.set_auto_exposure(True)
sensor.skip_frames(time=2000)

# v4.8.x 中 Tag 族用数值传入
# TAG16H5=1  TAG25H7=2  TAG25H9=4  TAG36H10=8  TAG36H11=16  ARTOOLKIT=32
TAG16H5   = 1
TAG25H7   = 2
TAG25H9   = 4
TAG36H10  = 8
TAG36H11  = 16
ARTOOLKIT = 32

clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()

    for name, val in [("TAG16H5",1),("TAG25H7",2),("TAG25H9",4),
                      ("TAG36H10",8),("TAG36H11",16)]:
        tags = img.find_apriltags(families=val)
        if tags:
            for t in tags:
                print("族:{} ID:{} margin:{:.1f} cx:{} cy:{}".format(
                    name, t.id, t.decision_margin, int(t.cx), int(t.cy)))
            break
    else:
        print("未检测到 Tag  FPS:{:.1f}".format(clock.fps()))

    time.sleep_ms(200)
