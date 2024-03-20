
#include "AudioFeature.h"


// Hz to Mel conversion
float hz_to_mel(float hz)
{
    return 2595 * log10(1 + hz / 700);
}

// Mel to Hz conversion
float mel_to_hz(float mel)
{
    return 700 * (pow(10, mel / 2595) - 1);
}

template <typename T> T abs(T val){
    return val > 0 ? val : -val;
}

AudioFeature::AudioFeature(int Sample_rate,int stft_length,int stft_step, float** output_matrix,int num_mel_bins, int audio_length,int lower_hz, int upper_hz)
{

    
    this->Sample_rate = Sample_rate;
    this->stft_length = stft_length;
    this->stft_step = stft_step;
    this->output_matrix = output_matrix;
    this->num_mel_bins = num_mel_bins;
    this->audio_length = audio_length;
    this->lower_hz = lower_hz;
    this->upper_hz = upper_hz;
    this->nframes = (this->audio_length - this->stft_length) / this->stft_step + 1;
    this->fft_input = (float *)heap_caps_malloc(sizeof(float) * this->stft_length, MALLOC_CAP_SPIRAM);
    this->fft_output = (float *)heap_caps_malloc(sizeof(float) * this->stft_length, MALLOC_CAP_SPIRAM);
    this->stft_matrix = (float **)heap_caps_malloc(sizeof(float *) * this->nframes, MALLOC_CAP_SPIRAM);
    this->_audio = (float *)heap_caps_malloc(sizeof(float) * this->audio_length, MALLOC_CAP_SPIRAM);
    for (int i = 0; i < this->nframes; i++)
    {
        this->stft_matrix[i] = (float *)heap_caps_malloc(sizeof(float) * this->stft_length, MALLOC_CAP_SPIRAM);
        memset(this->stft_matrix[i], 0, sizeof(float) * this->stft_length);
    }
    this->output_matrix = (float **)heap_caps_malloc(sizeof(float *) * this->nframes, MALLOC_CAP_SPIRAM);
    for (int i = 0; i < this->nframes; i++)
    {
        this->output_matrix[i] = (float *)heap_caps_malloc(sizeof(float) * this->num_mel_bins, MALLOC_CAP_SPIRAM);
        memset(this->output_matrix[i], 0, sizeof(float) * this->num_mel_bins);
    }
    this->_filter_bank = (float **)heap_caps_malloc(sizeof(float *) * this->stft_length, MALLOC_CAP_SPIRAM);
    for (int i = 0; i < this->stft_length; i++)
    {
        this->_filter_bank[i] = (float *)heap_caps_malloc(sizeof(float) * this->num_mel_bins, MALLOC_CAP_SPIRAM);
        memset(this->_filter_bank[i], 0, sizeof(float) * this->num_mel_bins);
    }
}

void AudioFeature::compute()
{
    this->_normallize_audio();
    this->_stft();
    //print max audio,max _audio, stft_matrix
    float max_audio = 0;
    float max__audio = 0;
    float max_stft = 0;
    for (int i = 0; i < this->audio_length; i++)
    {
        if (this->audio[i] > max_audio)
        {
            max_audio = this->audio[i];
        }
        if (this->_audio[i] > max__audio)
        {
            max__audio = this->_audio[i];
        }
    }
    for (int i = 0; i < this->nframes; i++)
    {
        for (int j = 0; j < this->stft_length; j++)
        {
            if (this->stft_matrix[i][j] > max_stft)
            {
                max_stft = this->stft_matrix[i][j];
            }
        }
    }
    printf("max audio: %f, max _audio: %f, max stft: %f\n", max_audio, max__audio, max_stft);
    this->_mel();
}

void AudioFeature::_normallize_audio(){
    for (int i = 0; i < this->audio_length; i++){
        this->_audio[i] = (this->audio[i] - 32768) / 32768.0 + 0.0001;
    }
}

