#include "include\libHybridConv_eapo.h"  

#include <iostream>
#include <vector>
#include "include\sndfile.h"
#include "include\fftw3.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
// 固定输入、IR 和输出文件路径
const std::string input_file  = "E:\\algorithm\\fast-conv\\music\\LiSA96.wav";
const std::string ir_file     = "E:\\algorithm\\fast-conv\\IR\\s1r2.wav";
const std::string output_file = "E:\\algorithm\\fast-conv\\music\\output_Bformat3.wav";

// 每个处理块的采样点数
constexpr unsigned FRAME_SIZE = 8192;

// 读取 WAV 文件数据到 vector
bool readWavFile(const std::string &filename, std::vector<float> &data, SF_INFO &sfinfo) {
    SNDFILE *file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Error opening file: " << filename << " - " << sf_strerror(file) << std::endl;
        return false;
    }
    data.resize(sfinfo.frames * sfinfo.channels);
    sf_count_t numFrames = sf_readf_float(file, data.data(), sfinfo.frames);
    if (numFrames != sfinfo.frames) {
        std::cerr << "Warning: read " << numFrames << " frames, expected " << sfinfo.frames << std::endl;
    }
    sf_close(file);
    return true;
}

// 写 WAV 文件数据
bool writeWavFile(const std::string &filename, const std::vector<float> &data, const SF_INFO &sfinfo) {
    SNDFILE *file = sf_open(filename.c_str(), SFM_WRITE, const_cast<SF_INFO*>(&sfinfo));
    if (!file) {
        std::cerr << "Error writing file: " << filename << " - " << sf_strerror(file) << std::endl;
        return false;
    }
    sf_count_t written = sf_writef_float(file, data.data(), sfinfo.frames);
    if (written != sfinfo.frames) {
        std::cerr << "Error: written frames (" << written << ") != expected frames (" << sfinfo.frames << ")" << std::endl;
    }
    sf_close(file);
    return true;
}

