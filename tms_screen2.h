byte SCREEN2_TABLE[8] = {
   0x02, 0xc0, 0x0e, 0xff, 0x03, 0x76, 0x03, 0x25
};

// SCREEN 2 VALUES

// pattern table:      $0000-$17FF   (256*8*3)
// sprite patterns:    $1800-$19FF
// color table:        $2000-$27FF   (256*8*3)
// name table:         $3800-$3AFF   (32*24 = 256*3 = 768)
// sprite attributes:  $3B00-$3BFF
// unused              $3C00-$3FFF
//

const word SCREEN2_PATTERN_TABLE   = 0x0000;
const word SCREEN2_NAME_TABLE      = 0x3800;
const word SCREEN2_COLOR_TABLE     = 0x2000;
const word SCREEN2_SPRITE_PATTERNS = 0x1800;
const word SCREEN2_SPRITE_ATTRS    = 0x3b00;
const word SCREEN2_SIZE            = (32*24);

void SCREEN2_FILL() {
   // fills name table x3 with increasing numbers
   set_vram_write_addr(SCREEN2_NAME_TABLE);
   for(word i=0;i<SCREEN2_SIZE;i++) {
      TMS_WRITE_DATA_PORT(i & 0xFF);
   }

   // fill pattern table with 0 (clear screen)
   set_vram_write_addr(SCREEN2_PATTERN_TABLE);
   for(word i=768*8;i!=0;i--) {
      TMS_WRITE_DATA_PORT(0);
   }

   // fill color table with black on white
   set_vram_write_addr(SCREEN2_COLOR_TABLE);
   for(word i=768*8;i!=0;i--) {
      TMS_WRITE_DATA_PORT(COLOR_BYTE(COLOR_BLACK,COLOR_WHITE));
   }
}

void SCREEN2_PUTC(byte ch, byte x, byte y, byte col) {
   byte *source = &FONT[(word)(ch-32)*8];
   word addr = x*8 + y*256;
   set_vram_write_addr(SCREEN2_PATTERN_TABLE + addr); for(byte i=0;i<8;i++) TMS_WRITE_DATA_PORT(source[i]);
   set_vram_write_addr(SCREEN2_COLOR_TABLE   + addr); for(byte i=0;i<8;i++) TMS_WRITE_DATA_PORT(col);
}

void SCREEN2_PUTS(byte x, byte y, byte col, char *s) {
   byte c;
   while(c=*s++) {
      SCREEN2_PUTC(c, x++, y, col);
   }
}

#define PLOT_MODE_RESET   0
#define PLOT_MODE_SET     1
#define PLOT_MODE_INVERT  2

byte SCREEN2_PLOT_MODE = PLOT_MODE_SET;

void SCREEN2_PLOT(byte x, byte y) {
   byte pow2_table_reversed[8] = { 128,64,32,16,8,4,2,1 };
   word paddr = SCREEN2_PATTERN_TABLE + (word)(x & 0b11111000) + (word)(y & 0b11111000)*32 + y%8;
   set_vram_read_addr(paddr);
   byte data = TMS_READ_DATA_PORT;
   byte mask = pow2_table_reversed[x%8];
   set_vram_write_addr(paddr);
   switch(SCREEN2_PLOT_MODE) {
      case PLOT_MODE_RESET:
         data &= ~mask;
         break;
      case PLOT_MODE_SET:
         data |= mask;
         break;
      case PLOT_MODE_INVERT:
         data ^= mask;
         break;
   }
   TMS_WRITE_DATA_PORT(data);
}

void screen2_square_sprites() {
   // fills first sprite pattern with 255
   set_vram_write_addr(SCREEN2_SPRITE_PATTERNS);    // start writing in the sprite patterns
   for(byte i=0;i<8;i++) {
      TMS_WRITE_DATA_PORT(0);
   }

   // set sprite coordinates
   set_vram_write_addr(SCREEN2_SPRITE_ATTRS);       // start writing in the sprite attribute
   for(byte i=0;i<32;i++) {
      TMS_WRITE_DATA_PORT(0);      NOP; NOP; NOP; NOP; // y coordinate
      TMS_WRITE_DATA_PORT(0);      NOP; NOP; NOP; NOP; // x coordinate
      TMS_WRITE_DATA_PORT(0);      NOP; NOP; NOP; NOP; // name
      TMS_WRITE_DATA_PORT(i);      NOP; NOP; NOP; NOP; // color
   }
}

