#include "slip.h"
#include <stddef.h>

/* SLIP special character codes */
#define SLIP_END        0xC0    //  indicates end of packet
#define SLIP_ESC        0xDB    //  indicates byte stuffing
#define SLIP_ESC_END    0xDC    //  ESC ESC_END means END data byte
#define SLIP_ESC_ESC    0xDD    //  ESC ESC_ESC means ESC data byte


int slip_register_handler(struct slip *handler, struct slip_config *config)
{
    return -1;
}

int slip_send_frame(struct slip *handler, uint8_t *buffer, uint16_t length)
{
    return -1;
}

int slip_receive_frame(struct slip *handler, uint8_t *buffer, uint16_t length, uint16_t *recv_length)
{
    return -1;
}

