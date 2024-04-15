#include "DataCollector"
DataCollector::DataCollector()
{
    this->audio.data = nullptr;
    this->raw_audio = nullptr;
    this->server = IPAddress(0, 0, 0, 0);
    this->port = 0;
    this->audio.length = 0;
    this->audio_time = 0;
}

DataCollector::~DataCollector()
{
    if (this->audio.data != nullptr)
    {
        free(this->audio.data);
    }
    if (this->raw_audio != nullptr)
    {
        free(this->raw_audio);
    }
}
void DataCollector::setServer(IPAddress server, uint16_t port)
{
    this->server = server;
    this->port = port;
}

void DataCollector::setScore(score_t score)
{
    this->score = score;
}

void DataCollector::setAudio(audioData audio)
{
    this->audio = audio;
    this->audio_time = millis();
    this->raw_audio = (int16_t *)malloc(sizeof(int16_t) * audio.length);
    memcpy(this->raw_audio, audio.data, sizeof(int16_t) * audio.length);
}

void DataCollector::send()
{

    AsyncUDP udp;
    if (!udp.connect(IPAddress(this->server), this->port))
    {
        printf("Cannot connect to server");
        return;
    }
    DataHeader header;
    header.score = this->score;
    header.audio_length = this->audio.length;
    header.audio_time = this->audio_time;
    header.sent_time = millis();
    u_int CHUNK = 300;
    int _SOF = 0xEEEEEEEE;
    int _EOF = 0xFFFFFFFF;
    udp.write((byte*)&_SOF, 4);
    udp.write((uint8_t *)&header, sizeof(DataHeader));
    for (u_int i = 0; i < this->audio.length; i += CHUNK)
    {
        udp.write((uint8_t *)&this->raw_audio[i], min(CHUNK, this->audio.length - i) * sizeof(int16_t));
    }
    udp.write((byte*)&_EOF, 4);
    free(this->raw_audio);
    this->raw_audio = nullptr;
    udp.close();
}

u_int min(u_int a, u_int b)
{
    return a < b ? a : b;
}
