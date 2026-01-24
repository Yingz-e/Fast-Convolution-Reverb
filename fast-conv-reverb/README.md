# Fast Convolution Reverb

A spatial audio processing project based on fast convolution, which processes stereo audio through B-format (Ambisonics) impulse responses to achieve spatial reverberation effects.

## Features

- Supports stereo audio input
- Compatible with B-format (4-channel) impulse responses
- Adjustable wet/dry ratio
- Automatic audio normalization (-0.3dB)
- FFT-based fast convolution algorithm
- Real-time progress display

## Requirements

- Windows OS
- MinGW-w64 compiler
- GNU Make

## Dependencies

- libsndfile: Audio file I/O
- FFTW3: Fast Fourier Transform
- libHybridConv: Fast convolution processing

## Building

1. Ensure MinGW-w64 and Make are installed
2. Run in project root directory:
```bash
mingw32-make clean
mingw32-make
```

## Usage

1. Prepare audio files:
   - Input audio: Stereo WAV format
   - Impulse response: B-format (4-channel) WAV format

2. Modify file paths in main.cpp:
   ```cpp
   const std::string input_file  = "path/to/input.wav";
   const std::string ir_file     = "path/to/ir.wav";
   const std::string output_file = "path/to/output.wav";
   ```

3. Run the program:
   ```bash
   .\fast_conv.exe
   ```

## Parameters

- `FRAME_SIZE`: Processing frame size, default 8192
- `wetDryRatio`: Wet/dry ratio, range 0.0-1.0
- `wGain`: W channel gain, default 0.7071
- `xyGain`: XY channel gain, default 0.6
- `zGain`: Z channel gain, default 0.0

## Important Notes

1. Ensure required DLLs are in the program directory:
   - libsndfile-1.dll
   - libfftw3f-3.dll

2. Input audio must be stereo WAV format
3. Impulse response must be B-format (4-channel) WAV format
4. Verify audio file paths and permissions
5. Recommended sample rates: 48kHz or 96kHz

## Project Structure

```
fast-conv/
├── include/          # Header files
├── lib/             # Library files
├── src/             # Source files
├── IR/              # Impulse response files
├── music/           # Audio files
├── main.cpp         # Main program
├── Makefile         # Build configuration
└── README.md        # Project documentation
```

## License

This project is licensed under the MIT License.