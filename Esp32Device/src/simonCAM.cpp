
/*
    Updated by simon on 2023/12/24.
*/

#include <Arduino.h>
#include "simoncamClient.h"
#include "esp_camera.h"
#include "cam/Cam.hpp"

const int streamPort = 8004;
const int serverPort = 8081;
const uint16_t localUdpPort = 2333;

simonCAMClient simoncamClient;
MsgHeader msgHeader;

Cam cam;

void camGeneralOp();

void camSettingOp();

void debugOp();

void setCamChannelIndex();

void (*opTab[5])() = {debugOp, camGeneralOp,
                      camSettingOp, setCamChannelIndex};

[[noreturn]] void receiveMessage(void *) {
    Serial.println("do receive");
    while (true) {
        int result = simoncamClient.readMsgHeader(&msgHeader);

        if (result == MSG_FAILED) {
            continue;
        }
        int opCode = msgHeader.opCode;
        Serial.printf("cmdId:%d,opCode:%d\n", msgHeader.cmdIdVal, opCode);
        opTab[opCode]();
    }
}


void setup() {
    Serial.begin(115200);
    pinMode(RESET_PIN, INPUT_PULLUP); // 网络复位引脚
    while (!Serial) {
    }
    // 初始化摄像头，并将设备属性中加入摄像头已有参数
    int attrCount = 4;
    String attr = cam.begin(&attrCount, String("version:1.0\n"
                                               "cameraDev:true\n"
                                               "devType:Camera\n"
                                               // setCamChannelIndex函数指针的索引，摄像头类型的设备需要带此属性，用于simonCAM server回调
                                               "setCamChannelIndexOpCode:3\n"
    ));
    char attr_[attr.length() + 1];
    attr.toCharArray(attr_, attr.length() + 1);
    Serial.println(attr_);

    ConnectInfo connectInfo;
    connectInfo.serverPort = serverPort;
    connectInfo.attrCount = attrCount;
    connectInfo.devAttr = attr_;

#ifdef SERVER_HOST
    connectInfo.host = SERVER_HOST;
#endif
    connectInfo.userName = USER_NAME;
    connectInfo.passwd = PASS_WD;
    simoncamClient.setup_wifi();
    // 等待WiFi连接成功
    while (WiFi.status() != WL_CONNECTED) {
        // 处理客户端请求
        server.handleClient();
    }
    const IPAddress &address = simoncamClient.connectServer(&connectInfo);
    // 控制通道连接成功才可打开 视频帧通道
    delay(1000);
    cam.createChannel(address, streamPort, localUdpPort);

    xTaskCreatePinnedToCore(receiveMessage, "receiveMessage", 10000,
                            NULL, 0, NULL, 0);
}

void loop() {
    while (WiFi.status() == WL_CONNECTED) {
            // 发送视频
            cam.sendVideo();
            delay(1000);
        }
    if(WiFi.status() != WL_CONNECTED)
    {
        // wifi断开重连
        Serial.print("wifi re-connecting...");
        simoncamClient.setup_wifi();
        // 等待WiFi连接成功
        while (WiFi.status() != WL_CONNECTED) 
        {
        // 处理客户端请求
            server.handleClient();
        }
        // 重连服务器
        ConnectInfo connectInfo;
        connectInfo.serverPort = serverPort;
    #ifdef SERVER_HOST
        connectInfo.host = SERVER_HOST;
    #endif
        connectInfo.userName = USER_NAME;
        connectInfo.passwd = PASS_WD;
        const IPAddress &address = simoncamClient.connectServer(&connectInfo);
        // 控制通道连接成功才可打开 视频帧通道
        delay(1000);
        cam.createChannel(address, streamPort, localUdpPort);

        xTaskCreatePinnedToCore(receiveMessage, "receiveMessage", 10000,
                                NULL, 0, NULL, 0);
    }
}

//摄像头无参操作函数调度 opCode 为 3 参数行是 一个数字，为一个无参函数下标
void camGeneralOp() {
    int functionIndex = simoncamClient.readInt('\n');
    if (functionIndex > -1 && functionIndex < 4) {
        void (Cam::*function)() = cam.generalFunTab[functionIndex];
        (cam.*function)();
    }
}

void camSettingOp() {
    int functionIndex = simoncamClient.readInt(' ');
    // 目标函数再次调度一张函数指针表
    int subFunctionIndex = simoncamClient.readInt(' ');

    String args = simoncamClient.readStringUntil('\n');
    if (functionIndex > -1 && functionIndex < 2) {
        void (Cam::*function)(int, String) = cam.camSettingOp[functionIndex];
        (cam.*function)(subFunctionIndex, args);
    }
}

void debugOp() {
    Serial.println("rec:" + simoncamClient.readStringUntil('\n'));
    ESP.restart();
}

/**
 * simonCAM server回调设置摄像头通道
 * */
void setCamChannelIndex(){
    int channelIndex = simoncamClient.readInt('\n');
    cam.setChannelIndex(channelIndex);
}