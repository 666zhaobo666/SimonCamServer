#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "UdpClient.hpp"

#ifndef cam_meta
#define cam_meta
class Cam {
private:
    simonCAMUDP streamSender;

    /**
     * 视频是否打开
     *
     *
     */
    volatile bool videoOn = false;

    sensor_t *sensor{};

    int (*singleIntSettingOptMap[28])(sensor_t *, int){};

    uint8_t channelIndex = 0xff;

public:
    String begin(int *attrCount, String attr);

    void createChannel(const IPAddress &address, uint16_t port, uint16_t localUdpPort = 2333);

    void startSend();

    void stopSend();

    /**
     * @brief 发送视频帧,同步调用会阻塞
     *
     *
     */
    void sendVideo();

    void ledOn();

    void ledOff();

    void singleIntSetting(int index, String arg);

    void otherSetting(int index, String arg);

    void (Cam::*generalFunTab[4])() = {&Cam::startSend, &Cam::stopSend, &Cam::ledOn, &Cam::ledOff};

    void (Cam::*camSettingOp[2])(int, String) = {&Cam::singleIntSetting,
                                                 &Cam::otherSetting};

    void setChannelIndex(uint8_t channel_index);
};
#endif // cam_meta