void prova_screen2() {
   TMS_INIT(SCREEN2_TABLE);
   SCREEN2_FILL();
   screen2_square_sprites();

   SCREEN2_PUTS(0,0,COLOR_BYTE(COLOR_BLACK,COLOR_WHITE),"*** P-LAB  VIDEO CARD SYSTEM ***");
   SCREEN2_PUTS(0,2,COLOR_BYTE(COLOR_BLACK,COLOR_WHITE),"16K VRAM BYTES FREE");
   SCREEN2_PUTS(0,4,COLOR_BYTE(COLOR_BLACK,COLOR_WHITE),"READY.");

   for(byte i=0;i<16;i++) {
      SCREEN2_PUTS(5,(byte)(6+i),(byte)(((15-i)<<4)+i),"     SCREEN 2     ");
   }

   vti_line(18, 45,232,187);
   vti_line(18,187,232, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_RESET;

   vti_line(18+5, 45,232+5,187);
   vti_line(18+5,187,232+5, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_INVERT;

   vti_line(18+5+5, 45,232+5+5,187);
   vti_line(18+5+5,187,232+5+5, 45);

   SCREEN2_PLOT_MODE = PLOT_MODE_SET;

   //vti_ellipse_rect(7,9,202,167);
}

signed int vti_abs(signed int x) {
    return x < 0 ? -x : x;
}

// http://members.chello.at/~easyfilter/bresenham.html
void vti_line(byte _x0, byte _y0, byte _x1, byte _y1) {

   signed int x0 = (signed int) _x0;
   signed int x1 = (signed int) _x1;
   signed int y0 = (signed int) _y0;
   signed int y1 = (signed int) _y1;

   signed int dx =  vti_abs(x1-x0);
   signed int dy = -vti_abs(y1-y0);
   signed int err = dx+dy; /* error value e_xy */

   bool ix = x0<x1;
   bool iy = y0<y1;
   signed int e2;

   for(;;){  /* loop */
      SCREEN2_PLOT((byte)x0, (byte)y0);
      if (x0==x1 && y0==y1) break;
      e2 = err<<1;//2*err;
      if (e2 >= dy) { err += dy; if(ix) ++x0; else --x0; } /* e_xy+e_x > 0 */
      if (e2 <= dx) { err += dx; if(iy) ++y0; else --y0; } /* e_xy+e_y < 0 */
   }
}

/*
// http://members.chello.at/~easyfilter/bresenham.html
void vti_ellipse_rect(byte _x0, byte _y0, byte _x1, byte _y1)
{
    //unsigned int x0,y0,x1,y1;
    signed int x0 = (signed int) _x0;
    signed int y0 = (signed int) _y0;
    signed int x1 = (signed int) _x1;
    signed int y1 = (signed int) _y1;

    signed int a = vti_abs(x1-x0), b = vti_abs(y1-y0);
    signed int b1 = b&1; // values of diameter
    signed int dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a; // error increment
    signed int err = dx+dy+b1*a*a, e2; // error of 1.step

    if (x0 > x1) { x0 = x1; x1 += a; } // if called with swapped points
    if (y0 > y1) y0 = y1; // .. exchange them
    y0 += (b+1)/2; y1 = y0-b1;   // starting pixel
    a *= 8*a; b1 = 8*b*b;

    do {
        SCREEN2_PLOT((byte) x1, (byte) y0); //   I. Quadrant
        SCREEN2_PLOT((byte) x0, (byte) y0); //  II. Quadrant
        SCREEN2_PLOT((byte) x0, (byte) y1); // III. Quadrant
        SCREEN2_PLOT((byte) x1, (byte) y1); //  IV. Quadrant
        e2 = 2*err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }  // y step
        if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; } // x step
    } while (x0 <= x1);

    while (y0-y1 < b) {  // too early stop of flat ellipses a=1
        SCREEN2_PLOT((byte) x0-1, (byte) (y0)); // -> finish tip of ellipse
        SCREEN2_PLOT((byte) x1+1, (byte) (y0++));
        SCREEN2_PLOT((byte) x0-1, (byte) (y1));
        SCREEN2_PLOT((byte) x1+1, (byte) (y1--));
    }
}
*/
