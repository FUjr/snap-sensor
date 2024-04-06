import scipy.io.wavfile as wav
import numpy as np
import socket
import struct
import time,os
# 配置参数
CHUNK = 1600
RATE = 16000

udp_port = 1234
save_path = os.sep.join(["saved_audio","score","%s_%s.wav"])
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', udp_port))
audio = []
SOF = b'\xEE\xEE\xEE\xEE'
EOF = b'\xFF\xFF\xFF\xFF'


def parse_header(data):
#     typedef struct{
#     float score;
#     int audio_length;
#     int audio_time;
#     int sent_time;
# } DataHeader;
    score = data[:4]
    audio_length = data[4:8]
    audio_time = data[8:12]
    sent_time = data[12:16]
    score = struct.unpack('f', score)[0]
    audio_length = struct.unpack('i', audio_length)[0]
    audio_time = struct.unpack('i', audio_time)[0]
    sent_time = struct.unpack('i', sent_time)[0]
    return score, audio_length, audio_time, sent_time

def parse_data(data):
    data = data[16:]
    audio = np.frombuffer(data, dtype=np.int16)
    return audio

class state(enumerate):
    WAIT_SOF = 0
    WAIT_EOF = 1


while True:
    datapack = b''
    data_state = state.WAIT_SOF
    data_recved = 0
    while (True):
        data, addr = sock.recvfrom(CHUNK)
        if data_state == state.WAIT_SOF:
            if data == SOF:
                data_state = state.WAIT_EOF
                print("Received SOF")
                continue
        elif data_state == state.WAIT_EOF:
            if data == EOF:
                print("Received EOF")
                break
            datapack += data
    data_recved = len(datapack)
    print(f"Received {data_recved} bytes")
    score, audio_length, audio_time, sent_time = parse_header(datapack)
    audio = parse_data(datapack)
    print(score,audio_length)
    wav.write(save_path % (score ,time.time() + (sent_time - audio_time)*0.001) , RATE, audio)
