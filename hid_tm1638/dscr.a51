.module DEV_DSCR 

; descriptor types
; same as setupdat.h
DSCR_DEVICE_TYPE=1
DSCR_CONFIG_TYPE=2
DSCR_STRING_TYPE=3
DSCR_INTERFACE_TYPE=4
DSCR_ENDPOINT_TYPE=5
DSCR_DEVQUAL_TYPE=6

; for the repeating interfaces
DSCR_INTERFACE_LEN=9
DSCR_ENDPOINT_LEN=7

; endpoint types
ENDPOINT_TYPE_CONTROL=0
ENDPOINT_TYPE_ISO=1
ENDPOINT_TYPE_BULK=2
ENDPOINT_TYPE_INT=3

    .globl _dev_dscr, _dev_qual_dscr, _highspd_dscr, _fullspd_dscr, _dev_strings, _dev_strings_end
    .area DSCR_AREA (CODE)

  _dev_dscr:
  .db dev_dscr_end-_dev_dscr      ; len
  .db DSCR_DEVICE_TYPE            ; type
  .dw 0x0002                      ; usb 2.0
  .db 0x00                        ; class : HID
  .db 0x00                        ; subclass
  .db 0x00                        ; protocol 
  .db 64                          ; packet size (ep0)
  ;;.dw 0xB404                    ; vendor id 
  ;;.dw 0x1386                    ; product id
  .db 0x37, 0x13                  ; Vendor ID  (0x1337)
  .db 0xDC, 0x0C                  ; Product ID (0x0CDC)
  .dw 0x0100                      ; version id
  .db 1                           ; manufacturure str idx    
  .db 2                           ; product str idx 
  .db 0                           ; serial str idx 
  .db 1                           ; n configurations
  dev_dscr_end:

_dev_qual_dscr:
 .db dev_qualdscr_end-_dev_qual_dscr
 .db DSCR_DEVQUAL_TYPE
 .dw 0x0002                               ; usb 2.0
 .db 0x00
 .db 0x00
 .db 0x00
 .db 64                                   ; max packet
 .db 1                                    ; n configs
 .db 0                                    ; extra reserved byte
dev_qualdscr_end:

_highspd_dscr:
 .db highspd_dscr_end-_highspd_dscr                      ; dscr len
 .db DSCR_CONFIG_TYPE
    ; can't use .dw because byte order is different
 .db (highspd_dscr_realend-_highspd_dscr) % 256           ; total length of config lsb
 .db (highspd_dscr_realend-_highspd_dscr) / 256           ; total length of config msb
 .db 1                                                    ; 1 interfaces
 .db 1                                                    ; config number
 .db 0                                                    ; config string
 .db 0x80                                                 ; attrs = bus powered, no wakeup
 .db 0x32                                                 ; max power = 100ma
highspd_dscr_end:
  ;; --- INTERFACE 0: HID Class ---
  .db DSCR_INTERFACE_LEN
  .db DSCR_INTERFACE_TYPE
  .db 0x00                    ; Interface Index
  .db 0x00                    ; Alt Setting
  .db 0x02                    ; 2 Endpoint (EP1 IN, EP1 OUT)
  .db 0x03                    ; Class: HID
  .db 0x00                    ; Subclass: 0 (No Boot)
  .db 0x00                    ; Protocol: 0 (None)
  .db 0x00                    ; String

  ;; --- HID DESCRIPTOR ---
  .db 0x09                    ; Length
  .db 0x21                    ; Descriptor Type: HID
  .db 0x11, 0x01              ; HID Class Spec 1.11
  .db 0x00                    ; Country Code
  .db 0x01                    ; Number of Class Descriptors
  .db 0x22                    ; Descriptor Type: REPORT
  .db 72                      ; *** REPORT DESCRIPTOR LENGTH (Matches our 72-byte array) ***
  .db 0x00                    ; msb

  ;; Endpoint 1 IN: Interrupt (Joystick/Buttons)
  .db DSCR_ENDPOINT_LEN
  .db DSCR_ENDPOINT_TYPE
  .db 0x81                    ; EP1 IN
  .db ENDPOINT_TYPE_INT       ; *** MUST BE INTERRUPT ***
  .db 0x40, 0x00              ; 64 bytes
  .db 0x01                    ; Polling interval (1ms)

  ;; Endpoint 2 OUT: Interrupt (LEDs/7-Segments)
  .db DSCR_ENDPOINT_LEN
  .db DSCR_ENDPOINT_TYPE
  .db 0x02                    ; EP2 OUT
  .db 0x03                    ; Attributes: INTERRUPT
  .db 0x00, 0x02              ; 512 bytes
  .db 0x01                    ; Polling interval (1ms)
