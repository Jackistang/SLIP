#ifndef SLIP
#define SLIP

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SLIP_SUCCESS = 0,
    SLIP_ERROR,
    SLIP_TYPE_ERROR,
    SLIP_NOMEMORY,
};

typedef struct slip {
    enum {
        SLIP_NONE = 0,
        SLIP_ENCODER,
        SLIP_DECODER,
    } type;

    uint8_t *buf;   /* Buffer for frame. */
    uint16_t len;   /* Buffer size. */
    uint16_t idx;   /* Index in Buffer */
    bool has_frame; /*  */

    enum {
        UNKNOWN_STATE = 0,
        FRAME_START_STATE,
        DECODING_STATE,
        ESCAPE_STATE,
    } state;    /* Decoder state */
} slip_t;

/* Encoder */

/**
 * 
 * @return None
*/
extern void slip_encoder_init(slip_t *encoder, uint8_t *buf, uint16_t len);

/**
 * 
 * @return SLIP_SUCCESS
 * @return -SLIP_TYPE_ERROR 
*/
extern int slip_encoder_deinit(slip_t *encoder);

/**
 * 
 * @return SLIP_SUCCESS
 * @return -SLIP_TYPE_ERROR
*/
extern int slip_encoder_process(slip_t *encoder, uint8_t *buf, uint16_t len);

/**
 * @return Actual frame pointer.
 * @return NULL. No frame in encoder.
*/
extern uint8_t* slip_encoder_get_frame(slip_t *encoder);

/**
 * 
 * @return Actual frame size.
 * @return 0. No frame in encoder.
*/
extern uint16_t slip_encoder_get_frame_size(slip_t *encoder);

/* Decoder */

/**
 * 
 * @return None
*/
extern void slip_decoder_init(slip_t *decoder, uint8_t *buf, uint16_t len);

/**
 * 
 * @return SLIP_SUCCESS
 * @return -SLIP_TYPE_ERROR 
*/
extern int slip_decoder_deinit(slip_t *decoder);

/**
 * 
 * @return SLIP_SUCCESS
 * @return -SLIP_ERROR
*/
extern int slip_decoder_process_byte(slip_t *decoder, uint8_t byte);

/**
 * 
 * @return true.    A frame in Decoder.
 * @return false.   No frame in Decoder.
*/
extern bool slip_decoder_has_frame(slip_t *decoder);

/**
 * @return Actual frame pointer.
 * @return NULL. No frame in decoder.
*/
extern uint8_t* slip_decoder_get_frame(slip_t *decoder);

/**
 * 
 * @return Actual frame size.
 * @return 0. No frame in decoder.
*/
extern uint16_t slip_decoder_get_frame_size(slip_t *decoder);

#ifdef __cplusplus
}
#endif

#endif /* SLIP */
