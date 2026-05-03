#pragma once

#include <string>
#include <functional>
#include <vector>
#include <http.h>

class TtsClient {
public:
    struct Config {
        std::string api_endpoint;
        std::string api_key;
        std::string model = "tts-1";
        std::string voice = "alloy";
        int sample_rate = 24000;
        std::string response_format = "pcm";
    };

    // callback: (success, pcm_data, sample_rate, error_message)
    using Callback = std::function<void(bool success, std::vector<int16_t>&& pcm_data, int sample_rate, const std::string& error_message)>;

    TtsClient();
    ~TtsClient();

    void SetConfig(const Config& config);
    void Synthesize(const std::string& text, Callback callback);

private:
    Config config_;
};
