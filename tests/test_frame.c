#include "epc/frame.h"
#include "check.h"

int main(void)
{
    static const uint8_t wire[]     = {0x06,0x0B,0x11,0x0F,0xFF,0xFE,0x04,0x11,0x31,0xDC,0x00};
    static const uint8_t expected[] = {0x0B,0x11,0x0F,0xFF,0xFE,0x00,0x11,0x31,0xDC};
    uint8_t buf[9];
    int result;

    //test function
    result = epc_frame_decode(wire, (int)sizeof(wire), buf, (int)sizeof(buf));

    CHECK_EQ(result, (int)sizeof expected);
    return check_report();
}