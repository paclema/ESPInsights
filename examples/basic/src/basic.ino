#include <Arduino.h>

#include <WiFi.h>

#include "esp32-hal-log.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#include <esp_insights.h>
#include "esp_diagnostics_system_metrics.h"
#include <esp_diagnostics_metrics.h>
#include <esp_diagnostics_variables.h>
#include "esp_rmaker_utils.h"

#include <firmware_info.h>

const char* ssid     = "wifiname";
const char* password = "wifipass";

#define ESP_INSIGHTS_AUTH_KEY "changeme"    // <- Replace with your ESP-Insights auth key from https://dashboard.insights.espressif.com/home/manage-auth-keys/
#define METRICS_DUMP_INTERVAL           10 * 1000
#define METRICS_DUMP_INTERVAL_TICKS     (METRICS_DUMP_INTERVAL / portTICK_RATE_MS)
static const char *TAG_INSIGHTS = "INSIGHTS";

unsigned long currentLoopMillis = 0;
unsigned long previousMainLoopMillis = 0;

unsigned long lastPublishedMetrics = millis();
bool insightsEnabled = true;
bool insightsLoop = true;


uint32_t taskCounter = 0;
uint32_t loopCounter = 0;


void TaskEspInsights( void *pvParameters ){
    while (true) {
        esp_diag_heap_metrics_dump();
        esp_diag_wifi_metrics_dump();
        ESP_LOGW(TAG_INSIGHTS, "ESP-Insights heap&wifi metrics updated from xTask. xTask counter: %" PRIu32, taskCounter);

        esp_diag_metrics_add_uint("taskCounter", taskCounter);
        esp_diag_variable_add_uint("taskCounterVariable", taskCounter);

        taskCounter++;
        vTaskDelay(METRICS_DUMP_INTERVAL_TICKS*2);
    }
}

void init_insights(void){
    esp_rmaker_time_sync_init(NULL);
    esp_insights_config_t config = {
      .log_type = ESP_DIAG_LOG_TYPE_ERROR | ESP_DIAG_LOG_TYPE_WARNING | ESP_DIAG_LOG_TYPE_EVENT,
      .auth_key = ESP_INSIGHTS_AUTH_KEY,
    };

    esp_err_t ret = esp_insights_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_INSIGHTS, "Failed to initialize ESP Insights, err:0x%x", ret);
    }
    // ESP_ERROR_CHECK(ret);

    esp_diag_heap_metrics_dump();
    esp_diag_wifi_metrics_dump();

    // Register a Metric:
    esp_diag_metrics_register("TaskCounter", "taskCounter", "Task Counter", "TestMetrics", ESP_DIAG_DATA_TYPE_UINT);

    // Register a Variable:
    esp_diag_variable_register("TestVariables", "taskCounterVariable", "Task Counter", "TestVariables.VariableGroup", ESP_DIAG_DATA_TYPE_UINT);
    esp_diag_variable_add_uint("taskCounterVariable", taskCounter);
    
    esp_diag_variable_register("TestVariables", "loopCounterSTR", "Loop Counter String", "TestVariables.VariableGroup", ESP_DIAG_DATA_TYPE_STR);


    xTaskCreatePinnedToCore(
        TaskEspInsights
        ,  "EspInsights"
        ,  1024*4  // Stack size
        ,  NULL
        ,  1  // Priority
        ,  NULL 
        ,  ARDUINO_RUNNING_CORE);

}




void setup() {

    // Open USB serial port
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    // esp_log_level_set("*", ESP_LOG_VERBOSE);
    esp_log_level_set("*", ESP_LOG_ERROR);
    esp_log_level_set("cpu_start", ESP_LOG_INFO);
    esp_log_level_set(TAG_INSIGHTS, ESP_LOG_VERBOSE);
    // esp_log_level_set("esp_core_dump_elf", ESP_LOG_VERBOSE);

    show_flash_info();
    show_firmware_description();

    Serial.printf("\n\nConnecting to %s", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    init_insights();

    ESP_LOGI(TAG_INSIGHTS, "###  Looping time");
    log_i("###  Looping time");
}

void loop() {

    currentLoopMillis = millis();

    if(insightsEnabled && insightsLoop && (currentLoopMillis-lastPublishedMetrics > METRICS_DUMP_INTERVAL)){
        lastPublishedMetrics = currentLoopMillis;
        esp_diag_heap_metrics_dump();
        esp_diag_wifi_metrics_dump();
        ESP_LOGW(TAG_INSIGHTS, "ESP-Insights heap & wifi metrics updated from loop. Loop counter: %" PRIu32, loopCounter);
        
        char loopCounterStr[50];
        snprintf(loopCounterStr, sizeof(loopCounterStr), "Loop counter: %d", loopCounter);
        esp_diag_variable_add_str("loopCounterSTR", loopCounterStr);
        loopCounter++;
    }

    previousMainLoopMillis = currentLoopMillis;
}