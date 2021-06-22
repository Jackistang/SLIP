#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/TestDB.h>
#include "slip.h"

#define CU_ASSERT_ARRAY_EQUAL   CU_ASSERT_NSTRING_EQUAL     // when data is larger than 125, may have bug.

static uint8_t buffer[200];
static uint16_t left;
static uint16_t right;
struct slip slip_handler;

static void buffer_reset(void)
{
    memset(buffer, 0, ARRAY_SIZE(buffer));
    left = right = 0;
}

static int send(uint8_t *buf, uint16_t length)
{
    for (size_t i = 0; i < length; i++) {
        buffer[right++] = buf[i];
        // roll back.
        if (right >= ARRAY_SIZE(buffer))
            right = 0;
        // buffer is full.
        if (right == left)
            return i;
    }
    return length;
}

static int recv(uint8_t *buf, uint16_t length)
{
    size_t i = 0;
    while (left != right) {
        // buf is full.
        if (i == length)
            break;
        buf[i++] = buffer[left++];
        // roll back.
        if (left >= ARRAY_SIZE(buffer))
            left = 0;
    }
    return i;
}

static struct slip_config config = {
    .send = send,
    .recv = recv
};

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite(void)
{
    int err = slip_register_handler(&slip_handler, &config);
    // CU_ASSERT_EQUAL(err, 0);
    return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
    return 0;
}

static uint8_t send_buf1[] = { 0x1 };
static uint8_t send_buf1_expect[] = {0xC0, 0x1, 0xC0};
static uint8_t send_buf2[] = { 0xC0 };
static uint8_t send_buf2_expect[] = {0xC0, 0xDB, 0xDC, 0xC0};
static uint8_t send_buf3[] = { 0xDB };
static uint8_t send_buf3_expect[] = {0xC0, 0xDB, 0xDD, 0xC0};
// test send truncate, SLIP_MAX_BUFFER is 100.
static uint8_t send_buf4[] = {  0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, };
static uint8_t send_buf4_expect[] = {   0xC0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 
                                        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0xC0, };

#define TEST_SEND_FRAME(n) do { \
    buffer_reset();             \
    uint8_t *p##n = send_buf##n;     \
    err = slip_send_frame(&slip_handler, p##n, ARRAY_SIZE(send_buf##n));   \
    CU_ASSERT_EQUAL(err, 0);    \
    CU_ASSERT_ARRAY_EQUAL(buffer, send_buf##n##_expect, ARRAY_SIZE(send_buf##n##_expect));    \
} while (0)

void test_slip_send_frame(void)
{
    int err;

    // Send frame success.
    TEST_SEND_FRAME(1);
    TEST_SEND_FRAME(2);
    TEST_SEND_FRAME(3);
    TEST_SEND_FRAME(4);

    // Send frame fail.
}

// Test Decoding State.
static uint8_t recv_buf1[] = {0xC0, 0x1, 0xC0};          // no escape
static uint8_t recv_buf1_expect[] = { 0x1 };
static uint8_t recv_buf2[] = {0xC0, 0xDB, 0xDC, 0xC0};   // 0xC0  escape
static uint8_t recv_buf2_expect[] = { 0xC0 };
static uint8_t recv_buf3[] = {0xC0, 0xDB, 0xDD, 0xC0};   // 0xDB escape
static uint8_t recv_buf3_expect[] = { 0xDB };
// static uint8_t recv_buf4[] = {0xC0, 0xC0};               // empty
// static uint8_t recv_buf4_expect[] = {};

// Test Unknown State.
static uint8_t recv_buf5[] = { 0x1, 0x1, 0xC0, 0x2, 0xC0 };
static uint8_t recv_buf5_expect[] = {0x2};

// Test Frame Start State.
static uint8_t recv_buf6[] = { 0xC0, 0xC0, 0x2, 0xC0 };
static uint8_t recv_buf6_expect[] = {0x2};

// Test Frame continious
static uint8_t recv_buf7[] = { 0xC0, 0x2, 0xC0, 0xC0, 0x1, 0xC0 };
static uint8_t recv_buf7_expect[] = {0x2};
static uint8_t recv_buf7_expect2[] = {0x1};

// It is different from Frame Start State.
static uint8_t recv_buf8[] = { 0xC0, 0x2, 0xC0, 0xC0, 0xC0 };
static uint8_t recv_buf8_expect[] = {0x2};
static uint8_t recv_buf8_expect2[] = {};

// Test Error State.
static uint8_t recv_buf9[] = { 0xC0, 0x2, 0xC0, 0x3, 0xC0, 0xC0, 0x1, 0xC0 };
static uint8_t recv_buf9_expect[] = {0x2};
static uint8_t recv_buf9_expect2[] = {0x1};

#define TEST_RECV_FRAME(n)  do { \
    buffer_reset(); \
    slip_reset(&slip_handler);  \
    memcpy(buffer, recv_buf##n, ARRAY_SIZE(recv_buf##n)); \
    right = ARRAY_SIZE(recv_buf##n); \
    err = slip_receive_frame(&slip_handler, recv_buffer, ARRAY_SIZE(recv_buffer), &recv_length); \
    CU_ASSERT_EQUAL(err, 0);    \
    CU_ASSERT_EQUAL(recv_length, ARRAY_SIZE(recv_buf##n##_expect));  \
    CU_ASSERT_ARRAY_EQUAL(recv_buffer, recv_buf##n##_expect, recv_length);   \
    recv_length = 0;    \
} while (0)

#define TEST_RECV_FRAME_2(n)    do { \
    err = slip_receive_frame(&slip_handler, recv_buffer, ARRAY_SIZE(recv_buffer), &recv_length); \
    CU_ASSERT_EQUAL(err, 0);    \
    CU_ASSERT_EQUAL(recv_length, ARRAY_SIZE(recv_buf##n##_expect2));  \
    CU_ASSERT_ARRAY_EQUAL(recv_buffer, recv_buf##n##_expect2, recv_length);   \
    recv_length = 0;    \
} while (0)

void test_slip_receive_frame(void)
{
    int err;
    uint16_t recv_length;
    uint8_t recv_buffer[100];
    memset(recv_buffer, 0, ARRAY_SIZE(recv_buffer));

    // recv_length = 0;
    TEST_RECV_FRAME(1);
    TEST_RECV_FRAME(2);
    TEST_RECV_FRAME(3);
    // TEST_RECV_FRAME(4);
    TEST_RECV_FRAME(5);
    TEST_RECV_FRAME(6);
    TEST_RECV_FRAME(7);
    TEST_RECV_FRAME_2(7);
    TEST_RECV_FRAME(8);
    TEST_RECV_FRAME_2(8);
    TEST_RECV_FRAME(9);
    TEST_RECV_FRAME_2(9);

    // Receive frame fail.
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

    CU_TestInfo test_array[] = {
        {"test slip send frame", test_slip_send_frame},
        {"test slip receive frame", test_slip_receive_frame},
        CU_TEST_INFO_NULL,
    };

    CU_SuiteInfo suites[] = {
        {"suite1", init_suite, clean_suite, NULL, NULL, test_array},
        CU_SUITE_INFO_NULL,
    };

    if (CUE_SUCCESS != CU_register_suites(suites)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}