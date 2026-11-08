#include "wavwriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int bytes_per_sample;
};


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

    (*wav_handle)->wav_file = fopen( file_path, "wb" );
    if ( !(*wav_handle)->wav_file ) {
        return -2;
    }

    (*wav_handle)->n_samples_written = 0;
    (*wav_handle)->sample_format = sample_format;


    memcpy( (*wav_handle)->wav_header.riff,         "RIFF", 4 );
    (*wav_handle)->wav_header.file_size = 0;
    memcpy( (*wav_handle)->wav_header.wave,         "WAVE", 4 );
    memcpy( (*wav_handle)->wav_header.fmt,          "fmt ", 4 );
    memcpy( (*wav_handle)->wav_header.data_header,  "data", 4 );

    (*wav_handle)->wav_header.fmt_header_len = 16;

    if ( sample_format == F32 || sample_format == F64 ) {
        (*wav_handle)->wav_header.data_format = 3;
    }
    else {
        (*wav_handle)->wav_header.data_format = 1;
    }

    (*wav_handle)->wav_header.channels = channels;
    (*wav_handle)->wav_header.sample_rate = sample_rate;

    switch( sample_format ) {
        case U8:
            (*wav_handle)->bytes_per_sample = 1;
            (*wav_handle)->wav_header.bits_per_sample =  8;
            break;

        case S16:
            (*wav_handle)->bytes_per_sample = 2;
            (*wav_handle)->wav_header.bits_per_sample = 16;
            break;

        case S24:
            (*wav_handle)->bytes_per_sample = 3;
            (*wav_handle)->wav_header.bits_per_sample = 24;
            break;

        case S32: case F32:
            (*wav_handle)->bytes_per_sample = 4;
            (*wav_handle)->wav_header.bits_per_sample = 32;
            break;

        case S64: case F64:
            (*wav_handle)->bytes_per_sample = 8;
            (*wav_handle)->wav_header.bits_per_sample = 64;
            break;
    }

    (*wav_handle)->wav_header.block_align =
        channels * ( ((*wav_handle)->wav_header.bits_per_sample+7)/8 );

    (*wav_handle)->wav_header.bytes_per_second =
        sample_rate * (*wav_handle)->wav_header.block_align;

    fwrite( &(*wav_handle)->wav_header, 1, 44, (*wav_handle)->wav_file );

    return 0;
} /* wav_open() */

/*-----------------------------------------------------------------*/

void wavwriter_write_samples( wav_file_t* wav_handle, void* samples, int n_samples ) {
    int size = n_samples * wav_handle->bytes_per_sample;
    fwrite( samples, size, 1, wav_handle->wav_file );
    wav_handle->n_samples_written += n_samples;
} /* wav_write_samples() */


/*-----------------------------------------------------------------*/

void wavwriter_close( wav_file_t* wav_handle ) {
    uint32_t file_size =
        wav_handle->n_samples_written *
        wav_handle->bytes_per_sample
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
