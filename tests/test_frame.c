#include "epc/frame.h"
#include "check.h"
#include <string.h>

int main(void)
{
    //always use max buffer size
    uint8_t buf[EPC_FRAME_MAX_ENCODED+1]; //262 to test all scenarios, including where buffer is completely filled
    const uint8_t initial = 0xAC;
    const uint8_t nonzero = 0x01;
    
    /* ENCODER FUNCTION TESTS */
    PUT THEM IN FUNCTIONS THAT MAIN CALLS
    //NULL payload, zero length
    static const uint8_t empty_payload_expected[]      = {0x03, 0xFF, 0xFF, 0x00}; //codebyte, 2x CRC bytes (0xFFFF) for NULL payload, delim
    memset(buf, initial, sizeof(buf)); //set all buffer to value to check against
    result = epc_frame_encode(NULL, 0, buf, sizeof(buf)); //test null payload
    CHECK_EQ(result, (int)sizeof empty_payload_expected); //check size is correct
    CHECK_EQ(memcmp(buf, empty_payload_expected, sizeof(empty_payload_expected)), 0); //check data is correct
    CHECK_EQ(buf[sizeof empty_payload_expected], initial); //check no extra writes
    
    //payload single zero
    static const uint8_t single_zero_payload[]          = {0x00};
    static const uint8_t single_zero_payload_expected[] = {0x01, 0x03, 0xE1, 0xF0, 0x00}; //codebyte, databyte replace, CRC for 0x00, delim
    memset(buf, initial, sizeof(buf)); //reset all buffer to value to check against
    result = epc_frame_encode(single_zero_payload, 1, buf, sizeof(buf)); //test single zero payload
    CHECK_EQ(result, (int)sizeof single_zero_payload_expected); //check size is correct
    CHECK_EQ(memcmp(buf, single_zero_payload_expected, sizeof(single_zero_payload_expected)), 0); //check data is correct
    CHECK_EQ(buf[sizeof single_zero_payload_expected], initial); //check no extra writes

    //normal payload
    static const uint8_t normal_payload[]          = {0x11, 0x22, 0x33, 0x44, 0x00, 0x55};
    static const uint8_t normal_payload_expected[] = {0x05, 0x11, 0x22, 0x33, 0x44, 0x04, 0x55, 0x61, 0x0B, 0x00}; //codebyte, payload w/ codebyte, CRC, delim
    memset(buf, initial, sizeof(buf)); //reset all buffer to value to check against
    result = epc_frame_encode(normal_payload, sizeof(normal_payload), buf, sizeof(buf)); //test normal payload
    CHECK_EQ(result, (int)sizeof normal_payload_expected); //check size is correct
    CHECK_EQ(memcmp(buf, normal_payload_expected, sizeof(normal_payload_expected)), 0); //check data is correct
    CHECK_EQ(buf[sizeof normal_payload_expected], initial); //check no extra writes

    //payload with 252 non-zero bytes (254 total bytes to encode, at COBS boundary)
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
    
    //payload with 256 non-zero bytes (258 total bytes to encode, at max payload limit)
    uint8_t max_payload[256];
    memset(max_payload, nonzero, sizeof(max_payload));
    memset(buf, initial, sizeof(buf)); //reset all buffer to value to check against
    result = epc_frame_encode(max_payload, sizeof(max_payload), buf, sizeof(buf)); //test 256 nonzero payload
    CHECK_EQ(result, EPC_FRAME_MAX_ENCODED); //check size is correct (max encoded size)
    CHECK_EQ(buf[0], 0xFF); //check codebyte is max
    CHECK_EQ(memcmp(&buf[1], max_payload, sizeof(max_payload)-2), 0); //check first 254 bytes of payload are unchanged
    CHECK_EQ(buf[255], 0x05); //check extra codebyte
    CHECK_EQ(memcmp(&buf[256], max_payload+254, 2), 0); //check last 2 bytes of payload are unchanged    
    CHECK_EQ(buf[258], 0x91); //check CRC matches expected
    CHECK_EQ(buf[259], 0xDC); //check CRC matches
    CHECK_EQ(buf[260], 0x00); //check delim
    CHECK_EQ(buf[261], initial); //check delim
    
    //payload at 257 bytes (one over max payload limit)
    //payload larger than out_cap size limit
    //payload at exactly out_cap size limit
    //null ptrs (payload  NULL with payload empty, payload not empty, output buf NULL)


    result = epc_frame_decode(wire, (int)sizeof(wire), buf, (int)sizeof(buf));

    CHECK_EQ(result, (int)sizeof expected);
    return check_report();
}
