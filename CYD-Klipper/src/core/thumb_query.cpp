
#include "thumb_query.h"
#include "data_setup.h"
#include "lvgl.h"
#include "../conf/global_config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

const unsigned int MAX_SIZE = 10000;
const unsigned int MAX_RES = 64;

ThumbImg fetch_gcode_thumb(const char* gcode_filename){
    if (gcode_filename == NULL){
        Serial.println("No gcode filename");
        return {0};
    }

    String url = "http://" + String(global_config.klipperHost) + ":" + String(global_config.klipperPort) + "/server/files/thumbnails?filename=" + String(gcode_filename);
    HTTPClient client;
    int httpCode = 0;
    try {
        client.begin(url.c_str());
        httpCode = client.GET();
    }
    catch (...){
        Serial.println("Exception while fetching gcode thumb location");
        return {0};
    }
    
    if (httpCode == 200)
    {
        String payload = client.getString();
        DynamicJsonDocument doc(4096);
        deserializeJson(doc, payload);
        auto result = doc["result"].as<JsonArray>();
        int chosen_size = 0;
        int chosen_width = 0;
        const char* chosen_thumb = NULL;

        for (auto file : result){
            int width = file["width"];
            int height = file["height"];
            int size = file["size"];
            const char* thumbnail = file["thumbnail_path"];

            if (width != height)
                continue;
            
            if (size > MAX_SIZE)
                continue;

            if (width > MAX_RES)
                continue;

            if (size <= chosen_size)
                continue;

            chosen_size = size;
            chosen_thumb = thumbnail;
            chosen_width = width;
        }

        if (chosen_thumb == NULL) {
            Serial.println("No thumb filename");
            return {0};
        }

        Serial.printf("Fetching thumbnail: %s\n", chosen_thumb);
        String thumb_url = "http://" + String(global_config.klipperHost) + ":" + String(global_config.klipperPort) + "/server/files/gcodes/" + String(chosen_thumb);
        client.begin(thumb_url.c_str());
        httpCode = client.GET();
        if (httpCode == 200)
        {
            unsigned char* data_png = (unsigned char*)malloc(chosen_size + 1);
            client.getStream().readBytes(data_png, chosen_size);
            return {chosen_width, chosen_size, data_png};
        }
        else
        {
            Serial.printf("Failed to fetch gcode thumb: %d\n", httpCode);
        }
    }
    else
    {
        Serial.printf("Failed to fetch gcode thumb location: %d\n", httpCode);
    }

    return {0};
}
