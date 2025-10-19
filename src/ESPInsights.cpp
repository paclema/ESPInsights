#include "ESPInsights.h"
ESPInsights* ESPInsights::instance = nullptr;

ESPInsights::ESPInsights(){
    this->nameConfigObject = "ESPInsights";
    esp_log_level_set("INSIGHTS", ESP_LOG_VERBOSE);
    esp_log_level_set("esp_insights", ESP_LOG_VERBOSE);

}

ESPInsights::~ESPInsights() {
	if (xMetricsHandler) {
        vTaskDelete(xMetricsHandler);
        xMetricsHandler = nullptr;
    }
}

ESPInsights* ESPInsights::getInstance() {
    if (!instance) {
        log_v("Creating singleton instance...");
        instance = new ESPInsights();
    } else 
        log_v("Passing singleton instance");

    return instance;
}


void ESPInsights::setup() {

    if (instance) {
        log_v("ESPInsights alreay instanced. Reseting ESPInsights instance before setting it up...");
        ESPInsights::reset();
    }

    // log_d("Setting up ESPInsights enabled: %s every %dms and key: %s ", this->enabled? "true" : "false", this->metricsInterval, this->authKey.c_str());
    esp_rmaker_time_sync_init(NULL);
    esp_insights_config_t config = {
      .log_type = ESP_DIAG_LOG_TYPE_ERROR | ESP_DIAG_LOG_TYPE_WARNING | ESP_DIAG_LOG_TYPE_EVENT,
      .auth_key = this->authKey.c_str(),
      .alloc_ext_ram = true,
    };

    esp_err_t ret = esp_insights_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE("INSIGHTS", "Failed to initialize ESP Insights, err:0x%x", ret);
    } else {
        ESP_LOGV("INSIGHTS", "ESP Insights initialized");
    }
    // ESP_ERROR_CHECK(ret);

    esp_diag_heap_metrics_reset_interval((uint32_t) this->metricsInterval/1000);
    esp_diag_wifi_metrics_reset_interval((uint32_t) this->metricsInterval/1000);
}

void ESPInsights::reset() {
    // WARNING: This method is not working properly. It is not possible to reset ESP Insights.
    // After calling esp_insights_deinit(); the component is not able to be initialized again using the setup() method.
    ESP_LOGE("INSIGHTS", "Reseting ESP Insights");
    // esp_insights_disable();
    // esp_insights_transport_unregister();
    esp_insights_deinit();
    if (xMetricsHandler) {
        ESP_LOGE("INSIGHTS", "Deleting ESP Insights xMetricsHandler");
        vTaskDelete(xMetricsHandler);
        xMetricsHandler = nullptr;
     }
}

void ESPInsights::parseWebConfig(JsonObjectConst configObject) {

    // JsonObject received:
    // serializeJsonPretty(configObject, Serial);

    // ESPInsights IWebConfig object:
    this->enabled = configObject["enabled"] | false;
    this->authKey = configObject["auth_key"].as<String>();
    this->metricsInterval = configObject["metrics_interval_ms"];

    esp_diag_heap_metrics_reset_interval((uint32_t) this->metricsInterval/1000);
    esp_diag_wifi_metrics_reset_interval((uint32_t) this->metricsInterval/1000);

    // log_e("parseWebConfig ESPInsights enabled: %s every %dms and key: %s ", this->enabled? "true" : "false", this->metricsInterval, this->authKey.c_str());
};
