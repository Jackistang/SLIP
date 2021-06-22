#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/TestDB.h>
#include "slip.h"

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))
#define CU_ASSERT_ARRAY_EQUAL   CU_ASSERT_NSTRING_EQUAL     // when data is larger than 125, may have bug.

static uint8_t buffer[100];
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
        if (right == right)
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

static uint8_t buf1[] = { 0x1 };
static uint8_t buf1_expect[] = {0xC0, 0x1, 0xC0};
static uint8_t buf2[] = { 0xC0 };
static uint8_t buf2_expect[] = {0xC0, 0xDB, 0xDC, 0xC0};
static uint8_t buf3[] = { 0xDB };
static uint8_t buf3_expect[] = {0xC0, 0xDB, 0xDD, 0xC0};

#define TEST_SEND_FRAME(n) do { \
    buffer_reset();             \
    uint8_t *p##n = buf##n;     \
    err = slip_send_frame(&slip_handler, p##n, ARRAY_SIZE(buf##n));   \
    CU_ASSERT_EQUAL(err, 0);    \
    CU_ASSERT_ARRAY_EQUAL(buffer, buf##n##_expect, ARRAY_SIZE(buf##n##_expect));    \
} while (0)

void test_slip_send_frame(void)
{
    int err;

    // Send frame success.
    TEST_SEND_FRAME(1);
    TEST_SEND_FRAME(2);
    TEST_SEND_FRAME(3);

    // Send frame fail.
}


static uint8_t buf4[] = {0xC0, 0x1, 0xC0};          // no escape
static uint8_t buf4_expect[] = { 0x1 };
static uint8_t buf5[] = {0xC0, 0xDB, 0xDC, 0xC0};   // 0xC0  escape
static uint8_t buf5_expect[] = { 0xC0 };
static uint8_t buf6[] = {0xC0, 0xDB, 0xDD, 0xC0};   // 0xDB escape
static uint8_t buf6_expect[] = { 0xDB };
static uint8_t buf7[] = {0xC0, 0x0, 0xC0, 0x10};    // out of frame
static uint8_t buf7_expect[] = {0x0};
static uint8_t buf8[] = {0xC0, 0xC0};               // empty
static uint8_t buf8_expect[] = {};


#define TEST_SEND_RECV_FRAME(n)  do { \
    buffer_reset(); \
    memcpy(buffer, buf##n, ARRAY_SIZE(buf##n)); \
    right = ARRAY_SIZE(buf##n); \
    err = slip_receive_frame(&slip_handler, recv_buffer, ARRAY_SIZE(recv_buffer), &recv_length); \
    CU_ASSERT_EQUAL(err, 0);    \
    CU_ASSERT_EQUAL(recv_length, ARRAY_SIZE(buf##n##_expect));  \
    CU_ASSERT_ARRAY_EQUAL(recv_buffer, buf##n##_expect, recv_length);   \
    recv_length = 0;    \
} while (0)

void test_slip_receive_frame(void)
{
    int err;
    uint16_t recv_length;
    uint8_t recv_buffer[100];
    memset(recv_buffer, 0, ARRAY_SIZE(recv_buffer));

    // recv_length = 0;
    TEST_SEND_RECV_FRAME(4);
    TEST_SEND_RECV_FRAME(5);
    TEST_SEND_RECV_FRAME(6);
    TEST_SEND_RECV_FRAME(7);
    TEST_SEND_RECV_FRAME(8);
    // TEST_SEND_RECV_FRAME(9);

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