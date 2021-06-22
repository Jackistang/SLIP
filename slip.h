#ifndef SLIP_H
#define SLIP_H

#include "ringbuffer.h"
#include <stdint.h>
#include <assert.h>

#if defined __cplusplus
extern "C" {
#endif

#define SLIP_MAX_BUFFER 100

#define SLIP_ASSERT assert

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))

typedef enum {
    SLIP_UNKNOWN_STATE = 0,
    SLIP_FRAME_START_STATE,
    SLIP_DECODING_STATE,
    SLIP_FRAME_END_STATE,
    SLIP_ERROR_STATE,
} SLIP_DECODER_STATE;

struct slip {
    SLIP_DECODER_STATE state;
    struct rt_ringbuffer ringbuffer;
    uint8_t ringbuffer_pool[SLIP_MAX_BUFFER];
    struct slip_config *config;
};
struct slip_config {
    /* Send data to uart. */
    void (*send)(uint8_t *buffer, uint16_t length);

    /**
     * @brief Receive data from uart.
     * 
     * @return int
     * @retval >=0   Receive data length.
     * @return -1    Error.
    */
    int (*recv)(uint8_t *buffer, uint16_t length);
};

/**
 * @brief Init a slip handler.
 * 
 * @param handler   slip handler point. 
 * @param config    slip configuration structure.
 * 
 * @return int
 * @retval 0        Success.
 * @retval -1       Error.
*/
int slip_init(struct slip *handler, struct slip_config *config);

/**
 * @brief Reset slip state.
 * 
 * @param handler   slip handler point.
 * 
 * @return void
*/
void slip_reset(struct slip *handler);

/**
 * @brief Send a frame, finally use `send()` function in `slip_config`.
 * 
 * @param handler       Slip handler.
 * @param buffer        Data to be sent.
 * @param length        Data length.
 * 
 * @return int
 * @retval 0        Send success.
 * @retval -1       Send failed.
 * 
 * @note If the length larger than SLIP_MAX_BUFFER, send data will be truncated.
*/
int slip_send_frame(struct slip *handler, uint8_t *buffer, uint16_t length);

/**
 * @brief Receive a slip frame, finally use `recv()` function in `slip_config`.
 * 
 * @param handler   Slip handler.
 * @param buffer    Buffer to store data.
 * @param length    Buffer length.
 * @param recv_length   Receive data length point.
 * 
 * @return int
 * @retval  0       Receive success.       
 * @retval  -1      Buffer is not enough.
*/
int slip_receive_frame(struct slip *handler, uint8_t *buffer, uint16_t length, uint16_t *recv_length);


#if defined __cplusplus
}
#endif

#endif /* SLIP_H */
