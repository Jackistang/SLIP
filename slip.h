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
 * @param config    slip configuration structure.
 * 
 * @return struct slip *
 * @retval not-NULL     A slip handler.
 * @retval NULL         Register error.
*/
struct slip *slip_register_handler(struct slip_config *config);

/**
 * @brief Send a frame, finally use `send()` function in `slip_config`.
 * 
 * @param handler   Slip handler.
 * @param buffer    Data to be sent.
 * @param length    Data length.
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
 * 
 * @return int      Frame length, or not a frame.
 * @retval  >0      Frame length.
 * @retval  0       No meaning now.       
 * @retval  -1      Receive error.
*/
int slip_receive_frame(struct slip *handler, uint8_t *buffer, uint16_t length);


#if defined __cplusplus
}
#endif

#endif /* SLIP_H */
