"""
frame_rx.py: read EPC frames from the target board over serial and print payloads

Usage: python frame_rx.py COM5 (or run with no argument to be prompted)
Frame format: COBS-encoded [payload][CRC-16/CCITT-FALSE, big-endian], 0x00 delimiter
"""
import serial, sys, binascii
from cobs import cobs

def main():
    # Port from the command line if given, otherwise ask for it
    if len(sys.argv) < 2:
        port = "COM" + input("COM port: ")
    else:
        port = sys.argv[1]

    # Must match the board's UART settings: 115200 baud, 8N1 (default in PySerial)
    # Timeout of 1 sec so it doesn't gate
    uart = serial.Serial(port, 115200, timeout=1)

    # Bytes collected since the last delimiter (i.e. a COBS frame)
    frame = bytearray()

    while True:
        byte = uart.read()

        # If timeout, continue to wait for next byte
        if not byte:
            continue

        # Get current byte if not delimiter
        elif byte != b'\x00':
            frame += byte

        # Delimiter detected
        else:
            # Skip empty frames
            if frame:
                try:
                    # Grab decoded frame (including CRC)
                    decoded = cobs.decode(frame)
                except cobs.DecodeError:
                    print("DECODE ERROR")
                else:
                    # Runs only if decode succeeded
                    # CRC over payload + big-endian CRC is 0 for correct frame
                    if binascii.crc_hqx(decoded, 0xFFFF) == 0:
                        print(decoded[:-2].hex(" "))   # frame only, CRC stripped
                    else:
                        print("CRC MISMATCH")
            # Start fresh for the next frame
            frame = bytearray()


if __name__ == "__main__":
    main()
