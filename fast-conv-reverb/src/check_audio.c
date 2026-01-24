#include <sndfile.h>
#include <stdio.h>

void check_audio_file(const char* filename) {
    SF_INFO sfinfo = {0};
    SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
    if (!file) {
        printf("无法打开文件 %s\n", filename);
        printf("错误信息: %s\n", sf_strerror(NULL));
        return;
    }
    
    printf("文件: %s\n", filename);
    printf("通道数: %d\n", sfinfo.channels);
    printf("采样率: %d\n", sfinfo.samplerate);
    printf("总帧数: %lld\n", sfinfo.frames);
    printf("格式: %d\n", sfinfo.format);
    
    sf_close(file);
}