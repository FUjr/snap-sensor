#ifndef AudioFeature_H
#define AudioFeature_H
#include <arduinoFFT.h>
#include <Arduino.h>
#include <math.h>
#include <kissfft/kiss_fft.h>
class AudioFeature{
    public:
        int Sample_rate;
        int stft_length;
        int stft_step;
        int nframes;
        int num_mel_bins;
        int upper_hz;
        int lower_hz;
        int audio_length;
        uint16_t* audio;
        float* _audio;
        float** output_matrix;
        float** stft_matrix;
        ~AudioFeature();
        AudioFeature(int Sample_rate,int stft_length,int stft_step, float** output_matrix,int num_mel_bins, int audio_length,int lower_hz, int upper_hz);
        void compute();
    protected:
        void _stft();
        void _mel();
        void _normallize_audio();
        void _generate_filter_bank();
        float *fft_input;
        float *fft_output;
        float **_filter_bank;
        int _audio_length;
};
#endif
