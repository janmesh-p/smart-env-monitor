#include <stdio.h>
#include <curl/curl.h>
#include "thingspeak.h"

#define THINGSPEAK_API_KEY "UETPF0GH3IHIQ6RW"
#define THINGSPEAK_URL "https://api.thingspeak.com/update"

void thingspeak_post(float temp, float hum, int fan_pct, int led_pct, int ldr_val, int mode) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char postdata[256];
        snprintf(postdata, sizeof(postdata),
                 "api_key=%s&field1=%.2f&field2=%.2f&field3=%d&field4=%d&field5=%d&field6=%d",
                 THINGSPEAK_API_KEY, temp, hum, fan_pct, ldr_val, led_pct, mode);
        curl_easy_setopt(curl, CURLOPT_URL, THINGSPEAK_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}
