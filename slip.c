#include "slip.h"
#include <stddef.h>

/* SLIP special character codes */
#define SLIP_END        0xC0    //  indicates end of packet
#define SLIP_ESC        0xDB    //  indicates byte stuffing
#define SLIP_ESC_END    0xDC    //  ESC ESC_END means END data byte
#define SLIP_ESC_ESC    0xDD    //  ESC ESC_ESC means ESC data byte


int slip_init(struct slip *handler, struct slip_config *config)
{
    SLIP_ASSERT(handler);
    SLIP_ASSERT(config);
    
    handler->state  = SLIP_UNKNOWN_STATE;
    rt_ringbuffer_init(&handler->ringbuffer, handler->ringbuffer_pool, ARRAY_SIZE(handler->ringbuffer_pool));
    handler->config = config;
    return 0;
}

void slip_reset(struct slip *handler)
{
    handler->state = SLIP_UNKNOWN_STATE;
    rt_ringbuffer_reset(&handler->ringbuffer);
}

int slip_send_frame(struct slip *handler, uint8_t *buffer, uint16_t length)
{
    SLIP_ASSERT(handler);
    SLIP_ASSERT(buffer);

    uint8_t send_buffer[SLIP_MAX_BUFFER];
    uint16_t idx = 0;
    
    send_buffer[idx++] = SLIP_END;
    for (uint16_t i = 0; i < length; i++) {
        uint8_t c = buffer[i];
        switch (c) {
        case SLIP_END:  
            send_buffer[idx++] = SLIP_ESC;
            c = SLIP_ESC_END;
            break;
        case SLIP_ESC:
            send_buffer[idx++] = SLIP_ESC;
            c = SLIP_ESC_ESC;
            break;
        default:    break;
        }
        // Check index, used to truncate buffer.
        if (idx >= SLIP_MAX_BUFFER-1)
            break;

        send_buffer[idx++] = c;

        // Check index, used to truncate buffer.
        if (idx >= SLIP_MAX_BUFFER-1)
            break;
    }
    send_buffer[idx++] = SLIP_END;

    handler->config->send(send_buffer, idx);

    return 0;
}

int slip_receive_frame(struct slip *handler, uint8_t *buffer, uint16_t length, uint16_t *recv_length)
{
    SLIP_ASSERT(handler);
    SLIP_ASSERT(buffer);
    SLIP_ASSERT(recv_length);
    SLIP_ASSERT(length > 0);

    struct rt_ringbuffer *rb = &handler->ringbuffer;

    uint16_t idx = 0;
    int size = 0;
    uint8_t temp_buf[SLIP_MAX_BUFFER];
    while (1) {
        if (rt_ringbuffer_data_len(rb) == 0) {
            size = handler->config->recv(temp_buf, ARRAY_SIZE(temp_buf));
            if (size <= 0)
                continue;
        }
        
        uint8_t ch;
        rt_ringbuffer_put_force(rb, temp_buf, size);
        while (rt_ringbuffer_data_len(rb) > 0) {
            rt_ringbuffer_getchar(rb, &ch);

            switch (handler->state) {
            case SLIP_UNKNOWN_STATE:
                if (ch == SLIP_END)
                    handler->state = SLIP_FRAME_START_STATE;
                break;
            case SLIP_FRAME_START_STATE:
                if (ch == SLIP_END)
                    break;
                handler->state = SLIP_DECODING_STATE;
                // fall through
            case SLIP_DECODING_STATE:
                if (ch == SLIP_ESC) {
                    rt_ringbuffer_getchar(rb, &ch);
                    SLIP_ASSERT(ch == SLIP_ESC_END || ch == SLIP_ESC_ESC);
                    buffer[idx++] = (ch == SLIP_ESC_END) ? SLIP_END : SLIP_ESC;
                } else if (ch == SLIP_END) {
                    handler->state = SLIP_FRAME_END_STATE;
                    // Success receive a frame.
                    *recv_length = idx;
                    return 0;
                } else {
                    buffer[idx++] = ch;
                }
                break;
            case SLIP_FRAME_END_STATE:
                if (ch == SLIP_END)
                    handler->state = SLIP_DECODING_STATE;
                else 
                    handler->state = SLIP_ERROR_STATE;
                break;
            case SLIP_ERROR_STATE:
                if (ch == SLIP_END)
                    handler->state = SLIP_FRAME_END_STATE;
                break;
            default:    break;
            }

            if (idx >= length)
                return -1;      // Buffer is not enough to store frame.
        }
    }

    return -1;
}

