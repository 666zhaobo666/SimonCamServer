/*
    Updated by simon on 2023/12/24.
*/

#include <WebServer.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#ifndef simonCAM_TCP_H
#define simonCAM_TCP_H

#define simonCAM_TCP_VERSION "0.0.4"

// 引入当前头文件需要先引入Arduino.h

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32

#include <WiFi.h>
#include <MD5Builder.h>

#endif

// 启动配置wifi界面
#define ROOT_HTML  "<!DOCTYPE html><html><head><title>ESP32 WIFI Config</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><style type=\"text/css\">.input{display: block; margin-top: 10px;}.input span{width: 100px; float: left; float: left; height: 36px; line-height: 36px;}.input input{height: 30px;width: 200px;}.btn{width: 120px; height: 35px; background-color: #000000; border:0px; color:#ffffff; margin-top:15px; margin-left:100px;}</style><body><form method=\"GET\" action=\"connect\"><label class=\"input\"><span>WiFi SSID</span><input type=\"text\" name=\"ssid\"></label><label class=\"input\"><span>WiFi PASS</span><input type=\"text\"  name=\"pass\"></label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submie\"> <div class=\"wifi-container\"><p><span style=\"background-color: yellow;\">Nearby wifi:</span></p></div></form>"
WebServer server(80);

//上位机为热点时 接收通知的端口
#define AS_GATE_WAY_PORT 8001

// cmd字段
#define CMD_ID "id"
#define OP_CODE "op"
#define MSG_OK 0
#define MSG_FAILED (-1)

// wifi名称与密码变量，用来从网页获取，不要在这里填^_^，没用的！
String wifi_ssid = "";
String wifi_pass = "";
String scanNetworksID = "";//用于储存扫描到的WiFi

typedef struct {
    int cmdIdVal;
    int opCode;
} MsgHeader;

typedef struct {

    // 服务端地址与端口
    char *host;
    uint16_t serverPort;
    // 设备用户名密码
    char *userName;
    char *passwd;
    // 设备属性数，和属性
    uint16_t attrCount;
    char *devAttr;
} ConnectInfo;

class simonCAMClient {
private:
    WiFiClient client;


#include <WiFiUdp.h>

#define TOPIC_LEN 12
#define TOPIC "simonCAM-server"

    static IPAddress getServerHost() {
        const String topic = String(TOPIC);
        WiFiUDP udp;
        uint8_t buf[TOPIC_LEN];
        // 加入组，接收来自上位机的组播
        udp.beginMulticast(IPAddress(224, 0, 0, 5), 8000);
        Serial.println("get server ip...");

        // 当上位机开热点的时候 大概率组播路由不会走它自己的ap的网段，这里向热点ip网关发送信息，
        // 如果对方是上位机，收到当前包 ，就往当前地址发包 来确认身份
        uint8_t client_topic[12] = {'l', 'i', 'g', 'h', 't',
                                    '-', 'c', 'l', 'i', 'e', 'n', 't'};
        const IPAddress &gwIp = WiFi.gatewayIP();

        while (true) {
            int len = udp.parsePacket();
            if (len >= TOPIC_LEN) {
                // 先尝试接受组播，
                udp.read(buf, TOPIC_LEN);
                if (topic.equals(String(buf, TOPIC_LEN))) {
                    const IPAddress &address = udp.remoteIP();
                    Serial.println();
                    Serial.print("remote ip is:");
                    Serial.println(address);
                    udp.stop();
                    return address;
                }

            }
            udp.beginPacket(gwIp, AS_GATE_WAY_PORT);
            udp.write(client_topic, 12);
            udp.endPacket();
            Serial.print("\r.");
            delay(400);
            Serial.print("\r..");
        }
    }

public:
    int readInt(char terminator) {
        String str = client.readStringUntil(terminator);
        return str.toInt();
    }

    String readKey() {
        String key = client.readStringUntil(':');
        key.trim();
        return key;
    }

    // 发现不是key 丢掉后续
    void dropLine(const String &key) {
        Serial.print("drop msg:\n\t");
        Serial.print(key);
        Serial.print(':');
        Serial.print(client.readStringUntil('\n'));
    }

