#include "Cam.hpp"

String Cam::begin(int *attrCount, String attr) {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 3;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return "";
    }
    Serial.println("get sensor ");
    sensor_t *s = esp_camera_sensor_get();

    // drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_HVGA);

#if  defined(CAM_LED_PIN) && CAM_LED_PIN > -1
    pinMode(CAM_LED_PIN, OUTPUT);
#endif


    attr += "framesize:" + String(s->status.framesize);
    attr += "\nquality:" + String(s->status.quality);
    attr += "\nbrightness:" + String(s->status.brightness);
    attr += "\ncontrast:" + String(s->status.contrast);
    attr += "\nsaturation:" + String(s->status.saturation);
    attr += "\nsharpness:" + String(s->status.sharpness);
    attr += "\nspecial_effect:" + String(s->status.special_effect);
    attr += "\nwb_mode:" + String(s->status.wb_mode);
    attr += "\nawb:" + String(s->status.awb);
    attr += "\nawb_gain:" + String(s->status.awb_gain);
    attr += "\naec:" + String(s->status.aec);
    attr += "\naec2:" + String(s->status.aec2);
    attr += "\nae_level:" + String(s->status.ae_level);
    attr += "\naec_value:" + String(s->status.aec_value);
    attr += "\nagc:" + String(s->status.agc);
    attr += "\nagc_gain:" + String(s->status.agc_gain);
    attr += "\ngainceiling:" + String(s->status.gainceiling);
    attr += "\nbpc:" + String(s->status.bpc);
    attr += "\nwpc:" + String(s->status.wpc);
    attr += "\nraw_gma:" + String(s->status.raw_gma);
    attr += "\nlenc:" + String(s->status.lenc);
    attr += "\nvflip:" + String(s->status.vflip);
    attr += "\nhmirror:" + String(s->status.hmirror);
    attr += "\ndcw:" + String(s->status.dcw);
    attr += "\ncolorbar:" + String(s->status.colorbar, DEC);
    camera_sensor_info_t *sensorInfo = esp_camera_sensor_get_info(&s->id);
    attr += "\nsupport_jpeg:" + String(sensorInfo->support_jpeg);
    attr += "\nsensor_name:" + String(sensorInfo->name);
    attr += "\npsram_size:" + String(ESP.getPsramSize());
    attr += "\nxclk_freq_hz:" + String(config.xclk_freq_hz);
    attr += '\n';

    *attrCount += 29;

    this->sensor = s;

    singleIntSettingOptMap[0] = sensor->set_contrast;
    singleIntSettingOptMap[1] = sensor->set_brightness;
    singleIntSettingOptMap[2] = sensor->set_saturation;
    singleIntSettingOptMap[3] = sensor->set_sharpness;
    singleIntSettingOptMap[4] = sensor->set_denoise;
    singleIntSettingOptMap[5] = sensor->set_quality;
    singleIntSettingOptMap[6] = sensor->set_colorbar;
    singleIntSettingOptMap[7] = sensor->set_whitebal;
    singleIntSettingOptMap[8] = sensor->set_gain_ctrl;
    singleIntSettingOptMap[9] = sensor->set_exposure_ctrl;
    singleIntSettingOptMap[10] = sensor->set_hmirror;
    singleIntSettingOptMap[11] = sensor->set_vflip;
    singleIntSettingOptMap[12] = sensor->set_aec2;
    singleIntSettingOptMap[13] = sensor->set_awb_gain;
    singleIntSettingOptMap[14] = sensor->set_agc_gain;
    singleIntSettingOptMap[15] = sensor->set_aec_value;
    singleIntSettingOptMap[16] = sensor->set_special_effect;
    singleIntSettingOptMap[17] = sensor->set_wb_mode;
    singleIntSettingOptMap[18] = sensor->set_ae_level;
    singleIntSettingOptMap[19] = sensor->set_dcw;
    singleIntSettingOptMap[20] = sensor->set_bpc;
    singleIntSettingOptMap[21] = sensor->set_wpc;
    singleIntSettingOptMap[22] = sensor->set_raw_gma;
    singleIntSettingOptMap[23] = sensor->set_lenc;
    singleIntSettingOptMap[24] = [](sensor_t *sensor_, int framesize) {
        return esp_camera_sensor_get()->set_framesize(sensor_, framesize_t(framesize));
    };
    singleIntSettingOptMap[25] = [](sensor_t *sensor_, int gainceiling) {
        return esp_camera_sensor_get()->set_gainceiling(sensor_, gainceiling_t(gainceiling));
    };
    singleIntSettingOptMap[26] = [](sensor_t *sensor_, int pixformat) {
        return esp_camera_sensor_get()->set_pixformat(sensor_, pixformat_t(pixformat));
    };

    singleIntSettingOptMap[27] = [](sensor_t *sensor_, int xclk) {
        return esp_camera_sensor_get()->set_xclk(sensor_, LEDC_TIMER_0, xclk ? 30000000 : 20000000);
    };

    return attr;
}


void Cam::createChannel(const IPAddress &address, uint16_t serverUdpPort, uint16_t localUdpPort) {
    streamSender.begin(WiFi.localIP(), localUdpPort);
    streamSender.setServer(address, serverUdpPort);
}

void Cam::sendVideo() {
    if (!videoOn) {
        // 减少空转
        delay(100);
        return;
    }
    camera_fb_t *fb = NULL;
    size_t len;
    Serial.println("do loop");
    // 表示未设置通道号,默认为simonCAM handler,兼容其它设备的发送内容
    if (channelIndex == 0xff) {
        while (videoOn) {
            fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("Camera capture failed");
                return;
            }
            len = fb->len;
            streamSender.send(fb->buf, len);
            esp_camera_fb_return(fb);
        }
    } else {
        // simonCAM server 支持多通道同时播放，因而需要设置通道号
        while (videoOn) {
            fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("Camera capture failed");
                return;
            }
            len = fb->len + 1;
            fb->buf[fb->len] = channelIndex;
            streamSender.send(fb->buf, len);
            esp_camera_fb_return(fb);
        }
    }
    Serial.println("Video sending task is over");
}

void Cam::startSend() {
    videoOn = true;
}

void Cam::stopSend() {
    videoOn = false;
}

void Cam::ledOn() {

#if  defined(CAM_LED_PIN) && CAM_LED_PIN > -1
    digitalWrite(CAM_LED_PIN, HIGH);
#else
    Serial.println("CAM_LED_PIN not defined");
    log_e("CAM_LED_PIN not defined");
#endif

}

void Cam::ledOff() {
#if  defined(CAM_LED_PIN) && CAM_LED_PIN > -1
    digitalWrite(CAM_LED_PIN, LOW);
#else
    Serial.println("CAM_LED_PIN not defined");
    log_e("CAM_LED_PIN not defined");
#endif
}

void Cam::singleIntSetting(int index, String arg) {
    int param = arg.toInt();
    this->singleIntSettingOptMap[index](this->sensor, param);
}

void Cam::otherSetting(int index, String arg) {

}

void Cam::setChannelIndex(uint8_t channel_index) {
    if (channel_index < 0xff) {
        Serial.print(" index is :");

        this->channelIndex = channel_index;
        Serial.println(channelIndex);
    }
}
