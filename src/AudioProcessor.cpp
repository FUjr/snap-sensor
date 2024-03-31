#include "AudioProcessor"

void hann_window(float *window, int length)
{
    for (int i = 0; i < length; i++)
    {
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (length - 1)));
    }
}

AudioProcessor::AudioProcessor(int fft_length,int fft_step,int audio_len,int sample_rate,int num_mel_bins,int lower_hz,int upper_hz,bool usePSRAM){
    this->fft_length = fft_length;
    this->fft_step = fft_step;
    this->audio_len = audio_len;
    this->sample_rate = sample_rate;
    this->num_mel_bins = num_mel_bins;
    this->upper_hz = upper_hz;
    this->lower_hz = lower_hz;
    this->usePSRAM = usePSRAM;
    AudioSourceCFG<float> *srcCfg;
    srcCfg = (AudioSourceCFG<float> *)this->_malloc_s(sizeof(AudioSourceCFG<float>));
    srcCfg->source = AudioSource::I2S;
    // AudioProvider config
    AudioProviderCFG<float> *cfg;
    cfg = (AudioProviderCFG<float> *)this->_malloc_s(sizeof(AudioProviderCFG<float>));
    cfg->chainLength = int(audio_len * 1.5);
    cfg->normalize = 1;
    cfg->usePSRAM = this->usePSRAM;
    cfg->srcCfg = srcCfg;
    this->_audioProvider = new AudioProvider<float>(cfg);
    this->_audioProvider->autoPool();
    // audio feature config
    this->audioFeature = new AudioFeature(sample_rate, fft_length, fft_step, num_mel_bins, audio_len, lower_hz, upper_hz);
    this->window = (float *)this->_malloc_s(sizeof(float) * this->audio_len);
    hann_window(this->window, this->audio_len);
}

void AudioProcessor::update(){
    memset(this->audioFeature->_audio, 0, this->audio_len * sizeof(float));
    this->ptr = this->_audioProvider->cfg->current;
    for (int i = 0; i < this->audio_len; i++)
    {
        this->audioFeature->_audio[this->audio_len - i] = this->ptr->audioMetadata * this->window[i];
        this->ptr = this->ptr->prev;
    }
    this->audioFeature->compute();
}

void *AudioProcessor::_malloc_s(size_t size)
{
    if (this->usePSRAM)
    {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }
    else
    {
        return malloc(size);
    }
}

AudioProcessor::~AudioProcessor(){
    delete this->audioFeature;
    delete this->_audioProvider;
    free(this->window);
}
