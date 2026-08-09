#include <stdbool.h>
#include <stdint.h>
#include "epc/frame.h"

#define DELIM 0x00

/*
Decodes current frame from input buffer.
Caller passes input buffer and length and struct pointer
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

            if (bytes_to_next_codebyte != 0) return EPC_FRAME_ERR_MALFORMED;
            return frame_idx;
        }
        /* DELIMITER PROCESSING END */
    }  
    return EPC_FRAME_ERR_TRUNCATED; //runs beyond delimiter
}
