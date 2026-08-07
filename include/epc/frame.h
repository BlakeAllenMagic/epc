#ifndef EPC_FRAME_H
#define EPC_FRAME_H

#include <stdint.h>
#include <stddef.h>

#define EPC_CEIL_DIV(n, d) (((n) + (d) - 1) / (d))

#define EPC_FRAME_MAX_PAYLOAD       256
#define EPC_FRAME_CRC_LEN           2
#define EPC_FRAME_COBS_DELIM_LEN    1
#define EPC_FRAME_MAX_DECODED       (EPC_FRAME_MAX_PAYLOAD + EPC_FRAME_CRC_LEN)
#define EPC_FRAME_COBS_OVERHEAD     EPC_CEIL_DIV(EPC_FRAME_MAX_DECODED, 254)
#define EPC_FRAME_COBS_MAX_ENCODED  (EPC_FRAME_MAX_DECODED + EPC_FRAME_COBS_OVERHEAD + EPC_FRAME_COBS_DELIM_LEN)

_Static_assert(EPC_FRAME_COBS_MAX_ENCODED == 261, "encoded size arithmetic changed");

typedef struct {
    uint8_t buf[EPC_FRAME_COBS_MAX_ENCODED];
    size_t  len;
    int     bad;
} epc_frame_decoder;

//ENCODER 
// it should refuse rather than truncate. Incomplete frame is bad.
// output is complete and ready to hand to UART
int epc_frame_encode(const uint8_t *payload, size_t len, uint8_t *out, size_t out_cap);

//DECODER
//init struct
void epc_frame_decodeer_init(epc_frame_decoder *d);

// state machine accepts arbitrary chunks of bytes
// reports completed frames as it finds them
// return > 0 : valid frame completed, out holds this many payload bytes
// return 0   : input consumed, no frame yet.
// return < 0 : error
int epc_frame_decode(epc_frame_decoder *d,
                    const uint8_t *in, size_t in_len,
                    uint8_t *out, size_t out_cap,
                    size_t *consumed);

#endif /* FRAME_H */