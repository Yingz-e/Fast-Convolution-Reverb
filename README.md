# Fast Convolution Reverb

这是一个基于快速卷积的空间音频处理项目，可以将双声道音频通过B-format（Ambisonics）格式的脉冲响应进行处理，实现空间混响效果。

## 功能特点

- 支持双声道（立体声）音频输入
- 支持B-format（4通道）脉冲响应
- 可调节干湿比例
- 支持自动音频标准化处理（-0.3dB）
- 基于FFT的快速卷积算法
- 实时进度显示

## 环境要求

- Windows操作系统
- MinGW-w64 编译器
- GNU Make工具

## 依赖库

- libsndfile：用于音频文件读写
- FFTW3：用于快速傅里叶变换
- libHybridConv：用于快速卷积处理

## 编译方法

1. 确保已安装MinGW-w64和Make工具
2. 在项目根目录下运行：

```bash
mingw32-make clean
mingw32-make

## 使用方法
1. 准备音频文件：
   
   - 输入音频：双声道WAV格式
   - 脉冲响应：B-format（4通道）WAV格式
2. 修改main.cpp中的文件路径：
   
   ```cpp
   const std::string input_file  = "path/to/input.wav";
   const std::string ir_file     = "path/to/ir.wav";
   const std::string output_file = "path/to/output.wav";
```

3. 运行程序：

   ```bash
   .\fast_conv.exe
   ```

## 参数调整

- FRAME_SIZE ：处理帧大小，默认8192
- wetDryRatio ：干湿比例，范围0.0-1.0
- wGain ：W通道增益，默认0.7071
- xyGain ：XY通道增益，默认0.6
- zGain ：Z通道增益，默认0.0

## 注意事项

1. 确保所有必要的DLL文件在程序运行目录下：

   - libsndfile-1.dll
   - libfftw3f-3.dll
2. 输入音频必须是双声道WAV格式
3. 脉冲响应必须是B-format（4通道）WAV格式
4. 确保音频文件路径正确且有读写权限
5. 建议使用48kHz或96kHz采样率的音频文件

## 目录结构

```plaintext
fast-conv/
├── include/          # 头文件目录
├── lib/             # 库文件目录
├── src/             # 源代码目录
├── IR/              # 脉冲响应文件目录
├── music/           # 音频文件目录
├── main.cpp         # 主程序
├── Makefile         # 编译配置
└── README.md        # 项目说明
```
