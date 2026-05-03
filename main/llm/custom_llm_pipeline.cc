#include "custom_llm_pipeline.h"

#include <esp_log.h>
#include <cstring>

#define TAG "CustomLlmPipeline"

CustomLlmPipeline::CustomLlmPipeline() = default;
CustomLlmPipeline::~CustomLlmPipeline() {
    if (worker_task_) {
        vTaskDelete(worker_task_);
    }
}

void CustomLlmPipeline::SetConfig(const Config& config) {
    config_ = config;
}

bool CustomLlmPipeline::IsBusy() const {
    return busy_;
}

void CustomLlmPipeline::Process(const std::string& user_text, CompletionCallback callback) {
    if (busy_) {
        callback(false, {}, 0, "", "Pipeline is busy");
        return;
    }

    busy_ = true;
    auto* ctx = new WorkerContext();
    ctx->pipeline = this;
    ctx->llm_client.SetConfig(config_.llm);
    ctx->tts_client.SetConfig(config_.tts);
    ctx->user_text = user_text;
    ctx->callback = std::move(callback);

    xTaskCreate(WorkerTask, "llm_worker", 8192, ctx, 2, &worker_task_);
}

void CustomLlmPipeline::WorkerTask(void* arg) {
    auto* ctx = static_cast<WorkerContext*>(arg);

    ESP_LOGI(TAG, "Worker task started, user_text: %s", ctx->user_text.c_str());

    // Step 1: Call LLM
    ctx->llm_client.Chat(ctx->user_text, [ctx](bool llm_success, const std::string& response_text, const std::string& llm_error) {
        if (!llm_success) {
            ESP_LOGE(TAG, "LLM failed: %s", llm_error.c_str());
            ctx->callback(false, {}, 0, "", llm_error);
            ctx->pipeline->busy_ = false;
            ctx->pipeline->worker_task_ = nullptr;
            delete ctx;
            vTaskDelete(NULL);
            return;
        }

        ESP_LOGI(TAG, "LLM success, response: %s", response_text.c_str());

        // Step 2: If using server TTS, skip TTS client and return text only
        if (!ctx->pipeline->config_.use_tts) {
            ESP_LOGI(TAG, "Server TTS mode, returning text only");
            ctx->callback(true, {}, 0, response_text, "");
            ctx->pipeline->busy_ = false;
            ctx->pipeline->worker_task_ = nullptr;
            delete ctx;
            vTaskDelete(NULL);
            return;
        }

        // Step 2: Call TTS with LLM response
        ctx->tts_client.Synthesize(response_text, [ctx, response_text](bool tts_success, std::vector<int16_t>&& pcm_data, int sample_rate, const std::string& tts_error) {
            if (!tts_success) {
                ESP_LOGE(TAG, "TTS failed: %s", tts_error.c_str());
                // Return the text response even if TTS failed
                ctx->callback(false, {}, sample_rate, response_text, tts_error);
            } else {
                ESP_LOGI(TAG, "TTS success, %u PCM samples", pcm_data.size());
                ctx->callback(true, std::move(pcm_data), sample_rate, response_text, "");
            }

            ctx->pipeline->busy_ = false;
            ctx->pipeline->worker_task_ = nullptr;
            delete ctx;
            vTaskDelete(NULL);
        });
    });
}
