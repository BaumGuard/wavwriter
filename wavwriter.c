#include "wavwriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INT24_MAX 8388607

/*-----------------------------------------------------------------*/

typedef struct {
    char riff [4];
    uint32_t file_size;
    char wave [4];

    char fmt [4];
    uint32_t fmt_header_len;
    uint16_t data_format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t bytes_per_second;
    uint16_t block_align;
    uint16_t bits_per_sample;

    char data_header [4];
    uint32_t data_block_len;
} WAV_Header;

struct wav_file_t {
    FILE* wav_file;
    WAV_Header wav_header;
    uint32_t n_samples_written;
    int sample_format;
    bool little_endian;
};

/*-----------------------------------------------------------------*/

bool _system_is_little_endian () {
    union endianness_test {
        uint16_t u16;
        uint8_t u8;
    };
    union endianness_test sample;
    sample.u16 = 0x1122;

    if ( sample.u8 == 0x22 ) {
        return true;
    }
    return false;
} /* _system_is_little_endian() */

void _switch_endianness( wav_sample_t* sample, int n_bytes ) {
    for ( int i = 0; i < n_bytes; i++ ) {
        sample[n_bytes-i-1] = sample[i];
    }
} /* _switch_endianness() */

/*-----------------------------------------------------------------*/

int wavwriter_open (
    wav_file_t** wav_handle,
    char* file_path,
    uint16_t channels, uint32_t sample_rate, int sample_format
) {
    (*wav_handle) = (wav_file_t*) malloc( sizeof(struct wav_file_t) );
    if ( !(*wav_handle) ) {
        return -1;
    }

    (*wav_handle)->little_endian = _system_is_little_endian();

    (*wav_handle)->wav_file = fopen( file_path, "wb" );
    if ( !(*wav_handle)->wav_file ) {
        return -2;
    }

    (*wav_handle)->n_samples_written = 0;
    (*wav_handle)->sample_format = sample_format;


    memcpy( (*wav_handle)->wav_header.riff, "RIFF", 4 );
    memcpy( (*wav_handle)->wav_header.wave, "WAVE", 4 );
    memcpy( (*wav_handle)->wav_header.fmt,  "fmt ", 4 );
    memcpy( (*wav_handle)->wav_header.data_header, "data", 4 );

    (*wav_handle)->wav_header.fmt_header_len = 16;

    if ( sample_format == F32 || sample_format == F64 ) {
        (*wav_handle)->wav_header.data_format = 3;
    }
    else {
        (*wav_handle)->wav_header.data_format = 1;
    }

    (*wav_handle)->wav_header.channels = channels;
    (*wav_handle)->wav_header.sample_rate = sample_rate;

    if ( sample_format == U8 ) {
        (*wav_handle)->wav_header.bits_per_sample = 8;
    }
    else if ( sample_format == S16 ) {
        (*wav_handle)->wav_header.bits_per_sample = 16;
    }
    else if ( sample_format == S24 ) {
        (*wav_handle)->wav_header.bits_per_sample = 24;
    }
    else if ( sample_format == S32 || sample_format == F32 ) {
        (*wav_handle)->wav_header.bits_per_sample = 32;
    }
    else if ( sample_format == S64 || sample_format == F64 ) {
        (*wav_handle)->wav_header.bits_per_sample = 64;
    }

    (*wav_handle)->wav_header.block_align =
        channels * ( ((*wav_handle)->wav_header.bits_per_sample+7)/8 );

    (*wav_handle)->wav_header.bytes_per_second =
        sample_rate * (*wav_handle)->wav_header.block_align;

    fwrite( &(*wav_handle)->wav_header, 1, 44, (*wav_handle)->wav_file );

    return 0;
} /* wav_open() */

/*-----------------------------------------------------------------*/