highspd_dscr_realend:

.even
_fullspd_dscr:
 .db fullspd_dscr_end-_fullspd_dscr                 ; dscr len
 .db DSCR_CONFIG_TYPE
                                                    ; can't use .dw because byte order is different
 .db (fullspd_dscr_realend-_fullspd_dscr) % 256     ; total length of config lsb
 .db (fullspd_dscr_realend-_fullspd_dscr) / 256     ; total length of config msb
 .db 1                                              ; 1 interfaces
 .db 1                                              ; config number
 .db 0                                              ; config string
 .db 0x80                                           ; attrs = bus powered, no wakeup
 .db 0x32                                           ; max power = 100ma
fullspd_dscr_end:
  ;; --- INTERFACE 0: HID Class ---
  .db DSCR_INTERFACE_LEN
  .db DSCR_INTERFACE_TYPE
  .db 0x00                    ; Interface Index
  .db 0x00                    ; Alt Setting
  .db 0x02                    ; 2 Endpoint (EP1 IN, EP1 OUT)
  .db 0x03                    ; Class: HID
  .db 0x00                    ; Subclass: 0 (No Boot)
  .db 0x00                    ; Protocol: 0 (None)
  .db 0x00                    ; String

  ;; --- HID DESCRIPTOR ---
  .db 0x09                    ; Length
  .db 0x21                    ; Descriptor Type: HID
  .db 0x11, 0x01              ; HID Class Spec 1.11
  .db 0x00                    ; Country Code
  .db 0x01                    ; Number of Class Descriptors
  .db 0x22                    ; Descriptor Type: REPORT
  .db 56                      ; *** REPORT DESCRIPTOR LENGTH (Matches our 56-byte array) ***
  .db 0x00                    ; msb

  ;; Endpoint 1 IN: Interrupt (Joystick/Buttons)
  .db DSCR_ENDPOINT_LEN
  .db DSCR_ENDPOINT_TYPE
  .db 0x81                    ; EP1 IN
  .db ENDPOINT_TYPE_INT       ; *** MUST BE INTERRUPT ***
  .db 0x40, 0x00              ; 64 bytes
  .db 0x01                    ; Polling interval (1ms)

  ;; Endpoint 2 OUT: Interrupt (LEDs/7-Segments)
  .db DSCR_ENDPOINT_LEN
  .db DSCR_ENDPOINT_TYPE
  .db 0x02                    ; EP2 OUT
  .db 0x03                    ; Attributes: INTERRUPT
  .db 0x40, 0x00              ; 64 bytes
  .db 0x01                    ; Polling interval (1ms)
fullspd_dscr_realend:

.even
_dev_strings:

;; --- String 0: Language IDs ---
_string0:
    .db string0end-_string0
    .db DSCR_STRING_TYPE
    .db 0x09, 0x04              ; English (US)
string0end:

;; --- String 1: Manufacturer ("MBS CLass@") ---
_string1:
    .db string1end-_string1
    .db DSCR_STRING_TYPE
    .db 'M',0,'B',0,'S',0,' ',0,'C',0,'L',0,'a',0,'s',0,'s',0,'@',0
string1end:

;; --- String 2: Product ("MBS TM1638") ---
_string2:
    .db string2end-_string2
    .db 0x03                      ; DSCR_STRING_TYPE
    .db 'M',0,'B',0,'S',0,' ',0,'T',0,'M',0,'1',0,'6',0,'3',0,'8',0
string2end:

;; --- String 3: Serial Number (Optional, "0001") ---
_string3:
    .db string3end-_string3
    .db DSCR_STRING_TYPE
    .db '0',0,'0',0,'0',0,'1',0
string3end:

_dev_strings_end:
    .dw 0x0000
