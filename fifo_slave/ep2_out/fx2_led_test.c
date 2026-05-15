#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

// Standard Cypress FX2 default IDs. Change if your EEPROM uses custom values.
// 1337:0cdc
#define VENDOR_ID   0x1337  
#define PRODUCT_ID  0x0cdc  

// Target Endpoint: 0x02 means Endpoint 2, Direction: OUT (Host to Device)
#define EP2_OUT     0x02    
#define TIMEOUT_MS  1000    // 1 second timeout

int main() {
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle = NULL;
    int result;

    // 1. Initialize libusb
    result = libusb_init(&ctx);
    if (result < 0) {
        fprintf(stderr, "Error: Failed to initialize libusb (%d)\n", result);
        return 1;
    }

    // 2. Open the CY7C68013A device
    dev_handle = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    if (!dev_handle) {
        fprintf(stderr, "Error: Could not find or open CY7C68013A device [0x%04X:0x%04X].\n", VENDOR_ID, PRODUCT_ID);
        fprintf(stderr, "Did you run with 'sudo' or set up udev rules?\n");
        libusb_exit(ctx);
        return 1;
    }
    printf("Successfully connected to CY7C68013A device.\n");

    // 3. Detach kernel driver if attached (standard practice for raw USB devices on Linux)
    if (libusb_kernel_driver_active(dev_handle, 0) == 1) {
        printf("Kernel driver active. Detaching...\n");
        libusb_detach_kernel_driver(dev_handle, 0);
    }

    // 4. Claim Interface 0
    result = libusb_claim_interface(dev_handle, 0);
    if (result < 0) {
        fprintf(stderr, "Error: Failed to claim interface 0 (%d)\n", result);
        libusb_close(dev_handle);
        libusb_exit(ctx);
        return 1;
    }

    // 5. Interactive Transmission Loop
    printf("\n=== CY7C68013A to Cyclone 10 LP LED Controller ===\n");
    printf("Enter an 8-bit value (0-255 or 0x00-0xFF) to write.\n");
    printf("Type 'exit' or press Ctrl+C to quit.\n\n");

    char input[32];
    unsigned char data_byte;
    int transferred = 0;

    while (1) {
        printf("Enter value: ");
        if (!fgets(input, sizeof(input), stdin)) break;

        // Remove newline character
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Parse input as either hex (0xFF) or base-10 integer
        int parsed_val;
        if (strncmp(input, "0x", 2) == 0 || strncmp(input, "0X", 2) == 0) {
            result = sscanf(input, "%x", &parsed_val);
        } else {
            result = sscanf(input, "%d", &parsed_val);
        }

        if (result != 1 || parsed_val < 0 || parsed_val > 255) {
            printf("Invalid input! Please enter an integer between 0 and 255.\n");
            continue;
        }

        data_byte = (unsigned char)parsed_val;

        unsigned char payload[2] = { data_byte, 0x00 };

        // 6. Execute USB Bulk Transfer (Sending 1 Byte to EP2)
        // for
        result = libusb_bulk_transfer(dev_handle, EP2_OUT, payload, 2, &transferred, TIMEOUT_MS);
        
        if (result == 0 && transferred == 2) {
            printf("Sent: 0x%02X (Binary: %d%d%d%d %d%d%d%d) -> Top 4 bits for LEDs: %d%d%d%d\n",
                   data_byte,
                   (data_byte >> 7) & 1, (data_byte >> 6) & 1, (data_byte >> 5) & 1, (data_byte >> 4) & 1,
                   (data_byte >> 3) & 1, (data_byte >> 2) & 1, (data_byte >> 1) & 1, data_byte & 1,
                   (data_byte >> 7) & 1, (data_byte >> 6) & 1, (data_byte >> 5) & 1, (data_byte >> 4) & 1);
        } else {
            fprintf(stderr, "Error: Bulk transfer failed or timed out (%s)\n", libusb_error_name(result));
        }
    }

    // 7. Clean up and Release
    libusb_release_interface(dev_handle, 0);
    libusb_close(dev_handle);
    libusb_exit(ctx);
    printf("Context cleaned. Exiting safely.\n");
    return 0;
}
