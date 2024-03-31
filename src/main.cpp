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
void setup()
{
  Serial.begin(115200);
  LoadModel loadmodel = LoadModel("/model_CNN.tflite");
  char* model = new char[loadmodel.model_size];
  memcpy(model, loadmodel.model, loadmodel.model_size);
  i2sPrepare::i2s_install(i2sPrepare::micModel::INMP441, I2S_NUM_0, 16000);
  i2sPrepare::i2s_set_pin(I2S_NUM_0, I2S_SCK, I2S_WS, I2S_SD);
  tensor = new Tensor(model, 1024);
  delete[] model;
  //homekit = new HomeKit();
  melSpectrogram = new AudioProcessor(1024,256,7680,16000,60,2000,6000,1);
}

void loop()
{
    unsigned long time_loop_start = millis();
    melSpectrogram->update();
    for (int i = 0; i < melSpectrogram->audioFeature->nframes; i++)
    {
      for (int j = 0; j < melSpectrogram->audioFeature->num_mel_bins; j++)
      {
        tensor->input->data.f[i * melSpectrogram->audioFeature->nframes + j] = melSpectrogram->audioFeature->output_matrix[i][j];

      }
    }
    tensor->infer();
    int output_length = tensor->output->bytes / sizeof(float);
    for (int j = 0; j < output_length; j++)
    {
      printf("output: %f\n", tensor->output->data.f[j]);
      if (tensor->output->data.f[j] > 0.95)
      {
        if (homekit->switchEvent != nullptr)
        {
          homekit->switchEvent->setVal(0);
        }
      }
    }
    unsigned long time_end = millis();
    //printf("Time total: %d\n", time_end - time_loop_start);
    // printf("Time get audio: %d\n", time_get_audio - time_loop_start);
    // printf("Time compute: %d\n", time_compute - time_get_audio);
    // printf("Time inferencing: %d\n", time_end - time_compute);
}
