#pragma once

#include <string>
#include <functional>
#include <http.h>

class LlmClient {
public:
    struct Config {
        std::string api_endpoint;
        std::string api_key;
        std::string model = "deepseek-chat";
        std::string system_prompt = "You are a helpful voice assistant. Keep responses short and conversational.";
        int max_tokens = 256;
        float temperature = 0.7f;
    };

    using Callback = std::function<void(bool success, const std::string& response_text, const std::string& error_message)>;

    LlmClient();
    ~LlmClient();

    void SetConfig(const Config& config);
    void Chat(const std::string& user_text, Callback callback);

private:
    Config config_;
};
