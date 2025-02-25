#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreAudio/AudioHardware.h>
#include <CoreAudio/AudioHardwareBase.h>

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Set audio format parameters for the default output device.\n\n");
    printf("Options:\n");
    printf("  -r, --rate=RATE       Set the sample rate in KHz\n");
    printf("  -b, --bits=BITS       Set the bit depth (Usually 16, 20, or 24)\n");
    printf("  -c, --channels=NUM    Set the number of channels (Usually between 1 and 8, inclusive)\n");
    printf("  -h, --help            Display this help message\n");
    printf("\nExamples:\n");
    printf("  %s --rate=44100 --bits=16 --channels=2\n", program_name);
    printf("  %s -r 48000 -b 24 -c 8\n", program_name);
}

AudioDeviceID getDefaultAudioDevice() {
    OSStatus status;

    AudioObjectPropertyAddress defaultOutputDeviceAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    UInt32 dataSize;
    status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject,
                                            &defaultOutputDeviceAddress,
                                            0, NULL,
                                            &dataSize);
    if (status != noErr) {
        fprintf(stderr, "Error getting calling AudioObjectGetPropertyDataSize: %d\n", (int)status);
        exit(1);
    }

    AudioDeviceID defaultDevice;
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                        &defaultOutputDeviceAddress,
                                        0, NULL,
                                        &dataSize,
                                        &defaultDevice);
    if (status != noErr) {
        fprintf(stderr, "Error getting default output device: %d\n", (int)status);
        exit(1);
    }

    return defaultDevice;
}

// Set the stream format (including bit depth)
void setStreamFormat(AudioDeviceID audioDevice, Float64 sampleRate, UInt32 bitDepth, UInt32 channelCount) {
    OSStatus status;
    
    AudioObjectPropertyAddress streamFormatAddress = {
        kAudioStreamPropertyPhysicalFormat,
        kAudioObjectPropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    UInt32 dataSize = 0;
    status = AudioObjectGetPropertyDataSize(audioDevice,
                                            &streamFormatAddress,
                                            0, NULL,
                                            &dataSize);
    if (status != noErr) {
        fprintf(stderr, "Error getting calling AudioObjectGetPropertyDataSize: %d\n", (int)status);
        exit(1);
    }

    AudioStreamBasicDescription* format = (AudioStreamBasicDescription*)calloc(1, dataSize);
    if (!format) {
        fprintf(stderr, "Failed to allocate memory for stream format\n");
        exit(1);
    }

    // First get the current format
    status = AudioObjectGetPropertyData(audioDevice,
                                      &streamFormatAddress,
                                      0, NULL,
                                      &dataSize,
                                      format);
    if (status != noErr) {
        fprintf(stderr, "Error getting current stream format for modification: %d\n", (int)status);
        free(format);
        exit(1);
    }

    // If we are updating the sample rate, override the current value
    if (sampleRate > 0) {
        format->mSampleRate = sampleRate;
    }

    // If we are updating the channel count, override the current value
    if (channelCount > 0) {
        format->mChannelsPerFrame = channelCount;
    }

    // Modify the format with our desired bit depth
    format->mBitsPerChannel = bitDepth;
    format->mBytesPerFrame = (format->mBitsPerChannel / 8) * format->mChannelsPerFrame + format->mChannelsPerFrame;
    format->mBytesPerPacket = format->mBytesPerFrame * format->mFramesPerPacket;

    // Set the new format
    status = AudioObjectSetPropertyData(audioDevice,
                                      &streamFormatAddress,
                                      0, NULL,
                                      dataSize,
                                      format);
    if (status != noErr) {
        fprintf(stderr, "Error setting stream format: %d\n", (int)status);
        free(format);
        exit(1);
    }

    // Verify the new format was applied
    status = AudioObjectGetPropertyData(audioDevice,
                                      &streamFormatAddress,
                                      0, NULL,
                                      &dataSize,
                                      format);
    if (status != noErr) {
        fprintf(stderr, "Error verifying stream format application: %d\n", (int)status);
        free(format);
        exit(1);    
    }

    const char* warn1 = "Warning: ";
    const char* warn2 = " was not applied.\n   Value may be invalid for this device, or the device does not support the resulting format.\n";
    const char* warn3 = "   Desired: ";
    const char* warn4 = ", Actual: ";

    if (sampleRate > 0 && format->mSampleRate != sampleRate) {
        fprintf(stdout, "%sNew sample rate%s%s%.0f%s%.0f\n",
                warn1, warn2, warn3, sampleRate, warn4, format->mSampleRate);
    }

    if (bitDepth > 0 && format->mBitsPerChannel != bitDepth) {
        fprintf(stdout, "%sNew bit depth%s%s%d%s%d\n",
                warn1, warn2, warn3, bitDepth, warn4, format->mBitsPerChannel);
    }

    if (channelCount > 0 && format->mChannelsPerFrame != channelCount) {
        fprintf(stdout, "%sNew channel count%s%s%d%s%d\n",
                warn1, warn2, warn3, channelCount, warn4, format->mChannelsPerFrame);
    }

    free(format);
}

int main(int argc, char *argv[]) {
    Float64 sampleRate = 0.0;
    UInt32 bitDepth = 0;
    UInt32 channelCount = 0;

    AudioDeviceID defaultDevice = getDefaultAudioDevice();
    
    static struct option long_options[] = {
        {"rate",     required_argument, 0, 'r'},
        {"bits",     required_argument, 0, 'b'},
        {"channels", required_argument, 0, 'c'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "r:b:c:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'r': {
                char *endptr;
                double inputRate = strtod(optarg, &endptr);
                if (*endptr != '\0' || inputRate <= 0) {
                    fprintf(stderr, "Error: Sample rate must be a positive number\n");
                    return 1;
                }
                sampleRate = inputRate;
                break;
            }
            case 'b': {
                char *endptr;
                long bits = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || (bits != 16 && bits != 20 && bits != 24)) {
                    fprintf(stderr, "Error: Bit depth must be 16, 20, or 24\n");
                    return 1;
                }
                bitDepth = (UInt32)bits;
                break;
            }
            case 'c': {
                char *endptr;
                long channels = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || (channels < 1 || channels > 8)) {
                    fprintf(stderr, "Error: Channel count must be between 1 and 8, inclusive\n");
                    return 1;
                }
                channelCount = (UInt32)channels;
                break;
            }
            case 'h':
            case '?':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (sampleRate > 0.0 || bitDepth > 0 || channelCount > 0) {
        setStreamFormat(defaultDevice, sampleRate, bitDepth, channelCount);
    } else {
        fprintf(stderr, "No valid options provided.\n\n");
        print_usage(argv[0]);
        return 1;
    }

   return 0;
}