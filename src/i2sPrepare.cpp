#include "i2sPrepare"
namespace i2sPrepare
{
    void i2s_install(micModel mic, i2s_port_t i2s_num, int sampleRate)
    {
        i2s_config_t i2s_config = {
            .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = sampleRate,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = 0,
            .dma_buf_count = 8,
            .dma_buf_len = 64,
            .use_apll = false,
            .tx_desc_auto_clear = false,
            .fixed_mclk = 0};
        i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    }
    void i2s_set_pin(i2s_port_t i2s_num, int bck, int ws, int sd)
    {
        i2s_pin_config_t pin_config = {
            .bck_io_num = bck,
            .ws_io_num = ws,
            .data_out_num = -1,
            .data_in_num = sd};
        i2s_set_pin(i2s_num, &pin_config);
    }

}