void AudioFeature::_stft()
{
    ArduinoFFT<float> FFT = ArduinoFFT<float>(this->fft_input, this->fft_output, this->stft_length, this->Sample_rate, 1);
    for (int i = 0; i < this->nframes; i++)
    {
        memset(this->stft_matrix[i], 0, sizeof(float) * this->stft_length);
        memset(this->fft_output, 0, sizeof(float) * this->stft_length);
        memset(this->fft_input, 0, sizeof(float) * this->stft_length);
        if (i * this->stft_step + this->stft_length <= this->audio_length)
        {
             memcpy(this->fft_input, this->_audio + i * this->stft_step, sizeof(float) * this->stft_length);
        }
        else
        {
             int cpy_len;
             cpy_len = this->audio_length - i * this->stft_step;
             memcpy(this->fft_input, this->_audio +  i * this->stft_step, sizeof(float) * cpy_len);
        }
        // for (int j = 0; j < this->stft_length; j++)
        // {
        //     if (i * this->stft_step + j < this->audio_length)
        //         this->fft_input[j] = this->_audio[i * this->stft_step + j];
        // }
        FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
        FFT.compute(FFTDirection::Forward);
        printf("majorPeak %f" , FFT.majorPeak());
        //FFT.complexToMagnitude();
        memcpy(this->stft_matrix[i], this->fft_output, sizeof(float) * this->stft_length);
    }
}

void AudioFeature::_mel()
{
    this->_generate_filter_bank();
    for (int i = 0; i < this->nframes; i++)
    {
        for (int j = 0; j < this->num_mel_bins; j++)
        {
            float sum = 0;
            for (int k = 0; k < this->stft_length; k++)
            {
                sum += abs(this->stft_matrix[i][k])* this->_filter_bank[k][j];
            }
            //sum = log(sum + 0.0001);
            this->output_matrix[i][j] = sum;

        }
    }
    float output_max = 0;
    for (int i = 0; i < this->nframes; i++)
    {
        for (int j = 0; j < this->num_mel_bins; j++)
        {
            if (this->output_matrix[i][j] > output_max)
            {
                output_max = this->output_matrix[i][j];
            }
        }
    }
    printf("output max: %f\n", output_max);
}

void AudioFeature::_generate_filter_bank()
{
    
    float mel_upper;
    float mel_lower;
    float mel_bins[this->num_mel_bins + 2];
    float hz_bins[this->num_mel_bins + 2];
    mel_upper = hz_to_mel(upper_hz);
    mel_lower = hz_to_mel(lower_hz);
    
    // Compute mel_points
    for (int i = 0; i < this->num_mel_bins + 2; i++)
    {
        mel_bins[i] = mel_lower + (mel_upper - mel_lower) / (this->num_mel_bins + 1) * i;
        hz_bins[i] = mel_to_hz(mel_bins[i]);
    }
    // Compute filter_bank
    for (int i = 0; i < this->stft_length; i++)
    {
        for (int j = 0; j < this->num_mel_bins; j++)
        {
            float lower = hz_bins[j];
            float center = hz_bins[j + 1];
            float upper = hz_bins[j + 2];
            float hz = (this->Sample_rate / 2) / (this->stft_length - 1) * i;
            if (hz >= lower && hz <= upper)
            {
                if (hz < center)
                {
                    this->_filter_bank[i][j] = (hz - lower) / (center - lower);
                }
                else
                {
                    this->_filter_bank[i][j] = (upper - hz) / (upper - center);
                }
            }
        }
    }
}

AudioFeature::~AudioFeature()
{
    for (int i = 0; i < this->nframes; i++)
    {
        free(this->stft_matrix[i]);
    }
    free(this->stft_matrix);
    free(this->fft_input);
    free(this->fft_output);
    free(this->_audio);
    for (int i = 0; i < this->nframes; i++)
    {
        free(this->output_matrix[i]);
    }
    free(this->output_matrix);
    for (int i = 0; i < this->stft_length; i++)
    {
        free(this->_filter_bank[i]);
    }
    free(this->_filter_bank);
}
