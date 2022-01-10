#include "slip.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* SLIP escape character codes */
#define END        0xC0    /* indicates end of packet */
#define ESC        0xDB    /* indicates byte stuffing */
#define ESC_END    0xDC    /* ESC ESC_END means END data byte */
#define ESC_ESC    0xDD    /* ESC ESC_ESC means ESC data byte */

#define HCI_3WIRE_0X11       0x11
#define HCI_3WIRE_0x13       0x13
#define ESC_HCI_3WIRE_0X11   0xDE
#define ESC_HCI_3WIRE_0X13   0xDF

#define CHECK_ENCODER_TYPE(encoder) do {\
    if ((encoder)->type != SLIP_ENCODER) {\
        return -SLIP_TYPE_ERROR;    \
    }   \
} while (0)

/* Encoder */
void slip_encoder_init(slip_t *encoder, uint8_t *buf, uint16_t len)
{
    assert(encoder != NULL);
    assert(buf != NULL);
    assert(len > 0);

    memset(buf, 0, len);

    slip_encoder_deinit(encoder);
    encoder->type = SLIP_ENCODER;
    encoder->buf  = buf;
    encoder->len  = len;
    
    return ;
}

int slip_encoder_deinit(slip_t *encoder)
{
    assert(encoder != NULL);

    encoder->buf       = NULL;
    encoder->len       = 0;
    encoder->type      = SLIP_NONE;
    encoder->idx       = 0;
    encoder->has_frame = false;
    return SLIP_SUCCESS;
}

int slip_encoder_process(slip_t *encoder, uint8_t *buf, uint16_t len)
{
    assert(encoder != NULL);

    if (buf == NULL || len == 0) {
        return -SLIP_ERROR;
    }
    CHECK_ENCODER_TYPE(encoder);

    /* 为了支持初始化编码器后，该函数能重复调用. */
    encoder->idx = 0;

    uint8_t *cur = buf;
    uint8_t *end = buf + len;

    encoder->buf[encoder->idx++] = END;
    while (cur < end) {
        if (encoder->idx >= encoder->len-1) {
            break;
        }

        uint8_t c = *cur;
        switch (c) {
        case END: 
            encoder->buf[encoder->idx++] = ESC;
            c =  ESC_END;
            break;
        case ESC: 
            encoder->buf[encoder->idx++] = ESC;
            c =  ESC_ESC;
            break;
        case HCI_3WIRE_0X11:
            encoder->buf[encoder->idx++] = ESC;
            c = ESC_HCI_3WIRE_0X11;
            break;
        case HCI_3WIRE_0x13:
            encoder->buf[encoder->idx++] = ESC;
            c = ESC_HCI_3WIRE_0X13;
            break;
        default:  break;
        }

        if (encoder->idx < encoder->len) {
            encoder->buf[encoder->idx++] = c;
        }

        cur++;
    }

    if (cur == end && encoder->idx < encoder->len) {
        encoder->buf[encoder->idx++] = END;
        encoder->has_frame = true;
        return SLIP_SUCCESS;
    }

    /* Clear memory in encoder. */
    /* encoder->idx = 0; */

    return -SLIP_NOMEMORY;
}

static bool slip_encoder_has_frame(slip_t *encoder) {
    if (encoder == NULL || !encoder->has_frame) {
        return false;
    }

    return true;
}

uint8_t* slip_encoder_get_frame(slip_t *encoder)
{
    if (!(slip_encoder_has_frame(encoder))) {
        return NULL;
    }

    return encoder->buf;
}

uint16_t slip_encoder_get_frame_size(slip_t *encoder)
{
    if (!(slip_encoder_has_frame(encoder))) {
        return 0;
    }

    return encoder->idx;
}


/* Decoder */
void slip_decoder_init(slip_t *decoder, uint8_t *buf, uint16_t len)
{
    assert(decoder != NULL);
    assert(buf != NULL);
    assert(len > 0);

    memset(buf, 0, len);

    slip_decoder_deinit(decoder);
    decoder->type = SLIP_DECODER;
    decoder->buf  = buf;
    decoder->len  = len;

    return ;
}

int slip_decoder_deinit(slip_t *decoder)
{
    assert(decoder != NULL);

    decoder->type      = SLIP_NONE;
    decoder->buf       = NULL;
    decoder->len       = 0;
    decoder->idx       = 0;
    decoder->has_frame = false;
    decoder->state     = UNKNOWN_STATE;

    return SLIP_SUCCESS;
}

static bool slip_decoder_process_escape(slip_t *decoder, uint8_t c)
{
    switch (c) {
    case ESC_END:            decoder->buf[decoder->idx++] = END;              break;
    case ESC_ESC:            decoder->buf[decoder->idx++] = ESC;              break;
    case ESC_HCI_3WIRE_0X11: decoder->buf[decoder->idx++] = HCI_3WIRE_0X11;   break;
    case ESC_HCI_3WIRE_0X13: decoder->buf[decoder->idx++] = HCI_3WIRE_0x13;   break;
    default:    
        decoder->idx = 0;   /* Clear memory in decoder. */
        return false;
    }

    return true;
}

int slip_decoder_process_byte(slip_t *decoder, uint8_t byte)
{
    assert(decoder != NULL);

    if (decoder->idx >= decoder->len) {
        return -SLIP_NOMEMORY;
    }

    uint8_t c = byte;
    switch (decoder->state) {
    case UNKNOWN_STATE: 
        if (c == END) {
            decoder->state = FRAME_START_STATE;
        }
        break;
    case FRAME_START_STATE: 
        if (c == END) {
            break;
        }
        decoder->state = DECODING_STATE;
        /* fallthrough */
    case DECODING_STATE:    
        if (c == END) {
            /* A frame is ready. */
            decoder->has_frame = true;
            decoder->state = UNKNOWN_STATE;
        } else if (c == ESC) {
            decoder->state = ESCAPE_STATE;
        } else {
            decoder->buf[decoder->idx++] = c;
        }
        break;
    case ESCAPE_STATE:
        if(!slip_decoder_process_escape(decoder, c)) {
            decoder->state = UNKNOWN_STATE;
            return -SLIP_ERROR;
        }

        decoder->state = DECODING_STATE;
        break;
    default: 
        return -SLIP_ERROR;
    }

    return SLIP_SUCCESS;
}

bool slip_decoder_has_frame(slip_t *decoder)
{
    if (decoder == NULL || !decoder->has_frame) {
        return false;
    }

    return true;
}

uint8_t* slip_decoder_get_frame(slip_t *decoder)
{
    if (!slip_decoder_has_frame(decoder)) {
        return NULL;
    }

    return decoder->buf;
}

uint16_t slip_decoder_get_frame_size(slip_t *decoder)
{
    if (!slip_decoder_has_frame(decoder)) {
        return 0;
    }

    return decoder->idx;
}
