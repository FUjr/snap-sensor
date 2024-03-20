#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "Arduino.h"
#include "CNN_Model.h"
#include "audio.h"
// #include "sample.h"
#include <esp_heap_caps.h>
// #include <I2S.h>
#include "AudioFeature.h"

const tflite::Model *model = nullptr;


tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

constexpr int kTensorArenaSize = 1024 * 1024;
uint8_t* tensor_arena = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM);




void setup(){
  Serial.begin(115200);
  model  = tflite::GetModel(model_0320_1822_model_CNN_tflite);
  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, nullptr);
  if (static_interpreter.AllocateTensors() != kTfLiteOk) {
    Serial.println("Error allocating tensors");
    return;
  }
  interpreter = &static_interpreter;
  Serial.println("Interpreter allocated");
  input = interpreter->input(0);
  output = interpreter->output(0);
  Serial.println("input and output allocated");
  
  //I2S.setAllPins(5,7,4,6,15);
  //I2S.begin(I2S_PHILIPS_MODE, 16000, 16);
}



void loop(){
  //audio feature
  printf("---Start---\n");
  float** output_metrix;
  int stft_length = 1024;
  int stft_step = 512;
  int CHUNK = 15360;
  int sample_rate = 16000;
  uint16_t* buffer = (uint16_t*)heap_caps_malloc(CHUNK * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
  int num_mel_bins = 39;
  int upper_hz = 5500;
  int lower_hz = 1500;
  //int res = I2S.read(buffer, CHUNK);
  for (int i = 0; i < CHUNK; i++){
     buffer[i] = data[i];
  }
  AudioFeature audioFeature(sample_rate, stft_length, stft_step, output_metrix,num_mel_bins,CHUNK,lower_hz, upper_hz);
  audioFeature.audio = buffer;
  audioFeature.compute();
  heap_caps_free(buffer);

  //inferencing
  //reset output
  memset(output->data.f, 0, output->bytes);
  printf("before void loop(): %d\n", xPortGetFreeHeapSize());
  unsigned long time_start = millis();
  for (int i = 0; i < audioFeature.nframes; i++){
    for (int j = 0; j < audioFeature.num_mel_bins; j++){
      input->data.f[i*audioFeature.nframes + j] = audioFeature.output_matrix[i][j];
    }
  }
  printf("before invoke\n");
  interpreter->Invoke();
  int output_length = output->bytes/sizeof(float);
  for (int j = 0; j < output_length; j++){
    printf("output: %f\n", output->data.f[j]);
  }
  
  unsigned long time_end = millis();
  printf("Time : %d\n", time_end - time_start);
  delay(1000);
  printf("---Finish---\n");
}
