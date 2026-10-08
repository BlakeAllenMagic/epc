#include "epc/frame.h"
#include "check.h"

int main(void)
{
    static const uint8_t wire_std[]     = {0x06,0x0B,0x11,0x0F,0xFF,0xFE,0x04,0x11,0x31,0xDC,0x00};
    static const uint8_t expected_std[] = {0x0B,0x11,0x0F,0xFF,0xFE,0x00,0x11,0x31,0xDC};
    static const uint8_t wire_empty[] = {};

    uint8_t buf[9];
    int result;

    //encoder function tests
    //payload empty
    //payload single zero
    //normal payload
    //payload with 252 non-zero bytes (254 total bytes to encode, at COBS boundary)
    //payload with 256 non-zero bytes (258 total bytes to encode, at max payload limit)
    //payload at 257 bytes (one over max payload limit)
    //payload larger than out_cap size limit
    //payload at exactly out_cap size limit
    //null ptrs (payload  NULL with payload empty, payload not empty, output buf NULL)


    result = epc_frame_decode(wire, (int)sizeof(wire), buf, (int)sizeof(buf));

    CHECK_EQ(result, (int)sizeof expected);
    return check_report();
}