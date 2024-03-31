#include "Tensor"
Tensor::Tensor(void *model_char, int tensor_arena_size){
    this->model_char = model_char;
    this->kTensorArenaSize = tensor_arena_size * 1024;
    _allocateTensors();
    _loadModel();
}

Tensor::~Tensor()
{
    free(this->tensor_arena);
    free(this->input);
    free(this->output);
}

void Tensor::infer()
{
    //print input.f
    // for (int i = 0; i < this->input->bytes / sizeof(float); i++){
    //     printf("input: %f\n", this->input->data.f[i]);
    // }
    memset(this->output->data.f, 0, this->output->bytes);
    if (this->interpreter->Invoke() != kTfLiteOk)
    {
        printf("Error invoking interpreter");
    }
    memset(this->input->data.f, 0, this->input->bytes);
}

void Tensor::_loadModel()
{
    this->model = tflite::GetModel(this->model_char);
    tflite::AllOpsResolver resolver;
    tflite::MicroInterpreter static_interpreter(this->model, resolver, this->tensor_arena, this->kTensorArenaSize, nullptr);
    if (static_interpreter.AllocateTensors() != kTfLiteOk)
    {
        printf("Error allocating tensors");
    }
    this->interpreter = &static_interpreter;
    this->input = this->interpreter->input(0);
    this->output = this->interpreter->output(0);
    printf("Interpreter allocated");
}

void Tensor::_allocateTensors()
{
    this->tensor_arena = (uint8_t *)heap_caps_malloc(this->kTensorArenaSize, MALLOC_CAP_SPIRAM);
}
