#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "slip.h"

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))

static slip_t encoder;
static slip_t decoder;
static uint8_t buf[512];

TEST_GROUP(EncoderTestGroup)
{
    void setup()
    {
        slip_encoder_init(&encoder, buf, ARRAY_SIZE(buf));
    }

    void teardown()
    {
        slip_encoder_deinit(&encoder);
    }
};

#define TEST_ENCODER_SUCCESS(data, expect) do {\
    int ret = 0;    \
    ret = slip_encoder_process(&encoder, data, ARRAY_SIZE(data));   \
    CHECK_EQUAL(SLIP_SUCCESS, ret); \
    LONGS_EQUAL(ARRAY_SIZE(expect), slip_encoder_get_frame_size(&encoder)); \
    MEMCMP_EQUAL(expect, slip_encoder_get_frame(&encoder), ARRAY_SIZE(expect)); \
} while (0)

/**
 * 测试 END, ESC 字符是否成功编码
*/
TEST(EncoderTestGroup, TestEncoderSuccessHasEscape)
{
    uint8_t data[] = {0xC0, 0xDB, 0xFF, 0x00};
    uint8_t expect[] = {0xC0, 0xDB, 0xDC, 0xDB, 0xDD, 0xFF, 0x00, 0xC0};
    TEST_ENCODER_SUCCESS(data, expect);
}

/**
 * 测试非转义字符编码
*/
TEST(EncoderTestGroup, TestEncoderSuccessNoEscape)
{
    uint8_t data[] = {0x00, 0xFF};
    uint8_t expect[] = {0xC0, 0x00, 0xFF, 0xC0};
    TEST_ENCODER_SUCCESS(data, expect);
}

/**
 * 测试编码器缓冲区太小
*/
TEST(EncoderTestGroup, TestEncoderFailSmallBuffer)
{
    uint8_t small_buf[2];
    slip_encoder_deinit(&encoder);
    slip_encoder_init(&encoder, small_buf, ARRAY_SIZE(small_buf));

    int ret = 0;
    uint8_t data[] = {0x00, 0xFF};
    uint8_t expect[] = {0xC0, 0x00, 0xFF, 0xC0};

    ret = slip_encoder_process(&encoder, data, ARRAY_SIZE(data));
    CHECK_EQUAL(-SLIP_NOMEMORY, ret);
    LONGS_EQUAL(0, slip_encoder_get_frame_size(&encoder));
    POINTERS_EQUAL(NULL, slip_encoder_get_frame(&encoder));
}

/**
 * 测试 3Wire 编码
*/
TEST(EncoderTestGroup, TestEncoderSuccess3Wire)
{
    uint8_t data[] = {0x11, 0x13};
    uint8_t expect[] = {0xC0, 0xDB, 0xDE, 0xDB, 0xDF, 0xC0};
    TEST_ENCODER_SUCCESS(data, expect);
}


TEST_GROUP(DecoderTestGroup)
{
    void setup()
    {
        slip_decoder_init(&decoder, buf, sizeof(buf)/sizeof(buf[0]));
    }

    void teardown()
    {
        slip_decoder_deinit(&decoder);
    }
};

#define TEST_DECODER_SUCCESS(data, expect) do {\
    int ret = 0;    \
    size_t i;   \
                \
    for (i = 0; i < ARRAY_SIZE(data)-1; i++) {  \
        ret = slip_decoder_process_byte(&decoder, data[i]); \
        CHECK_EQUAL(SLIP_SUCCESS, ret); \
        CHECK_FALSE(slip_decoder_has_frame(&decoder));  \
    }   \
        \
    ret = slip_decoder_process_byte(&decoder, data[i]); \
    CHECK_EQUAL(SLIP_SUCCESS, ret); \
    CHECK(slip_decoder_has_frame(&decoder));    \
    LONGS_EQUAL(ARRAY_SIZE(expect), slip_decoder_get_frame_size(&decoder)); \
    MEMCMP_EQUAL(expect, slip_decoder_get_frame(&decoder), ARRAY_SIZE(expect)); \
} while (0)

/**
 * 测试 END,ESC 是否成功解码
*/
TEST(DecoderTestGroup, TestDecoderSuccessHasEscape)
{
    uint8_t data[] = {0xC0, 0xDB, 0xDC, 0xDB, 0xDD, 0xFF, 0x00, 0xC0};
    uint8_t expect[] = {0xC0, 0xDB, 0xFF, 0x00};
    TEST_DECODER_SUCCESS(data, expect);
}

/**
 * 测试无转义字符是否成功解码
*/
TEST(DecoderTestGroup, TestDecoderSuccessNoEscape)
{
    uint8_t data[] = {0xC0, 0x00, 0xFF, 0xC0};
    uint8_t expect[] = {0x00, 0xFF};
    TEST_DECODER_SUCCESS(data, expect);
}

/**
 * 测试"数据流不以 END 开始"是否成功解码
*/
TEST(DecoderTestGroup, TestDecoderSuccessUnknownState)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0xC0, 0x00, 0xFF, 0xC0};
    uint8_t expect[] = {0x00, 0xFF};
    TEST_DECODER_SUCCESS(data, expect);
}

#define TEST_DECODER_FAIL(data) do {\
    int ret = 0;    \
                    \
    ret = slip_decoder_process_byte(&decoder, data[0]); \
    CHECK_EQUAL(SLIP_SUCCESS, ret); \
    ret = slip_decoder_process_byte(&decoder, data[1]); \
    CHECK_EQUAL(SLIP_SUCCESS, ret); \
    ret = slip_decoder_process_byte(&decoder, data[2]); \
    CHECK_EQUAL(-SLIP_ERROR, ret);  \
                                    \
    LONGS_EQUAL(0, slip_decoder_get_frame_size(&decoder));  \
    POINTERS_EQUAL(NULL, slip_encoder_get_frame(&decoder)); \
} while (0)


/**
 * 测试不支持的转义字符在“下边界内”
*/
TEST(DecoderTestGroup, TestDecoderFailInLowerBoundary)
{
    uint8_t data[] = {0xC0, 0xDB, 0x05, 0xC0};
    TEST_DECODER_FAIL(data);
}

/**
 * 测试不支持的转义字符在“下边界边缘”
*/
TEST(DecoderTestGroup, TestDecoderFailInLowerBoundaryEdge)
{
    uint8_t data[] = {0xC0, 0xDB, 0xDB, 0xC0};
    TEST_DECODER_FAIL(data);
}

/**
 * 测试不支持的转义字符在“上边界内”
*/
TEST(DecoderTestGroup, TestDecoderFailInHigherBoundary)
{
    uint8_t data[] = {0xC0, 0xDB, 0xEF, 0xC0};
    TEST_DECODER_FAIL(data);
}

/**
 * 测试不支持的转义字符在“上边界边缘”
*/
TEST(DecoderTestGroup, TestDecoderFailInHigherBoundaryEdge)
{
    uint8_t data[] = {0xC0, 0xDB, 0xE0, 0xC0};
    TEST_DECODER_FAIL(data);
}

/**
 * 测试 3Wire 是否成功解码
*/
TEST(DecoderTestGroup, TestDecoderSuccess3Wire)
{
    uint8_t data[] = {0xC0, 0xDB, 0xDE, 0xDB, 0xDF, 0xC0};
    uint8_t expect[] = {0x11, 0x13};
}

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
