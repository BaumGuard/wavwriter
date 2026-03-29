#ifndef WAV_WRITER_H
#define WAV_WRITER_H

#include <stdint.h>

/*-----------------------------------------------------------------*/

// SAMPLE TYPES
enum SampleTypes {
    // Signed integers
    S8, S16, S24, S32, S64,

    // Unsigned integers
    U8, U16, U24, U32, U64,

    // Floating point
    F32, F64
};

// WAV FILE HANDLE
typedef struct wav_file_t wav_file_t;


// SAMPLE
typedef union {
    // Unsigned integers
    uint8_t u8; uint16_t u16; uint32_t u24; uint32_t u32; uint64_t u64;

    // Signed integers
    int8_t i8; int16_t i16; int32_t s24; int32_t i32; int64_t i64;

    // Floating point
    float f32; double f64;
} wav_sample_t;


/*-----------------------------------------------------------------*/

/*
Open a new WAV file

Args:
 - wav_handle       : Pointer to the WAV file handle
 - file_path        : Path of the WAV file that should be created
 - channels         : Number of channels
 - sample_rate      : Sample rate in Hz
 - sample_format    : Data format of the sample (see enum SampleTypes)

Returns:
 -  0 : Success
 - -1 : Could not create wav_handle
 - -2 : Coult not create file
*/
int wavwriter_open(
    wav_file_t** wav_handle,
    char* file_path,
    uint16_t channels, uint32_t sample_rate, int sample_format
);

/*
Write a set of audio samples to the file

Args:
 - wav_handle       : Pointer to the WAV file handle
 - samples          : Array of samples to be written into the file
                      If the number of channels is larger than 1, the samples
                      must be present in an interleaved form in the array
                        e. g. for Stereo: LRLRLR...
 - n_samples        : Total number of samples for all channels in the array
*/
void wavwriter_write_samples(
    wav_file_t* wav_handle, wav_sample_t* samples, int n_samples
);

/*
Close and finalize the WAV file
IMPORTANT:  Make sure that this function is executed after all samples
            have been written into the WAV. Otherways the file will be
            corrupt because the file header will be incomplete!

Args:
 - wav_handle       : Pointer to the WAV file handle to be closed and freed
*/
void wavwriter_close( wav_file_t* wav_handle );

#endif
