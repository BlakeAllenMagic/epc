#include "epc/frame.h"
#include "check.h"
#include <string.h>

static const uint8_t initial = 0xAC;
static const uint8_t nonzero = 0x01; //CRC checks assume this is 0x01. Update CRC values accordingly if changed

// tests encoding null pointer with empty payload 
static void test_encode_null_empty(void)
{
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1];
    int result;
    static const uint8_t empty_payload_expected[]      = {0x03, 0xFF, 0xFF, 0x00}; //codebyte, 2x CRC bytes (0xFFFF) for NULL payload, delim
    memset(buf, initial, sizeof(buf)); //set all buffer to value to check against
    result = epc_frame_encode(NULL, 0, buf, sizeof(buf)); //test null payload
    CHECK_EQ(result, (int)sizeof empty_payload_expected); //check size is correct
    CHECK_EQ(memcmp(buf, empty_payload_expected, sizeof(empty_payload_expected)), 0); //check data is correct
    CHECK_EQ(buf[sizeof empty_payload_expected], initial); //check no extra writes
}

// tests encoding null pointer with non-empty payload 
static void test_encode_null_nonempty(void)
{
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1];
    int result;
    memset(buf, initial, sizeof(buf)); //set all buffer to value to check against
    result = epc_frame_encode(NULL, 5, buf, sizeof(buf)); //test null pointer with length > 0
    CHECK_EQ(result, EPC_FRAME_ERR_NULL_PTR); // check error code returns correct
    CHECK_EQ(buf[0], initial); //check output buffer not written
}

// tests encoding null output pointer
static void test_encode_null_out(void)
{
    static const uint8_t payload[] = {0x01};
    int result;
    result = epc_frame_encode(payload, sizeof(payload), NULL, EPC_FRAME_MAX_ENCODED); //test null output buffer
    CHECK_EQ(result, EPC_FRAME_ERR_NULL_PTR); // check error code returns correct
}

// tests encoding oversized payload
static void test_encode_too_large(void)
{
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1];
    int result;
    uint8_t overlim[EPC_FRAME_MAX_PAYLOAD+1];
    memset(buf, initial, sizeof(buf)); //set all buffer to value to check against
    memset(overlim, nonzero, sizeof(overlim)); //initialize oversized payload
    result = epc_frame_encode(overlim, sizeof(overlim), buf, sizeof(buf)); //test over max payload size
    CHECK_EQ(result, EPC_FRAME_ERR_TOO_LARGE); // check error code returns correct
    CHECK_EQ(buf[0], initial); //check output buffer not written
}

// tests encoding output buffer too small
static void test_encode_overflow(void)
{
    uint8_t small_buf[EPC_FRAME_MAX_ENCODED-1]; //initial buffer to a byte too small for max payload
    int result;
    uint8_t payload[EPC_FRAME_MAX_PAYLOAD];
    memset(small_buf, initial, sizeof(small_buf)); //set all buffer to value to check against
    memset(payload, nonzero, sizeof(payload)); //initialize max payload
    result = epc_frame_encode(payload, sizeof(payload), small_buf, sizeof(small_buf)); //test overflow of buf
    CHECK_EQ(result, EPC_FRAME_ERR_OVERFLOW); // check error code returns correct
    CHECK_EQ(small_buf[0], initial); //check output buffer not written
}

// tests encoding single zero payload 
static void test_encode_single_zero(void)
{
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1];
    int result;
    static const uint8_t single_zero_payload[]          = {0x00};
    static const uint8_t single_zero_payload_expected[] = {0x01, 0x03, 0xE1, 0xF0, 0x00}; //codebyte, databyte replace, CRC for 0x00, delim
    memset(buf, initial, sizeof(buf)); //reset all buffer to value to check against
    result = epc_frame_encode(single_zero_payload, sizeof(single_zero_payload), buf, sizeof(buf)); //test single zero payload
    CHECK_EQ(result, (int)sizeof single_zero_payload_expected); //check size is correct
    CHECK_EQ(memcmp(buf, single_zero_payload_expected, sizeof(single_zero_payload_expected)), 0); //check data is correct
    CHECK_EQ(buf[sizeof single_zero_payload_expected], initial); //check no extra writes
}

