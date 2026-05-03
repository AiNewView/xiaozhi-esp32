#pragma once

#include "llm_client.h"
#include "tts_client.h"

#include <string>
#include <vector>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class CustomLlmPipeline {
public:
    struct Config {
        LlmClient::Config llm;
        TtsClient::Config tts;
        bool use_tts = true;  // false = server TTS, true = local TTS client
    };

    // completion callback: (success, pcm_data, sample_rate, text_response, error_message)
    using CompletionCallback = std::function<void(bool success, std::vector<int16_t>&& pcm_data, int sample_rate, const std::string& text_response, const std::string& error_message)>;

    CustomLlmPipeline();
    ~CustomLlmPipeline();

    void SetConfig(const Config& config);
    bool IsBusy() const;
    void Process(const std::string& user_text, CompletionCallback callback);

private:
    Config config_;
    bool busy_ = false;
    TaskHandle_t worker_task_ = nullptr;

    struct WorkerContext {
        CustomLlmPipeline* pipeline;
        LlmClient llm_client;
        TtsClient tts_client;
        std::string user_text;
        CompletionCallback callback;
    };

    static void WorkerTask(void* arg);
};
