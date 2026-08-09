#include <stdbool.h>
#include <stdint.h>
#include "epc/frame.h"

#define DELIM 0x00

int main(){
    int len = 10;
    uint8_t input_buf[256];
    uint8_t frame_buf[EPC_FRAME_MAX_DECODED];
    int frame_idx = 0;
    int bytes_to_next_codebyte = 0x00;
    bool frame_start = true;
    bool prev_ff = false;
    bool overflow = false;

    for(int i = 0; i < len; i++)
    {   
        uint8_t current_byte = input_buf[i];
        
        /* FRAME PROCESSING BEGIN */
        if(current_byte != DELIM)
        {
            if(overflow) continue;

            /* CODE BYTE PROCESSING BEGIN */
            if(bytes_to_next_codebyte == 0x00)
            {
                // insert zero if not start of frame
                if(!frame_start && !prev_ff)
                {   
                    if(frame_idx < EPC_FRAME_MAX_DECODED)
                    {
                        frame_buf[frame_idx++] = 0x00;
                    }
                    else overflow = true;
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
                if(frame_idx < EPC_FRAME_MAX_DECODED)
                {
                    frame_buf[frame_idx++] = current_byte;
                }
                else overflow = true;
            }
            /* DATA BYTE PROCESSING END */

            bytes_to_next_codebyte--; // decrement bytes to next codebyte4
            /* FRAME PROCESSING END */
        }

        /* DELIMITER PROCESSING BEGIN */
        else
        {
            if (overflow)
            {
                /* FRAMING ERROR */
            }
            else if (bytes_to_next_codebyte != 0)
            {
                /* FRAMING ERROR */
            }
            else{
                /* DELIVER FRAME */
            }
            frame_start = true;
            frame_idx = 0;
            bytes_to_next_codebyte = 0;
            prev_ff = false;
            overflow = false;
        }
        /* DELIMITER PROCESSING END */
    }   
}