// tests encoding normal payload
static void test_encode_normal(void)
{
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1];
    int result;
    static const uint8_t normal_payload[]          = {0x11, 0x22, 0x33, 0x44, 0x00, 0x55};
    static const uint8_t normal_payload_expected[] = {0x05, 0x11, 0x22, 0x33, 0x44, 0x04, 0x55, 0x61, 0x0B, 0x00}; //codebyte, payload w/ codebyte, CRC, delim
    memset(buf, initial, sizeof(buf)); //reset all buffer to value to check against
    result = epc_frame_encode(normal_payload, sizeof(normal_payload), buf, sizeof(buf)); //test normal payload
    CHECK_EQ(result, (int)sizeof normal_payload_expected); //check size is correct
    CHECK_EQ(memcmp(buf, normal_payload_expected, sizeof(normal_payload_expected)), 0); //check data is correct
    CHECK_EQ(buf[sizeof normal_payload_expected], initial); //check no extra writes
}

// tests encoding COBS boundary payload
static void test_encode_cobs_boundary(void)
{
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1];
    int result;
    uint8_t cobs_boundary_payload[252];
    memset(cobs_boundary_payload, nonzero, sizeof(cobs_boundary_payload));
    memset(buf, initial, sizeof(buf)); //reset all buffer to value to check against
    result = epc_frame_encode(cobs_boundary_payload, sizeof(cobs_boundary_payload), buf, sizeof(buf)); //test 252 nonzero payload
    CHECK_EQ(result, 256); //check size is correct
    CHECK_EQ(buf[0], 0xFF); //check codebyte is max
    CHECK_EQ(memcmp(&buf[1], cobs_boundary_payload, sizeof(cobs_boundary_payload)), 0); //check payload is unchanged
    CHECK_EQ(buf[253], 0x91); //check CRC matches expected
    CHECK_EQ(buf[254], 0x10); //check CRC matches
    CHECK_EQ(buf[255], 0x00); //check delim
    CHECK_EQ(buf[256], initial); //check no extra writes
}

// tests encoding max payload
static void test_encode_max(void)
{
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1];
    int result;
    uint8_t max_payload[256];
    memset(max_payload, nonzero, sizeof(max_payload));
    memset(buf, initial, sizeof(buf)); //reset all buffer to value to check against
    result = epc_frame_encode(max_payload, sizeof(max_payload), buf, sizeof(buf)-1); //test 256 nonzero payload (with encoder seeing true max buf size)
    CHECK_EQ(result, EPC_FRAME_MAX_ENCODED); //check size is correct (max encoded size)
    CHECK_EQ(buf[0], 0xFF); //check codebyte is max
    CHECK_EQ(memcmp(&buf[1], max_payload, sizeof(max_payload)-2), 0); //check first 254 bytes of payload are unchanged
    CHECK_EQ(buf[255], 0x05); //check extra codebyte
    CHECK_EQ(memcmp(&buf[256], max_payload+254, 2), 0); //check last 2 bytes of payload are unchanged    
    CHECK_EQ(buf[258], 0x91); //check CRC matches expected
    CHECK_EQ(buf[259], 0xDC); //check CRC matches
    CHECK_EQ(buf[260], 0x00); //check delim
    CHECK_EQ(buf[261], initial); //check no extra writes
}

// tests decoding null input buffer pointer, input empty
static void test_decode_null_empty(void)
{
    uint8_t frame[EPC_FRAME_MAX_DECODED+1];
    int result;
    memset(frame, initial, sizeof(frame)); //set all of frame to value to check against
    result = epc_frame_decode(NULL, 0, frame, sizeof(frame)); //test null input buffer
    CHECK_EQ(result, EPC_FRAME_NEED_MORE); //check return code is correct (need more data)
    CHECK_EQ(frame[0], initial); //check frame buffer hasn't been written to
}

int main(void)
{
    /* ENCODER FUNCTION TESTS */
    /* ENCODER FAILURE TESTS */
    test_encode_null_nonempty();
    test_encode_null_out();
    test_encode_too_large();
    test_encode_overflow();
    /* ENCODER SUCCESS TESTS*/
    test_encode_null_empty();
    test_encode_single_zero();
    test_encode_normal();
    test_encode_cobs_boundary();
    test_encode_max();

    /* DECODER FUNCTION TESTS */
    /* DECODER FAILURE TESTS*/
    test_decode_null_nonempty();
    test_decode_null_out();
    test_decode_overflow();
    test_decode_too_large();
    test_decode_delim_malformed();
    test_decode_too_short_malformed();
    test_decode_crc_mismatch();
    test_decode_only_delims_truncated();
    test_decode_no_delim_truncated();

    /* DECODER SUCCESS TESTS */
    test_decode_null_empty(); //this isn't an error, just means no data
    test_decode_empty_payload();
    test_decode_leading_delim();
    test_decode_single_zero();
    test_decode_normal();
    test_decode_cobs_boundary();
    test_decode_max();

    /* ROUND TRIP TEST */
    test_round_trip();
    return check_report();
}
