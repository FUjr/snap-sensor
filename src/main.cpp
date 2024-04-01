#include "Arduino.h"
#include "CNN_Model.h"
#include <esp_heap_caps.h>
#include "i2sPrepare"
#include "Tensor"
#include "HomeKit"
#include "AudioProcessor"
#include "LoadModel"


#define I2S_SD 12
#define I2S_SCK 11
#define I2S_WS 10

static HomeKit *homekit = nullptr;
static Tensor *tensor = nullptr;
static AudioProcessor *melSpectrogram;
static float threshold;
void setup()
{
  Serial.begin(115200);
  i2sPrepare::i2s_install(i2sPrepare::micModel::INMP441, I2S_NUM_0, 16000);
  i2sPrepare::i2s_set_pin(I2S_NUM_0, I2S_SCK, I2S_WS, I2S_SD);
  homekit = new HomeKit();
  LoadModel::ModelConfig loadmodel  = LoadModel::loadModelConfig("/model_CNN.tflite","/model_config.json");
  threshold = loadmodel.threshold;
  tensor = new Tensor(loadmodel.model, 1024, false);
  melSpectrogram = new AudioProcessor(
    loadmodel.fft_shift_length,
    loadmodel.fft_step_length,
    loadmodel.num_frames,
    loadmodel.sample_rate,
    loadmodel.num_mel_bins,
    loadmodel.lower_frequency_limit,
    loadmodel.upper_frequency_limit,
    1
    );
}

void loop()
{
    unsigned long time_loop_start = millis();
    melSpectrogram->update();
    unsigned long time_get_audio_feat = millis();
    for (int i = 0; i < melSpectrogram->audioFeature->nframes; i++)
    {
      for (int j = 0; j < melSpectrogram->audioFeature->num_mel_bins; j++)
      {
        tensor->input->data.f[i * melSpectrogram->audioFeature->nframes + j] = melSpectrogram->audioFeature->output_matrix[i][j];
      }
    }
    unsigned long time_copy_audio = millis();
    tensor->infer();
    unsigned long time_end_infering = millis();
    int output_length = tensor->output->bytes / sizeof(float);
    for (int j = 0; j < output_length; j++)
    {
      printf("output: %f\n", tensor->output->data.f[j]);
      if (tensor->output->data.f[j] > threshold)
      {
        if (homekit->switchEvent != nullptr)
        {
          homekit->switchEvent->setVal(0);
          delay(512+ time_loop_start - time_end_infering );
        }
      }
    }
    unsigned long time_end = millis();
    // printf("Time total: %d\n", time_end - time_loop_start);
    // printf("Time get audio feat: %d\n", time_get_audio_feat - time_loop_start);
    // printf("Time copy audio: %d\n", time_copy_audio - time_get_audio_feat);
    // printf("Time inferencing: %d\n", time_end - time_copy_audio);
}
