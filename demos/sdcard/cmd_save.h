// global len

void comando_save() {

   // send command byte
   send_byte_to_MCU(CMD_WRITE);
   if(TIMEOUT) return;

   // send filename
   send_string_to_MCU(filename);
   if(TIMEOUT) return;

   // get first response
   byte response = receive_byte_from_MCU();
   if(TIMEOUT) return;
   if(response == ERR_RESPONSE) {
      // error with file, print message
      print_string_response();
      return;
   }

   // send file size   
   //tmpword = ((word) *BASIC_HIMEM) - ((word)*BASIC_LOMEM) + 512;
   // in assembly:
   asm {
      sec
      lda BASIC_HIMEM
      sbc BASIC_LOMEM
      sta tmpword
      lda BASIC_HIMEM+1
      sbc BASIC_LOMEM+1
      sta tmpword+1            
      inc tmpword+1
      inc tmpword+1
   }

   send_word_to_mcu();
   if(TIMEOUT) return;

   // send actual bytes

   // "A1" header 
   send_byte_to_MCU('A');
   send_byte_to_MCU('1');
   if(TIMEOUT) return;
   
   // lowmem + stack chuck
   for(token_ptr=(byte *)2; token_ptr<=(byte *)0x1ff; token_ptr++) {
      send_byte_to_MCU(*token_ptr);
      if(TIMEOUT) return;
   }

   // basic data
   tmpword = (word) *BASIC_HIMEM;
   for(token_ptr=*BASIC_LOMEM; token_ptr<(byte *)tmpword; token_ptr++) {
      send_byte_to_MCU(*token_ptr);
      if(TIMEOUT) return;

      #ifdef LOADING_DOTS
      if(((byte)token_ptr) == 0) woz_putc('.');
      #endif
   }

   // get second response
   response = receive_byte_from_MCU();
   if(TIMEOUT) return;
   if(response == ERR_RESPONSE) {
      // error with file, print message
      print_string_response();
      return;
   }

   bas_file_info();
}
