#ifndef EPC_FRAME_H
#define EPC_FRAME_H

#include <stdint.h>
#include <stddef.h>
// #include <stdbool.h>

#define EPC_CEIL_DIV(n, d) (((n) + (d) - 1) / (d))

#define EPC_FRAME_MAX_PAYLOAD       256
#define EPC_FRAME_CRC_LEN           2
#define EPC_FRAME_COBS_DELIM_LEN    1
#define EPC_FRAME_MAX_DECODED       (EPC_FRAME_MAX_PAYLOAD + EPC_FRAME_CRC_LEN)
#define EPC_FRAME_COBS_OVERHEAD     EPC_CEIL_DIV(EPC_FRAME_MAX_DECODED, 254)
#define EPC_FRAME_COBS_MAX_ENCODED  (EPC_FRAME_MAX_DECODED + EPC_FRAME_COBS_OVERHEAD + EPC_FRAME_COBS_DELIM_LEN)

_Static_assert(EPC_FRAME_COBS_MAX_ENCODED == 261, "encoded size arithmetic changed");

typedef enum {
    EPC_FRAME_ERR_TOO_LARGE     = -1, // exceeds frame cap
    EPC_FRAME_ERR_MALFORMED     = -2, // delimiter mid frame
    EPC_FRAME_ERR_TRUNCATED     = -3, // truncated frame
} epc_frame_status;


// /* STREAMING IMPLEMENTATION */
// typedef struct {
//     uint8_t frame_buf[EPC_FRAME_MAX_DECODED];
//     size_t  frame_idx;
//     int     bytes_to_next_codebyte; //int so impossible negative would be visible
//     bool    frame_start;
//     bool    prev_ff;
//     bool    overflow;
// } epc_frame_decoder;
//
// void epc_frame_decoder_init(epc_frame_decoder *d);

//ENCODER 
// it should refuse rather than truncate. Incomplete frame is bad.
// output is complete and ready to hand to UART
int epc_frame_encode(const uint8_t *payload, size_t len, uint8_t *out, size_t out_cap);

/* DECODER */
// return value > 0: frame length
// return value = 0: empty frame
// return value < 0: error
int epc_frame_decode(const uint8_t *input_buf, int in_len,
                        uint8_t *frame_buf, int frame_cap);

#endif /* EPC_FRAME_H */