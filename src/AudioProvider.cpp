#include "AudioProvider"
#include "driver/i2s.h"



template <typename T>
AudioProvider<T>::AudioProvider(AudioProviderCFG<T> *cfg)
{
    this->cfg = cfg;
    createAudioChain();
}

template <typename T>
AudioProvider<T>::~AudioProvider()
{
    AudioChain<T> *current = this->cfg->head;
    for (int i = 0; i < this->cfg->chainLength; i++)
    {
        AudioChain<T> *temp = current;
        current = current->next;
        free(temp);
    }
    free(current);
}

template <typename T>
void AudioProvider<T>::createAudioChain()
{

    int chainLength;
    AudioChain<T> *head;
    AudioChain<T> *current;
    chainLength = this->cfg->chainLength;
    head = (AudioChain<T> *)this->_malloc_s(sizeof(AudioChain<T>));
    current = head;
    for (int i = 0; i < chainLength; i++)
    {
        current->audioMetadata = 0;
        current->next = (AudioChain<T> *)this->_malloc_s(sizeof(AudioChain<T>));
        current->next->prev = current;
        current = current->next;
    }
    head->prev = current;
    current->next = head;
    this->cfg->head = head;
    this->cfg->current = head;
}

template <typename T>
void AudioProvider<T>::recordAudio()
{
    switch (this->cfg->srcCfg->source)
    {
    case AudioSource::ADC:
        //_recordADC();
        break;
    case AudioSource::I2S:

        _recordI2S();
        break;
    case AudioSource::ARRAY:
        _recordARRAY();
        break;
    case AudioSource::FILE:
        //_recordFILE();
        break;
    default:
        break;
    }
}

template <typename T>
void AudioProvider<T>::_recordARRAY()
{
    int fakeWating = 1000 * this->cfg->srcCfg->chunkSize / this->cfg->srcCfg->sampleRate;
    int chunk = 0;
    for (int i = 0; i < this->cfg->srcCfg->sourceArrayLength; i++)
    {
        if (this->cfg->normalize)
        {
            float max_value;
            // bool is_unsign;
            // max_value,is_unsign = AudioProvider<T>::typemaxvalue();
            // float one_n = 1 / max_value ? is_unsign : 2 / max_value;
            // this->cfg->current->audioMetadata = this->cfg->srcCfg->sourceArray[i] / one_n;
        }
        else
        {
            this->cfg->current->audioMetadata = this->cfg->srcCfg->sourceArray[i];
        }
        this->cfg->current = this->cfg->current->next;
        chunk++;
        if (chunk >= this->cfg->srcCfg->chunkSize)
        {
            chunk = 0;
            delay(fakeWating);
        }
    }
}

template <typename T>
void AudioProvider<T>::_recordI2S()
{
    size_t bytesRead = 0;
    uint8_t buffer32[8 * 4] = {0};
    i2s_read(I2S_NUM_0, &buffer32, sizeof(buffer32), &bytesRead, 100);
    int samplesRead = bytesRead / 4;
    int16_t buffer16[8] = {0};
    for (int i = 0; i < samplesRead; i++)
    {
        uint8_t mid = buffer32[i * 4 + 2];
        uint8_t msb = buffer32[i * 4 + 3];
        uint16_t raw = (((uint32_t)msb) << 8) + ((uint32_t)mid);
        memcpy(&buffer16[i], &raw, sizeof(raw)); // Copy so sign bits aren't interfered with somehow.
        if (this->cfg->normalize)
        {
            float one_n = 1 / 32768.0;
            this->cfg->current->audioMetadata = buffer16[i] * one_n;
        }
        else
        {
            this->cfg->current->audioMetadata = buffer16[i];
        }
        //this->cfg->current->audioMetadata = buffer16[i];
        this->cfg->current = this->cfg->current->next;
    }
}
template <typename T>
void *AudioProvider<T>::_malloc_s(size_t size)
{
    if (this->cfg->usePSRAM)
    {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }
    else
    {
        return malloc(size);
    }
}

template <typename T>
void AudioProvider<T>::pool(int times)
{
    if (times == -1)
    {
        while (true)
        {
            int start_time = millis();
            recordAudio();
            int end_time = millis();
        }
    }
    for (int i = 0; i < times; i++)
    {
        recordAudio();
    }
}

template <typename T>
void AudioProvider<T>::autoPool()
{
    TaskHandle_t autoPoolTask;
    xTaskCreate(AudioProvider<T>::poolWrapper, "autoPoolTask", 1024 * 8, this, 5, &autoPoolTask);
}

template <typename T>
void AudioProvider<T>::poolWrapper(void *arg)
{
    AudioProvider<T> *instance = (AudioProvider<T> *)arg;
    instance->pool(-1);
}

// template <typename T>
// float AudioProvider<T>::typemaxvalue(void *arg){
//     bool is_unsign = 0;
//     T all_ones = ~T(0);
//     float max_value = (all_ones > 0) ? (T)(-1) ^ (T)(1 << (sizeof(T) * 8 - 1)) : all_ones;
//     is_unsign = 1 ? all_ones > 0 : 0;
//     return (max_value,is_unsign);
// }