void wavwriter_write_samples( wav_file_t* wav_handle, wav_sample_t* samples, int n_samples ) {
    wav_sample_t sample;
    switch ( wav_handle->sample_format ) {
        case S16:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 2 );
                }
                fwrite( &sample, 1, 2, wav_handle->wav_file );
            }
            break;
        case S24:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 3 );
                }
                fwrite( &sample, 1, 3, wav_handle->wav_file );
            }
            break;
        case S32:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 4 );
                }
                fwrite( &sample, 1, 4, wav_handle->wav_file );
            }
            break;
        case S64:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 8 );
                }
                fwrite( &sample, 1, 8, wav_handle->wav_file );
            }
            break;

        case U8:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                fwrite( &sample, 1, 1, wav_handle->wav_file );
            }
            break;

        case F32:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 4 );
                }
                fwrite( &sample, 1, 4, wav_handle->wav_file );
            }
            break;
        case F64:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 8 );
                }
                fwrite( &sample, 1, 8, wav_handle->wav_file );
            }
            break;
    }

    wav_handle->n_samples_written += n_samples;
} /* wav_write_samples() */

/*-----------------------------------------------------------------*/

void wavwriter_write_samples_from_float(
    wav_file_t* wav_handle, wav_sample_t* samples, int n_samples
) {
    wav_sample_t sample;
    switch ( wav_handle->sample_format ) {
        case S16:
            for ( int i = 0; i < n_samples; i++ ) {
                sample.i16 = (int16_t)( samples[i].f32 * INT16_MAX );
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 2 );
                }
                fwrite( &sample, 1, 2, wav_handle->wav_file );
            }
            break;
        case S24:
            for ( int i = 0; i < n_samples; i++ ) {
                sample.i24 = (int32_t)( samples[i].f32 * INT24_MAX );
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 3 );
                }
                fwrite( &sample, 1, 3, wav_handle->wav_file );
            }
            break;
        case S32:
            for ( int i = 0; i < n_samples; i++ ) {
                sample.i32 = (int32_t)( samples[i].f32 * INT32_MAX );
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 4 );
                }
                fwrite( &sample, 1, 4, wav_handle->wav_file );
            }
            break;
        case S64:
            for ( int i = 0; i < n_samples; i++ ) {
                sample.i64 = (int64_t)( samples[i].f32 * INT64_MAX );
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 8 );
                }
                fwrite( &sample, 1, 8, wav_handle->wav_file );
            }
            break;

        case U8:
            for ( int i = 0; i < n_samples; i++ ) {
                sample.u8 = (uint8_t)( (0.5*samples[i].f32 + 0.5) * UINT8_MAX );
                fwrite( &sample, 1, 1, wav_handle->wav_file );
            }
            break;

        case F32:
            for ( int i = 0; i < n_samples; i++ ) {
                sample = samples[i];
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 4 );
                }
                fwrite( &sample, 1, 4, wav_handle->wav_file );
            }
            break;
        case F64:
            for ( int i = 0; i < n_samples; i++ ) {
                sample.f64 = (double) samples[i].f32;
                if ( !wav_handle->little_endian ) {
                    _switch_endianness( &sample, 8 );
                }
                fwrite( &sample, 1, 8, wav_handle->wav_file );
            }
            break;
    }

    wav_handle->n_samples_written += n_samples;
} /* wavwriter_write_samples_from_float() */

/*-----------------------------------------------------------------*/

void wavwriter_close( wav_file_t* wav_handle ) {
    uint32_t file_size =
        wav_handle->wav_header.channels *
        wav_handle->wav_header.sample_rate *
        wav_handle->n_samples_written *
        (wav_handle->wav_header.bits_per_sample / 8)
        + 44 - 8;

    uint32_t data_len =
        wav_handle->wav_header.block_align *
        (wav_handle->n_samples_written / wav_handle->wav_header.channels);

    wav_handle->wav_header.file_size = file_size;
    wav_handle->wav_header.data_block_len = data_len;

    rewind( wav_handle->wav_file );
    fwrite( &wav_handle->wav_header, 44, 1, wav_handle->wav_file );
    fclose( wav_handle->wav_file );

    free( wav_handle );
} /* wav_close() */
