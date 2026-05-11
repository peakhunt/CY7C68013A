#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    const char *device = "/dev/hidraw5";
    int fd = open(device, O_WRONLY);
    
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    uint8_t patterns[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    uint8_t report[10];

    printf("Sending 10-byte packets to %s...\n", device);

    while (1) {
        for (int i = 0; i < 8; i++) {
            // [Byte 0]: Linux HID dummy (Report ID 0)
            // [Byte 1]: Your firmware's Command/ID (0x00)
            // [Bytes 2-9]: Pattern data
            report[0] = 0x00;
            report[1] = 0x00;
            memset(&report[2], patterns[i], 8);

            if (write(fd, report, 10) != 10) {
                perror("Write error - check device connection");
                close(fd);
                return 1;
            }
            usleep(100000); // 0.2s delay
        }
    }

    close(fd);
    return 0;
}
