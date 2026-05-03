#include "tts_client.h"
#include "board.h"

#include <esp_log.h>
#include <cJSON.h>
#include <cstring>

#define TAG "TtsClient"

TtsClient::TtsClient() = default;
TtsClient::~TtsClient() = default;

void TtsClient::SetConfig(const Config& config) {
    config_ = config;
}

void TtsClient::Synthesize(const std::string& text, Callback callback) {
    if (config_.api_endpoint.empty() || config_.api_key.empty()) {
        callback(false, {}, 0, "TTS endpoint or API key not configured");
        return;
    }

    auto network = Board::GetInstance().GetNetwork();
    if (!network) {
        callback(false, {}, 0, "Network not available");
        return;
    }

    auto http = network->CreateHttp(1);
    if (!http) {
        callback(false, {}, 0, "Failed to create HTTP client");
        return;
    }

    // Build JSON request body
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", config_.model.c_str());
    cJSON_AddStringToObject(root, "input", text.c_str());
    cJSON_AddStringToObject(root, "voice", config_.voice.c_str());
    cJSON_AddStringToObject(root, "response_format", config_.response_format.c_str());
    if (config_.response_format == "pcm") {
        cJSON_AddNumberToObject(root, "speed", 1.0);
    }

    char* json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // Send HTTP request
    http->SetHeader("Content-Type", "application/json");
    http->SetHeader("Authorization", ("Bearer " + config_.api_key).c_str());
    http->SetTimeout(15000);
    http->SetContent(std::string(json_str));
    free(json_str);

    ESP_LOGI(TAG, "Sending TTS request to %s", config_.api_endpoint.c_str());

    if (!http->Open("POST", config_.api_endpoint)) {
        int error = http->GetLastError();
        ESP_LOGE(TAG, "HTTP open failed, error=0x%x", error);
        callback(false, {}, 0, "Failed to connect to TTS API");
        return;
    }

    int status_code = http->GetStatusCode();

    if (status_code != 200) {
        std::string body = http->ReadAll();
        http->Close();
        ESP_LOGE(TAG, "TTS API error: %s", body.c_str());
        callback(false, {}, 0, "TTS API returned status " + std::to_string(status_code));
        return;
    }

    // Read raw PCM body
    std::string body = http->ReadAll();
    http->Close();

    if (body.empty()) {
        callback(false, {}, 0, "TTS API returned empty body");
        return;
    }

    // Convert raw bytes to int16_t PCM samples
    size_t sample_count = body.size() / sizeof(int16_t);
    std::vector<int16_t> pcm_data(sample_count);
    memcpy(pcm_data.data(), body.data(), body.size());

    ESP_LOGI(TAG, "TTS response: %u samples, %d Hz", sample_count, config_.sample_rate);
    callback(true, std::move(pcm_data), config_.sample_rate, "");
}
