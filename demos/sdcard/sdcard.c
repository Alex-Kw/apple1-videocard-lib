// TODO comando CD
// TODO comando MKDIR
// TODO comando RMDIR
// TODO comando DEL
// TODO comando REN
// TODO comando COPY
// TODO comando JMP
// TODO comando RUN a vuoto
// TODO questione del puntino che va in timeout

#define APPLE1_USE_WOZ_MONITOR 1

#pragma start_address(0x8000)

#pragma zp_reserve(0x4a) 
#pragma zp_reserve(0x4b)
#pragma zp_reserve(0x4c)
#pragma zp_reserve(0x4d)
#pragma zp_reserve(0x4e)
#pragma zp_reserve(0x4f)

#pragma zp_reserve(0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f)
#pragma zp_reserve(0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f)
#pragma zp_reserve(0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f)
#pragma zp_reserve(0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f)
#pragma zp_reserve(0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f)
#pragma zp_reserve(0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f)
#pragma zp_reserve(0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf)
#pragma zp_reserve(0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf)
#pragma zp_reserve(0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf)
#pragma zp_reserve(0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf)
#pragma zp_reserve(0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef)
#pragma zp_reserve(0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff)

#include <utils.h>
#include <apple1.h>
#include <string.h>

byte *const PORTB = (byte *) 0xA000;  // port B register
byte *const PORTA = (byte *) 0xA001;  // port A register
byte *const DDRB  = (byte *) 0xA002;  // port A data direction register
byte *const DDRA  = (byte *) 0xA003;  // port B data direction register

#define CPU_STROBE(v) (*PORTB = (v))  /* CPU strobe is bit 0 OUTPUT */
#define MCU_STROBE    (*PORTB & 128)  /* MCU strobe is bit 7 INPUT  */

__address(3) byte TIMEOUT;
__address(4) word TIMEOUT_MAX = 0x1388;
__address(6) word TIMEOUT_CNT;

void wait_mcu_strobe(byte v) {   
   if(TIMEOUT) return;

   TIMEOUT_CNT = 0;
   while((v==0 && MCU_STROBE != 0) || (v!=0 && MCU_STROBE == 0)) {
      TIMEOUT_CNT++;
      if(TIMEOUT_CNT > TIMEOUT_MAX) {
         TIMEOUT = 1;
         return;
      }
   }   
}

void send_byte_to_MCU(byte data) {   
   *DDRA = 0xFF;        // set port A as output   
   *PORTA = data;       // deposit byte on the data port   
   CPU_STROBE(1);       // set strobe high   
   wait_mcu_strobe(1);  // wait for the MCU to read the data   
   CPU_STROBE(0);       // set strobe low   
   wait_mcu_strobe(0);  // wait for the MCU to set strobe low
}

// note: allocates 1 byte for return value
byte receive_byte_from_MCU() {
   *DDRA = 0;          // set port A as input
   CPU_STROBE(0);      // set listen
   wait_mcu_strobe(1); // wait for the MCU to deposit data
   byte data = *PORTA; // read data
   CPU_STROBE(1);      // set strobe high
   wait_mcu_strobe(0); // wait for the MCU to set strobe low
   CPU_STROBE(0);      // set strobe low
   return data;
}

void VIA_init() {
   *DDRB = 1;       // pin 1 is output (CPU_STROBE), pin7 is input (MCU_STROBE)
   CPU_STROBE(0);   // initial state
}

// send a string to the MCY (0 terminator is sent as well)
void send_string_to_MCU(char *msg) {
   while(1) {
      byte data = *msg++;
      send_byte_to_MCU(data);
      if(TIMEOUT) break;
      if(data == 0) break;
   }
}

// print a string sent by the MCU
void print_string_response() {
   while(1) {
      byte data = receive_byte_from_MCU();
      if(TIMEOUT) break;
      if(data == 0) break;  // string terminator
      else woz_putc(data);
   }
}

word receive_word_from_mcu() {
   word data;
   *((byte *)&data)     = receive_byte_from_MCU();
   *((byte *)(&data+1)) = receive_byte_from_MCU();
   return data;
}

void send_word_to_mcu(word data) {
   send_byte_to_MCU( *((byte *)&data)     );
   send_byte_to_MCU( *((byte *)(&data+1)) );
}

#include "console.h"

void main() {
//#ifdef APPLE1_JUKEBOX
//   apple1_eprom_init();
//#endif

   console();
}
