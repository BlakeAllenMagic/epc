#include "epc/crc16.h"
#include "check.h"

int main(void)
{
    const char test_data[] = "123456789"; //test string
    uint16_t crc;

    //test function
    crc = crc16_ccitt_false((const uint8_t *)test_data, sizeof(test_data) - 1);

    //123456789 crc should = 0x29B1, else assert
    CHECK_EQ(crc, 0x29B1);
    return check_report();
}