int main() {
    // 定义 SF_INFO 结构体
    SF_INFO inputInfo = {0}, irInfo = {0};

    // 添加干湿比参数（0.0表示全干信号，1.0表示全湿信号）
    const float wetDryRatio = 0.5f;  // 70%湿信号，30%干信号
    
    std::vector<float> inputData, irInterleaved;
    // 读取输入音频（期望为双声道）
    if (!readWavFile(input_file, inputData, inputInfo)) {
        return EXIT_FAILURE;
    }
    
    std::cout << "成功读取输入文件: " << input_file << std::endl;
    std::cout << "采样率: " << inputInfo.samplerate << " Hz" << std::endl;
    std::cout << "通道数: " << inputInfo.channels << std::endl;
    std::cout << "帧数: " << inputInfo.frames << std::endl;
    
    if (inputInfo.channels != 2) {
        std::cerr << "Error: 输入音频必须是双声道 (2 channels)" << std::endl;
        return EXIT_FAILURE;
    }

    // 读取脉冲响应文件（期望为 B-format，即 4 通道）
    if (!readWavFile(ir_file, irInterleaved, irInfo)) {
        return EXIT_FAILURE;
    }
    
    std::cout << "成功读取IR文件: " << ir_file << std::endl;
    std::cout << "IR采样率: " << irInfo.samplerate << " Hz" << std::endl;
    std::cout << "IR通道数: " << irInfo.channels << std::endl;
    std::cout << "IR帧数: " << irInfo.frames << std::endl;
    
    if (irInfo.channels != 4) {
        std::cerr << "Error: 脉冲响应文件必须是 B-format (4 channels)" << std::endl;
        return EXIT_FAILURE;
    }

    // 计算帧数
    unsigned inputFrameCount = inputInfo.frames; // 输入文件的帧数
    unsigned irFrameCount = irInfo.frames;       // IR 文件的帧数

    // 拆分 IR 数据，使用所有B格式通道进行更复杂的空间处理
    std::vector<float> irW(irFrameCount);
    std::vector<float> irX(irFrameCount);
    std::vector<float> irY(irFrameCount);
    std::vector<float> irZ(irFrameCount);
    
    for (unsigned i = 0; i < irFrameCount; i++) {
        // B格式通道: W(全向), X(前/后), Y(左/右), Z(上/下)
        irW[i] = irInterleaved[i * irInfo.channels + 0];
        irX[i] = irInterleaved[i * irInfo.channels + 1];
        irY[i] = irInterleaved[i * irInfo.channels + 2];
        irZ[i] = irInterleaved[i * irInfo.channels + 3];
    }
    
    // 创建左右耳的IR，使用B格式的线性组合来模拟双耳效果
    // 使用基于扬声器方向角的解码矩阵
    std::vector<float> irLeft(irFrameCount);
    std::vector<float> irRight(irFrameCount);
    
    // 定义扬声器角度（以弧度为单位）
    const float theta_L = 30.0f * 3.1415926 / 180.0f;  // 左声道 +30度
    const float theta_R = -30.0f * 3.1415926 / 180.0f; // 右声道 -30度
    
    // 计算解码系数
    const float cos_theta_L = std::cos(theta_L);
    const float sin_theta_L = std::sin(theta_L);
    const float cos_theta_R = std::cos(theta_R);
    const float sin_theta_R = std::sin(theta_R);
    
    // 可选的增益调整系数
    const float wGain = 0.7071f; // W通道增益，通常为1/sqrt(2)
    const float xyGain = 0.6f;   // X和Y通道增益
    const float zGain = 0.0f;    // Z通道增益（通常在水平面布局中不使用）
    
    for (unsigned i = 0; i < irFrameCount; i++) {
        // 使用解码矩阵计算左右声道
        // L = W + X*cos(θL) + Y*sin(θL)
        irLeft[i] = wGain * irW[i] + 
                    xyGain * irX[i] * cos_theta_L + 
                    xyGain * irY[i] * sin_theta_L + 
                    zGain * irZ[i];
        
        // R = W + X*cos(θR) + Y*sin(θR)
        irRight[i] = wGain * irW[i] + 
                     xyGain * irX[i] * cos_theta_R + 
                     xyGain * irY[i] * sin_theta_R + 
                     zGain * irZ[i];
    }
    
    // 初始化卷积滤波器（每个通道一个 HConvSingle 实例）
    HConvSingle filters[2];
    hcInitSingle(&filters[0], irLeft.data(), irFrameCount, FRAME_SIZE, 1);
    hcInitSingle(&filters[1], irRight.data(), irFrameCount, FRAME_SIZE, 1);

    // 为输出数据分配空间，与输入数据大小一致（双声道 interleaved）
    std::vector<float> outputData(inputData.size(), 0.0f);

    // 创建临时缓冲区用于处理单声道数据
    std::vector<float> inputBufferLeft(FRAME_SIZE);
    std::vector<float> inputBufferRight(FRAME_SIZE);
    std::vector<float> outputBufferLeft(FRAME_SIZE);
    std::vector<float> outputBufferRight(FRAME_SIZE);

    // 按帧处理数据，每个帧包含 FRAME_SIZE 个样本
    std::cout << "开始处理卷积..." << std::endl;
    unsigned totalFrames = inputFrameCount;
    for (unsigned frameIdx = 0; frameIdx < totalFrames; frameIdx += FRAME_SIZE) {
        // 计算当前帧的实际处理样本数（最后一帧可能不足 FRAME_SIZE）
        unsigned curFrameSize = FRAME_SIZE;
        if (frameIdx + FRAME_SIZE > totalFrames) {
            curFrameSize = totalFrames - frameIdx;
        }

        // 从交错格式中提取左右声道数据
        for (unsigned i = 0; i < curFrameSize; i++) {
            inputBufferLeft[i] = inputData[(frameIdx + i) * 2];
            inputBufferRight[i] = inputData[(frameIdx + i) * 2 + 1];
        }
        
        // 如果是最后一帧且不足FRAME_SIZE，将剩余部分填充为0
        if (curFrameSize < FRAME_SIZE) {
            for (unsigned i = curFrameSize; i < FRAME_SIZE; i++) {
                inputBufferLeft[i] = 0.0f;
                inputBufferRight[i] = 0.0f;
            }
        }

        // 处理左声道
        hcPutSingle(&filters[0], inputBufferLeft.data());
        hcProcessSingle(&filters[0]);
        hcGetSingle(&filters[0], outputBufferLeft.data());

        // 处理右声道
        hcPutSingle(&filters[1], inputBufferRight.data());
        hcProcessSingle(&filters[1]);
        hcGetSingle(&filters[1], outputBufferRight.data());

        // 将处理后的左右声道数据合并回交错格式，并应用干湿比
        for (unsigned i = 0; i < curFrameSize; i++) {
            // 应用干湿比混合
            float dryLeftSample = inputBufferLeft[i];
            float dryRightSample = inputBufferRight[i];
            float wetLeftSample = outputBufferLeft[i];
            float wetRightSample = outputBufferRight[i];
            
            // 混合干湿信号
            outputData[(frameIdx + i) * 2] = (1.0f - wetDryRatio) * dryLeftSample + wetDryRatio * wetLeftSample;
            outputData[(frameIdx + i) * 2 + 1] = (1.0f - wetDryRatio) * dryRightSample + wetDryRatio * wetRightSample;
        }
        
        // 每处理1000帧显示一次进度
        if (frameIdx % (FRAME_SIZE * 1000) == 0) {
            std::cout << "已处理: " << frameIdx / FRAME_SIZE << " 帧，总计: " 
                      << totalFrames / FRAME_SIZE << " 帧 (" 
                      << (frameIdx * 100 / totalFrames) << "%)" << std::endl;
        }
    }

    // 释放卷积滤波器资源
    hcCloseSingle(&filters[0]);
    hcCloseSingle(&filters[1]);

    // 检查输出数据是否有效
    bool hasNonZero = false;
    float maxOutputValue = 0.0f;
    
    // 查找整个输出数据中的最大绝对值
    for (size_t i = 0; i < outputData.size(); i++) {
        float absValue = std::abs(outputData[i]);
        if (absValue > maxOutputValue) {
            maxOutputValue = absValue;
            hasNonZero = true;
        }
    }
    
    std::cout << "输出数据" << (hasNonZero ? "包含" : "不包含") << "非零值" << std::endl;
    if (hasNonZero) {
        std::cout << "最大绝对值: " << maxOutputValue << std::endl;
        
        // 应用-0.3dB的标准化处理
        // -0.3dB对应的增益系数约为0.9659
        const float targetGain = 0.9659f;
        
        // 计算需要的缩放因子，确保最大值不超过targetGain
        float scaleFactor = 1.0f;
        if (maxOutputValue > targetGain) {
            scaleFactor = targetGain / maxOutputValue;
            std::cout << "应用标准化处理，缩放因子: " << scaleFactor << std::endl;
            
            // 对所有样本应用缩放
            for (size_t i = 0; i < outputData.size(); i++) {
                outputData[i] *= scaleFactor;
            }
        } else {
            std::cout << "音频最大值已经低于-0.3dB阈值，无需标准化" << std::endl;
        }
    }

    // 写入输出文件
    std::cout << "正在写入输出文件: " << output_file << std::endl;
    
    // 创建一个新的SF_INFO结构体用于输出
    SF_INFO outputInfo = inputInfo;
    
    SNDFILE *file = sf_open(output_file.c_str(), SFM_WRITE, &outputInfo);
    if (!file) {
        std::cerr << "Error writing file: " << output_file << " - " << sf_strerror(file) << std::endl;
        return EXIT_FAILURE;
    }
    
    sf_count_t written = sf_writef_float(file, outputData.data(), inputInfo.frames);
    if (written != inputInfo.frames) {
        std::cerr << "Error: written frames (" << written << ") != expected frames (" << inputInfo.frames << ")" << std::endl;
    } else {
        std::cout << "成功写入 " << written << " 帧数据" << std::endl;
    }
    sf_close(file);

    std::cout << "卷积混响处理完成，输出文件保存到: " << output_file << std::endl;
    
    return EXIT_SUCCESS;
}
