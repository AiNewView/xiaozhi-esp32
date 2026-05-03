#include "llm_client.h"
#include "board.h"

#include <esp_log.h>
#include <cJSON.h>
#include <cstring>

#define TAG "LlmClient"

LlmClient::LlmClient() = default;
LlmClient::~LlmClient() = default;

void LlmClient::SetConfig(const Config& config) {
    config_ = config;
}

void LlmClient::Chat(const std::string& user_text, Callback callback) {
    if (config_.api_endpoint.empty() || config_.api_key.empty()) {
        callback(false, "", "LLM endpoint or API key not configured");
        return;
    }

    auto network = Board::GetInstance().GetNetwork();
    if (!network) {
        callback(false, "", "Network not available");
        return;
    }

    auto http = network->CreateHttp(1);
    if (!http) {
        callback(false, "", "Failed to create HTTP client");
        return;
    }

    // Build JSON request body
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", config_.model.c_str());

    cJSON* messages = cJSON_AddArrayToObject(root, "messages");

    cJSON* sys_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(sys_msg, "role", "system");
    cJSON_AddStringToObject(sys_msg, "content", config_.system_prompt.c_str());
    cJSON_AddItemToArray(messages, sys_msg);

    cJSON* user_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(user_msg, "role", "user");
    cJSON_AddStringToObject(user_msg, "content", user_text.c_str());
    cJSON_AddItemToArray(messages, user_msg);

    cJSON_AddNumberToObject(root, "max_tokens", config_.max_tokens);
    cJSON_AddNumberToObject(root, "temperature", config_.temperature);

    char* json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // Send HTTP request
    http->SetHeader("Content-Type", "application/json");
    http->SetHeader("Authorization", ("Bearer " + config_.api_key).c_str());
    http->SetTimeout(15000);
    http->SetContent(std::string(json_str));
    free(json_str);

    ESP_LOGI(TAG, "Sending LLM request to %s", config_.api_endpoint.c_str());

    if (!http->Open("POST", config_.api_endpoint)) {
        int error = http->GetLastError();
        ESP_LOGE(TAG, "HTTP open failed, error=0x%x", error);
        callback(false, "", "Failed to connect to LLM API");
        return;
    }

    int status_code = http->GetStatusCode();
    std::string body = http->ReadAll();
    http->Close();

    ESP_LOGI(TAG, "LLM response: status=%d, body_len=%d", status_code, body.length());

    if (status_code != 200) {
        ESP_LOGE(TAG, "LLM API error: %s", body.c_str());
        callback(false, "", "LLM API returned status " + std::to_string(status_code));
        return;
    }

    // Parse response JSON
    cJSON* response = cJSON_Parse(body.c_str());
    if (!response) {
        callback(false, "", "Failed to parse LLM response JSON");
        return;
    }

    cJSON* choices = cJSON_GetObjectItem(response, "choices");
    if (!cJSON_IsArray(choices) || cJSON_GetArraySize(choices) == 0) {
        cJSON_Delete(response);
        callback(false, "", "LLM response: no choices");
        return;
    }

    cJSON* first_choice = cJSON_GetArrayItem(choices, 0);
    cJSON* message = cJSON_GetObjectItem(first_choice, "message");
    cJSON* content = cJSON_GetObjectItem(message, "content");

    if (!cJSON_IsString(content)) {
        cJSON_Delete(response);
        callback(false, "", "LLM response: missing content");
        return;
    }

    std::string response_text(content->valuestring);
    cJSON_Delete(response);

    ESP_LOGI(TAG, "LLM response text: %s", response_text.c_str());
    callback(true, response_text, "");
}
