#include <stdbool.h>
#include <stdint.h>
#include "epc/frame.h"

#define DELIM 0x00

/*
Encodes the given payload into the EPC frame format and stores it in the output buffer.
Returns the length of the encoded frame on success, or a negative error code on failure.
*/
int epc_frame_encode(const uint8_t *payload, size_t payload_len, uint8_t *out_buf, size_t out_cap)
{
    // initialize counters
    uint16_t count_to_zero = 1;
    size_t codebyte = 0;
    size_t out_idx = 1;

    // check if payload length exceeds maximum allowed
    if(payload_len > EPC_FRAME_MAX_PAYLOAD)
        return EPC_FRAME_TOO_LARGE;

    // check if output buffer is large enough to hold the encoded frame
    if(EPC_FRAME_ENCODED_LEN(payload_len) > out_cap)
        return EPC_FRAME_ERR_OVERFLOW;

    /* PAYLOAD ENCODING BEGIN */
    for(size_t i = 0; i < payload_len; i++){

        //check if 0xFF count (doesn't use up an input byte)
        if(count_to_zero == 0xFF)
        {
            //store current count in last codebyte
            out_buf[codebyte] = count_to_zero;

            //load new codebyte at current output byte
            codebyte = out_idx;

            //increment output index
            out_idx++;

            //reset count to 1
            count_to_zero = 1;

        }

        //check if zero is present in payload 
        if(payload[i] == 0x00)
        {

            //store current count in last codebyte
            out_buf[codebyte] = count_to_zero;

            //load new codebyte at current output byte
            codebyte = out_idx;

            //reset count to 1
            count_to_zero = 1;
        }

        //if not zero, store data byte and increment counter
        else
        {
            out_buf[out_idx] = payload[i];
            count_to_zero++;
        }
        out_idx++;
    }
    
    //after payload, update last codebyte and add delimiter
    out_buf[codebyte] = count_to_zero;
    out_buf[out_idx] = 0x00;
    /* PAYLOAD PROCESSING END */
    return EPC_FRAME_OK; // return total frame length
}

/*
Decodes the EPC frame from the input buffer and stores the payload in the frame buffer.
Returns the length of the decoded frame on success, or a negative error code on failure.
*/
int epc_frame_decode(const uint8_t *input_buf, int in_len,
                        uint8_t *frame_buf, int frame_cap)
{
    int frame_idx = 0;
    int bytes_to_next_codebyte = 0x00;
    bool frame_start = true;
    bool prev_ff = false;
    // bool overflow = false;

    for(int i = 0; i < in_len; i++)
    {   
        uint8_t current_byte = input_buf[i];
        
        /* FRAME PROCESSING BEGIN */
        if(current_byte != DELIM)
        {
            // if(overflow) continue; // streaming implementation

            /* CODE BYTE PROCESSING BEGIN */
            if(bytes_to_next_codebyte == 0x00)
            {
                // insert zero if not start of frame
                if(!frame_start && !prev_ff)
                {   
                    if(frame_idx >= frame_cap) return EPC_FRAME_ERR_TOO_LARGE;
                    frame_buf[frame_idx++] = 0x00;
                    // else overflow = true; // streaming implementation
                }
                frame_start = false; // frame start check complete
                bytes_to_next_codebyte = current_byte; // store code byte as counter to next code byte
                prev_ff = (current_byte == 0xFF); // check for 0xFF byte
            }
            /* CODE BYTE PROCESSING END */
            
            /* DATA BYTE PROCESSING BEGIN */
            else
            {
                // store data byte
                if(frame_idx >= frame_cap) return EPC_FRAME_ERR_TOO_LARGE;
                frame_buf[frame_idx++] = current_byte;

                // else overflow = true; //streaming implentation
            }
            /* DATA BYTE PROCESSING END */

            bytes_to_next_codebyte--; // decrement bytes to next codebyte4
            /* FRAME PROCESSING END */
        }

        /* DELIMITER PROCESSING BEGIN */
        else
        {
            // streaming implementation
            // if (overflow)
            // {
            //     /* FRAMING ERROR */
            // }
            if (frame_start) return EPC_FRAME_ERR_MALFORMED; //empty frame
            if (bytes_to_next_codebyte != 0) return EPC_FRAME_ERR_MALFORMED; //detected code byte when not expected
            return frame_idx;
        }
        /* DELIMITER PROCESSING END */
    }  
    return EPC_FRAME_ERR_TRUNCATED; //runs beyond delimiter
}