    // 读字符串
    String readStringUntil(char terminator) {
        return client.readStringUntil(terminator);
    }

    int readMsgHeader(MsgHeader *msgHeader) {
        String cmdIdKey = readKey();
        if (cmdIdKey.isEmpty()) {
            return MSG_FAILED;
        }
        if (!cmdIdKey.equals(CMD_ID)) {
            dropLine(cmdIdKey);
            return MSG_FAILED;
        }
        int cmdIdVal = readInt('\n');

        String opCodeKey = readKey();
        if (!opCodeKey.equals(OP_CODE)) {
            dropLine(opCodeKey);
            return MSG_FAILED;
        }
        int opCode = readInt('\n');

        msgHeader->cmdIdVal = cmdIdVal;
        msgHeader->opCode = opCode;
        return MSG_OK;
    }

    /******************************/
    /***** 自动化配置 wifi 函数 ****/
    /******************************/
    //用于配置WiFi
    void wifi_Config()
    {
    Serial.println("scan start");
    // 扫描附近WiFi
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
        scanNetworksID = "no networks found";
    } else {
        Serial.print(n);
        Serial.println("networks found");
        for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
        scanNetworksID += "<P>" + WiFi.SSID(i) + "</P>";
        delay(10);
        }
    }
    Serial.println("");

    WiFi.mode(WIFI_AP);//配置为AP模式
    boolean result = WiFi.softAP(AP_SSID, ""); //开启WIFI热点
    // boolean result = WiFi.softAP(AP_SSID, AP_PASSWD); //开启WIFI热点(带密码)

    if (result)
    {
        IPAddress myIP = WiFi.softAPIP();
        //打印相关信息
        Serial.println("");
        Serial.print("Soft-AP IP address = ");
        Serial.println(myIP);
        Serial.println(String("MAC address = ")  + WiFi.softAPmacAddress().c_str());
        Serial.println("waiting ...");
    } else {  //开启热点失败
        Serial.println("WiFiAP Failed");
        delay(3000);
        ESP.restart();  //复位esp32
    }

    if (MDNS.begin("esp32")) {
        Serial.println("MDNS responder started");
    }

    //首页
    server.on("/", []() {
        server.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");
    });

    //连接
    server.on("/connect", []() {

        server.send(200, "text/html", "<html><body><font size=\"10\">successd,wifi connecting...<br />Please close this page manually.</font></body></html>");

        WiFi.softAPdisconnect(true);
        //获取输入的WIFI账户和密码
        wifi_ssid = server.arg("ssid");
        wifi_pass = server.arg("pass");
        server.close();
        WiFi.softAPdisconnect();
        Serial.println("WiFi Connect SSID:" + wifi_ssid + "  PASS:" + wifi_pass);

        //设置为STA模式并连接WIFI
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
        uint8_t Connect_time = 0; //用于连接计时，如果长时间连接不成功，复位设备
        while (WiFi.status() != WL_CONNECTED) {  //等待WIFI连接成功
        delay(500);
        Serial.print(".");
        Connect_time ++;
        if (Connect_time > 80) {  //长时间连接不上，复位设备
            Serial.println("Connection timeout, check input is correct or try again later!");
            delay(3000);
            ESP.restart();
        }
        }
        Serial.println("");
        Serial.println("WIFI Config Success");
        Serial.printf("SSID:%s", WiFi.SSID().c_str());
        Serial.print("  LocalIP:");
        Serial.print(WiFi.localIP());
        Serial.println("");

    });
    server.begin();
    }

    //用于上电自动连接WiFi
    bool AutoConfig()
    {
    WiFi.begin();
    for (int i = 0; i < 15; i++)
    {
        int wstatus = WiFi.status();
        if (wstatus == WL_CONNECTED)
        {
        Serial.println("WIFI SmartConfig Success");
        Serial.printf("SSID:%s", WiFi.SSID().c_str());
        Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
        Serial.print("LocalIP:");
        Serial.print(WiFi.localIP());
        Serial.print(" ,GateIP:");
        Serial.println(WiFi.gatewayIP());
        return true;
        }
        else
        {
        Serial.print("WIFI AutoConfig Waiting......");
        Serial.println(wstatus);
        delay(1000);
        }
    }
    Serial.println("WIFI AutoConfig Faild!" );
    return false;
    }

    /*连接wifi函数*/
    void setup_wifi() 
    {
        // 连接WiFi
        if (!AutoConfig())
        {
            wifi_Config();
        }

        //用于删除已存WiFi
        if (digitalRead(RESET_PIN) == LOW) {
            delay(1000);
            esp_wifi_restore();
            delay(10);
            ESP.restart();  //复位esp32
        }
    }
    /******************************/
    /***** 自动化配置 wifi 结束 ****/
    /******************************/

    bool login(const char *userName,
               const char *passwd) {
        // 获取nonce
        uint8_t nonce[8];
        this->client.write("nonce");
        this->client.flush();
        Serial.println("send nonce request\nnone len:");
        size_t len;
        do {
            Serial.print("\rtry get none:");
            len = this->client.readBytes(nonce, 8);
            Serial.print(len);
        } while (len < 1);

        Serial.println("\nget nonce down");
        // 发送用户名
        this->client.write(strlen_P(userName));
        this->client.write(userName);
        this->client.flush();
        Serial.println("send userName");
        // md5(userName + passwd)
        uint8_t md[16];
        MD5Builder md5Builder{};
        md5Builder.begin();
        md5Builder.add(userName);
        md5Builder.add(passwd);
        md5Builder.calculate();
        md5Builder.getBytes(md);
        // 认证的摘要 md5(nonce + md5(userName + passwd))
        md5Builder.begin();
        md5Builder.add(nonce, 8);
        md5Builder.add(md, 16);
        md5Builder.calculate();
        md5Builder.getBytes(md);
        this->client.write(md, 16);
        this->client.flush();
        Serial.println("send message digest");
        delay(200);
        return this->client.connected();
    }


    void helloServer(const uint16_t attrCount, const char *devAttr) {
        //与服务端握手
        client.println("hello");
        uint8_t mac[6];
        WiFi.macAddress(mac);
        Serial.printf("MAC is:%02X%02X%02X%02X%02X%02X\n", mac[0], mac[1],
                      mac[2], mac[3], mac[4], mac[5]);
        // sn 长度，这里将mac作为设备序列号。长度为6字节
        client.write(6);
        client.write(mac, 6);
        client.write('\n');
        //属性行数，占两个字节
        client.write((0xff00 & attrCount) >> 8);
        client.write(0xff & attrCount);
        client.write('\n');
        client.write(devAttr);
        client.write('\n');
        client.flush();
    }

    IPAddress connectServer(ConnectInfo *connectInfo) {
        Serial.println("simonCAM client version:"
                       simonCAM_TCP_VERSION
        );
        // connectWifi(connectInfo->ssid, connectInfo->passphrase);

        IPAddress address((uint32_t) 0);
        if (connectInfo->host == nullptr) {
            Serial.println("Server host is not specified");
            address = getServerHost();
        } else{
            Serial.println("Server host is specified as :");
            Serial.println(connectInfo->host);
            if (!(WiFiGenericClass::hostByName(connectInfo->host, address))) {
                Serial.println("get host by name failed");
                return address;
            }
        }


        WiFiClient wiFiClient;
        if (WiFiClass::status() != WL_CONNECTED) {
            return false;
        }
        while (!wiFiClient.connect(address, connectInfo->serverPort)) {
            Serial.println("connection failed,wait 1 sec...");
            delay(1000);
        }
        this->client = wiFiClient;
        Serial.println("connect to tcp server success.");
        this->client.setNoDelay(true);
        if (!login(connectInfo->userName, connectInfo->passwd)) {
            Serial.println("login in failed!");
            return false;
        }
        helloServer(connectInfo->attrCount, connectInfo->devAttr);

        return address;
    }

};

#endif  // simonCAM_TCP_H
