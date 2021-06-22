#ifndef SLIP_H
#define SLIP_H

#include <stdint.h>

#if defined __cplusplus
extern "C" {
#endif

struct slip {
    struct slip_config *config;
};
struct slip_config {
    /* Send data to uart. */
    int (*send)(uint8_t *buffer, uint16_t length);

    /* Receive data from uart. */
    int (*recv)(uint8_t *buffer, uint16_t length);
};

/**
 * @brief Register a slip handler.
 * 
 * @param handler   slip handler point. 
 * @param config    slip configuration structure.
 * 
 * @return int
 * @retval 0        Success.
 * @retval -1       Error.
*/
int slip_register_handler(struct slip *handler, struct slip_config *config);

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
 * @retval  -1      Receive error.
*/
int slip_receive_frame(struct slip *handler, uint8_t *buffer, uint16_t length, uint16_t *recv_length);


#if defined __cplusplus
}
#endif

#endif /* SLIP_H */
