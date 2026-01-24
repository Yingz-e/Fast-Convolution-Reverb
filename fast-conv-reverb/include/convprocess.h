#ifndef CONVPROCESS_H
#define CONVPROCESS_H

#include <sndfile.h>
#include "libHybridConv.h"

// B-format转换系数
#define A_COEF 0.707106781f  // 1/√2
#define B_COEF 0.707106781f  // 1/√2
#define C_COEF 0.707106781f  // 1/√2
#define D_COEF 0.5f          // 1/2

// 音频文件检查函数
void check_audio_file(const char* filename);

// 主处理函数
int process_convolution(const char* input_file, const char* ir_file,
                       const char* output_file, int sflen, int mflen,
                       int lflen, int mode);

#endif // CONVPROCESS_H