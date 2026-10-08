#include <stdbool.h>
#include <stdint.h>
#include "epc/frame.h"
#include "epc/crc16.h"

#define DELIM 0x00

/* Encoder struct to manage encoder state. Needed to handle payload + CRC encoding easily */
typedef struct {
    uint8_t *out_buf;       // buffer for encoded bytes
    size_t   out_idx;       // output buffer index for writing encoded bytes
    size_t   codebyte;      // codebyte location for COBS
    uint16_t count_to_zero; // counter to place in codebyte when zero hits
} cobs_enc_t;

/* prototype for encoder helper */
static void cobs_put_byte(cobs_enc_t *ptr_s, uint8_t byte);

/*
Computes CRC (big-endian) and encodes the given payload via COBS and stores it in the output buffer.
Returns the length of the encoded frame on success, or a negative error code on failure.
*/
int epc_frame_encode(const uint8_t *payload, size_t payload_len, uint8_t *out_buf, size_t out_cap)
{
    // initialize counters
    // outbuf pointer, idx starts at 1 (codebyte is idx 0), codebyte starts at 0, count_to_zero starts at 1)
    cobs_enc_t s = {out_buf, 1, 0, 1};
    uint16_t crc;

    // check if payload length exceeds maximum allowed
    if(payload_len > EPC_FRAME_MAX_PAYLOAD)
        return EPC_FRAME_ERR_TOO_LARGE;

    // check if output buffer is large enough to hold the encoded frame
    if(EPC_FRAME_ENCODED_LEN(payload_len) > out_cap)
        return EPC_FRAME_ERR_OVERFLOW;

    // check if null input ptr
    if((payload_len > 0) && (payload == NULL))
        return EPC_FRAME_ERR_NULL_PTR;

    // check if null output ptr
    if(s.out_buf == NULL)
        return EPC_FRAME_ERR_NULL_PTR;

    // payload processing
    for(size_t i = 0; i < payload_len; i++){
        cobs_put_byte(&s, payload[i]);
    }
    
    // crc processing (big-endian)
    crc = crc16_ccitt_false(payload, payload_len);
    cobs_put_byte(&s, crc >> 8);  // put top byte first (big-endian)
    cobs_put_byte(&s, crc & 0xFF); // then bottom byte

    // after payload, update last codebyte and add delimiter
    s.out_buf[s.codebyte] = s.count_to_zero;
    s.out_buf[s.out_idx] = 0x00;

    return s.out_idx + 1; // return total frame length
}

/* 
Encoder helper function, forward scans one byte at a time for cobs encoding.
Uses state machine struct to hold state between calls.
 */
static void cobs_put_byte(cobs_enc_t *ptr_s, uint8_t byte)
{
//check if 0xFF count (doesn't use up an input byte)
    if(ptr_s->count_to_zero == 0xFF)
    {
        //store current count in last codebyte
        ptr_s->out_buf[ptr_s->codebyte] = ptr_s->count_to_zero;

        //load new codebyte at current output byte
        ptr_s->codebyte = ptr_s->out_idx;

        //increment output index
        ptr_s->out_idx++;

        //reset count to 1
        ptr_s->count_to_zero = 1;

    }

    //check if zero is present in payload 
    if(byte == 0x00)
    {

        //store current count in last codebyte
        ptr_s->out_buf[ptr_s->codebyte] = ptr_s->count_to_zero;

        //load new codebyte at current output byte
        ptr_s->codebyte = ptr_s->out_idx;

        //reset count to 1
        ptr_s->count_to_zero = 1;
    }

    //if not zero, store data byte and increment counter
    else
    {
        ptr_s->out_buf[ptr_s->out_idx] = byte;
        ptr_s->count_to_zero++;
    }
    ptr_s->out_idx++;
}

/*
Decodes the EPC frame from the input buffer and stores the payload in the frame buffer.
Returns the length of the decoded frame on success (minus CRC), or a negative error code on failure.
*/
int epc_frame_decode(const uint8_t *input_buf, size_t in_len,
                        uint8_t *frame_buf, size_t frame_cap)
{
    size_t frame_idx = 0;
    int bytes_to_next_codebyte = 0x00;
    bool frame_start = true;
    bool prev_ff = false;
    // bool overflow = false;

    // check if null input ptr
    if((in_len > 0) && (input_buf == NULL))
        return EPC_FRAME_ERR_NULL_PTR;

    // check if null output ptr
    if(frame_buf == NULL)
        return EPC_FRAME_ERR_NULL_PTR;

    // check if frame cap provided doesn't account for max frame)
    if(frame_cap < EPC_FRAME_MAX_DECODED)
        return EPC_FRAME_ERR_OVERFLOW;

    //encoded payload processing
    for(size_t i = 0; i < in_len; i++)
    {   
        uint8_t current_byte = input_buf[i];
        
        /* FRAME PROCESSING BEGIN */
        // if not at end
        if(current_byte != DELIM)
        {
            // if(overflow) continue; // streaming implementation

            // check for codebyte
            if(bytes_to_next_codebyte == 0x00)
            {
                // insert zero if not start of frame
                if(!frame_start && !prev_ff)
                {   
                    if(frame_idx >= EPC_FRAME_MAX_DECODED) return EPC_FRAME_ERR_TOO_LARGE; //incoming frame exceeds frame buffer
                    frame_buf[frame_idx++] = 0x00;
                    // else overflow = true; // streaming implementation
                }
                frame_start = false; // frame start check complete
                bytes_to_next_codebyte = current_byte; // store code byte as counter to next code byte
                prev_ff = (current_byte == 0xFF); // check for 0xFF byte
            }
            
            // data byte processing
            else
            {
                // store data byte
                if(frame_idx >= EPC_FRAME_MAX_DECODED) return EPC_FRAME_ERR_TOO_LARGE; //incoming frame exceeds frame buffer
                frame_buf[frame_idx++] = current_byte;

                // else overflow = true; //streaming implentation
            }

            bytes_to_next_codebyte--; // decrement bytes to next codebyte
        }

        // end of frame processing (if current idx points to DELIM)
        else
        {
            // streaming implementation
            // if (overflow)
            // {
            //     /* FRAMING ERROR */
            // }
            if(frame_start) continue; //ignore 00
            if(bytes_to_next_codebyte != 0) return EPC_FRAME_ERR_MALFORMED; //delimiter detected midframe
            if(frame_idx < 2) return EPC_FRAME_ERR_MALFORMED; //too short to hold CRC, bad data
            // check CRC, should equal 0 due to crc(payload + crc) = 0 in crc16_ccitt_false
            if(crc16_ccitt_false(frame_buf, frame_idx) != 0)
                return EPC_FRAME_ERR_CRC_MISMATCH;
                
    
            else return frame_idx - 2; //return payload length only, not crc
        }
        /* DELIMITER PROCESSING END */
    }  
    return EPC_FRAME_NEED_MORE; //no complete frame in input
}
