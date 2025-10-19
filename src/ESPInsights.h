#ifndef ESPInsights_H
#define ESPInsights_H
#pragma once

#include <IWebConfig.h>


#include "esp_log.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#define ESPInsights_TAG "ESPInsights"
#include "esp32-hal-log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#include <esp_insights.h>
#include "esp_diagnostics_system_metrics.h"
#include <esp_diagnostics_metrics.h>
#include <esp_diagnostics_variables.h>
#include "esp_rmaker_utils.h"


class ESPInsights : public IWebConfig {
public:
	static ESPInsights* getInstance();
	ESPInsights(const ESPInsights& obj)	= delete;
	void operator=(ESPInsights const&)  = delete;

	void setup();
	bool isEnabled(void) { return enabled;}

	// TODO: this is not that functional. 
	// It should be possible to disable the insights and re-enable it again 
	// with new init configuration using the setup() method
	void stop(void) { 
		// ESPInsights::reset();
		esp_insights_disable();
	}

	void parseWebConfig(JsonObjectConst configObject);

private:

	static ESPInsights* instance;

	bool enabled = false;
	String authKey = "";
	int metricsInterval = 60000;

	TaskHandle_t xMetricsHandler = nullptr;

	ESPInsights();
	~ESPInsights();

	void reset();

};

#endif