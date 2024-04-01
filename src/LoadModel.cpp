#include "LoadModel"
namespace LoadModel
{

    ModelConfig loadModelConfig(String Model_filename,String Config_filename)
    {
        SPIFFS.begin();
        File file = SPIFFS.open(Config_filename, "r");
        if (!file)
        {
            Serial.println("Failed to open file for reading");
        }
        ModelConfig modelConfig;
        String config = file.readString();
        file.close();
        jparse_ctx_t jctx;
        char *model;  
        int num_frames;
        int num_mel_bins;
        int lower_frequency_limit;
        int upper_frequency_limit;
        int sample_rate;
        int fft_shift_length;
        int fft_step_length;
        float threshold;
        json_parse_start(&jctx, config.c_str(), config.length());
        json_obj_get_int(&jctx, "num_frames", &num_frames);
        json_obj_get_int(&jctx, "num_mel_bins", &num_mel_bins);
        json_obj_get_int(&jctx, "lower_frequency_limit", &lower_frequency_limit);
        json_obj_get_int(&jctx, "upper_frequency_limit", &upper_frequency_limit);
        json_obj_get_int(&jctx, "sample_rate", &sample_rate);
        json_obj_get_int(&jctx, "fft_shift_length", &fft_shift_length);
        json_obj_get_int(&jctx, "fft_step_length", &fft_step_length);
        json_obj_get_float(&jctx, "threshold", &threshold);
        json_parse_end(&jctx);
        File file2 = SPIFFS.open(Model_filename, "r");
        if (!file2)
        {
            Serial.println("Failed to open file for reading");
        }
        model = (char *)heap_caps_malloc(file2.size(), MALLOC_CAP_SPIRAM);
        file2.readBytes(model, file2.size());
        modelConfig.model = model;
        modelConfig.num_frames = num_frames;
        modelConfig.num_mel_bins = num_mel_bins;
        modelConfig.lower_frequency_limit = lower_frequency_limit;
        modelConfig.upper_frequency_limit = upper_frequency_limit;
        modelConfig.sample_rate = sample_rate;
        modelConfig.fft_shift_length = fft_shift_length;
        modelConfig.fft_step_length = fft_step_length;
        modelConfig.threshold = threshold;
        return modelConfig;
    }
}
