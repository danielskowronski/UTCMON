/*

  u8x8_d_ssd1322.c
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  


  SSD1322: 
    480 x 128 dot matrix
    16 gray scale
  
  
*/
#include "u8x8.h"

#ifdef UTCMON_SSD1322_PATCH
// WARN INSIDE(UTCMON_SSD1322_PATCH): temporary machine-generated code

volatile uint8_t  u8x8_inj_rand_a0   = 0;   // 1=pick random A0 A/B on each injection when first byte is 0xA0
volatile uint8_t  u8x8_inj_idx_min   = 0;   // inclusive
volatile uint8_t  u8x8_inj_idx_max   = 0;   // inclusive; if max>=min, random index in [min,max]
volatile uint16_t u8x8_inj_ns_min    = 0;   // inclusive
volatile uint16_t u8x8_inj_ns_max    = 0;   // inclusive; if max>=min, random ns in [min,max]
void u8x8_ssd1322_inject_rand_a0(uint8_t enable) {
  u8x8_inj_rand_a0 = enable ? 1 : 0;
}

volatile uint8_t u8x8_inj_randa0_force_b_en  = 0;
volatile uint8_t u8x8_inj_randa0_force_b_val = 0x11;

void u8x8_ssd1322_inject_randa0_force_b(uint8_t enable, uint8_t bval) {
  if (enable) {
    u8x8_inj_randa0_force_b_en  = 1;
    u8x8_inj_randa0_force_b_val = (bval == 0x11) ? 0x11 : 0x01; // clamp to legal values
  } else {
    u8x8_inj_randa0_force_b_en  = 0;
  }
}
void u8x8_ssd1322_inject_rand_idx(uint8_t min_idx, uint8_t max_idx) {
  if (min_idx > 31) min_idx = 31;
  if (max_idx > 31) max_idx = 31;
  if (max_idx < min_idx) { uint8_t t = min_idx; min_idx = max_idx; max_idx = t; }
  u8x8_inj_idx_min = min_idx;
  u8x8_inj_idx_max = max_idx;
}
void u8x8_ssd1322_inject_rand_ns(uint16_t min_ns, uint16_t max_ns) {
  if (max_ns < min_ns) { uint16_t t = min_ns; min_ns = max_ns; max_ns = t; }
  u8x8_inj_ns_min = min_ns;
  u8x8_inj_ns_max = max_ns;
}
volatile uint8_t  u8x8_inj_enable = 0;      // 0=off, 1=on
volatile uint8_t  u8x8_inj_at     = 12;     // index in 32-byte tile payload (0..31)
volatile uint16_t u8x8_inj_ns     = 200;    // guard time around DC edges (ns)
volatile uint8_t  u8x8_inj_len    = 3;      // how many bytes to inject (1..4)
volatile uint8_t  u8x8_inj_bytes[4] = { 0xA0, 0x06, 0x11, 0x00 }; // default: A0 A=06 B=11

volatile uint16_t u8x8_inj_permille = 0;    // 0=always when enabled, else 1..1000 per tile/band
volatile uint8_t  u8x8_inj_scope    = 0;    // 0=per tile, 1=per band
volatile uint8_t  u8x8_inj_once     = 0;    // 1=auto-disable after first injection
static   uint32_t u8x8_inj_rng      = 0x12345678; // xorshift32 state

static inline uint32_t u8x8_inj_rand_next(void) {
  uint32_t x = u8x8_inj_rng;
  x ^= x << 13; x ^= x >> 17; x ^= x << 5;
  if (x == 0) x = 0x6D2B79F5;
  u8x8_inj_rng = x;
  return x;
}

void u8x8_ssd1322_set_inject(uint8_t enable, uint8_t at_byte, const uint8_t *bytes, uint8_t len, uint16_t ns) {
  u8x8_inj_enable = enable ? 1 : 0;
  u8x8_inj_at     = (at_byte > 31) ? 31 : at_byte;
  if (bytes && len) {
    if (len > 4) len = 4;
    for (uint8_t i = 0; i < len; i++) u8x8_inj_bytes[i] = bytes[i];
    u8x8_inj_len = len;
  }
  u8x8_inj_ns = ns;
}

void u8x8_ssd1322_inject_cfg(uint8_t enable, uint16_t permille, uint8_t scope, uint8_t once, uint32_t seed) {
  u8x8_inj_enable   = enable ? 1 : 0;
  u8x8_inj_permille = (permille > 1000) ? 1000 : permille;
  u8x8_inj_scope    = (scope ? 1 : 0);
  u8x8_inj_once     = (once ? 1 : 0);
  if (seed) u8x8_inj_rng = seed;
}

void u8x8_ssd1322_inject_enable(uint8_t enable) {
  u8x8_inj_enable = enable ? 1 : 0;
}
#endif


static const uint8_t u8x8_d_ssd1322_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* ssd1322: display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1322_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* ssd1322: display off */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


/* interpret b as a monochrome bit pattern, write value 15 for high bit and value 0 for a low bit */
/* topbit (msb) is sent last */
/* example: b = 0x083 will send 0xff, 0x00, 0x00, 0xf0 */

/* 4 Jan 2017: I think this procedure not required any more. Delete? */
/*
static uint8_t u8x8_write_byte_to_16gr_device(u8x8_t *u8x8, uint8_t b)
{
  static uint8_t buf[4];
  static uint8_t map[4] = { 0, 0x00f, 0x0f0, 0x0ff };
  buf [3] = map[b & 3];
  b>>=2;
  buf [2] = map[b & 3];
  b>>=2;
  buf [1] = map[b & 3];
  b>>=2;
  buf [0] = map[b & 3];
  return u8x8_cad_SendData(u8x8, 4, buf);
}
*/


/*
  input:
    one tile (8 Bytes)
  output:
    Tile for SSD1325 (32 Bytes)
*/

static uint8_t u8x8_ssd1322_to32_dest_buf[32];

static uint8_t *u8x8_ssd1322_8to32(U8X8_UNUSED u8x8_t *u8x8, uint8_t *ptr)
{
  uint8_t v;
  uint8_t a,b;
  uint8_t i, j;
  uint8_t *dest;
  
  for( j = 0; j < 4; j++ )
  {
    dest = u8x8_ssd1322_to32_dest_buf;
    dest += j;
    a =*ptr;
    ptr++;
    b = *ptr;
    ptr++;
    for( i = 0; i < 8; i++ )
    {
      v = 0;
      if ( a&1 ) v |= 0xf0;
      if ( b&1 ) v |= 0x0f;
      *dest = v;
      dest+=4;
      a >>= 1;
      b >>= 1;
    }
  }
  
  return u8x8_ssd1322_to32_dest_buf;
}

static uint8_t *u8x8_ssd1322_4to32(U8X8_UNUSED u8x8_t *u8x8, uint8_t *ptr)
{
  uint8_t v;
  uint8_t a;
  uint8_t i, j;
  uint8_t *dest;
  
  for( j = 0; j < 4; j++ )
  {
    dest = u8x8_ssd1322_to32_dest_buf;
    dest += j;
    a =*ptr;
    ptr++;
    for( i = 0; i < 8; i++ )
    {
      v = 0;
      if ( a&1 ) v = 0xff;
      *dest = v;
      dest+=4;
      a >>= 1;
    }
  }
  
  return u8x8_ssd1322_to32_dest_buf;
}


uint8_t u8x8_d_ssd1322_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x; 
  uint8_t y, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* U8X8_MSG_DISPLAY_SETUP_MEMORY is handled by the calling function */
    /*
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_init_seq);
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_powersave1_seq);
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x0C1 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* ssd1322 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 2;		// only every 4th col can be addressed
      x += u8x8->x_offset;		
    
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      y *= 8;
    
      
      u8x8_cad_SendCmd(u8x8, 0x075 );	/* set row address, moved out of the loop (issue 302) */
      u8x8_cad_SendArg(u8x8, y);
      u8x8_cad_SendArg(u8x8, y+7);
      
      do
      {
        c = ((u8x8_tile_t *)arg_ptr)->cnt;
        ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
#ifdef UTCMON_SSD1322_PATCH
        uint8_t u8x8_inj_band_fired = 0;
#endif
        do
        {

    // FIXME: begin experiment
    //u8x8_cad_SendCmd(u8x8, 0x075 );	/* set row address, moved out of the loop (issue 302) */
    //u8x8_cad_SendArg(u8x8, y);
    //u8x8_cad_SendArg(u8x8, y+7);
    // FIXME: end experiment

    
	  u8x8_cad_SendCmd(u8x8, 0x015 );	/* set column address */
	  u8x8_cad_SendArg(u8x8, x );	/* start */
	  u8x8_cad_SendArg(u8x8, x+1 );	/* end */

          u8x8_cad_SendCmd(u8x8, 0x05c );
          {
            uint8_t *buf = u8x8_ssd1322_8to32(u8x8, ptr);
#ifdef UTCMON_SSD1322_PATCH // WARN INSIDE(UTCMON_SSD1322_PATCH): probabilistic mid-tile injector
            uint8_t do_inject = 0;
            if (u8x8_inj_enable) {
              if (u8x8_inj_permille == 0) {
                do_inject = (u8x8_inj_scope == 0) || (u8x8_inj_band_fired == 0);
              } else {
                uint16_t r = (uint16_t)(u8x8_inj_rand_next() % 1000U);
                if (r < u8x8_inj_permille)
                  do_inject = (u8x8_inj_scope == 0) || (u8x8_inj_band_fired == 0);
              }
            }
            if (do_inject) {
              uint8_t k;
              if (u8x8_inj_idx_max >= u8x8_inj_idx_min) {
                uint32_t r = u8x8_inj_rand_next();
                uint8_t span = (uint8_t)(u8x8_inj_idx_max - u8x8_inj_idx_min + 1);
                k = u8x8_inj_idx_min + (uint8_t)(r % span);
              } else k = u8x8_inj_at;
              if (k > 31) k = 31;

              uint16_t ns_eff;
              if (u8x8_inj_ns_max >= u8x8_inj_ns_min) {
                uint32_t r2 = u8x8_inj_rand_next();
                uint32_t span_ns = (uint32_t)u8x8_inj_ns_max - (uint32_t)u8x8_inj_ns_min + 1U;
                ns_eff = (uint16_t)(u8x8_inj_ns_min + (r2 % span_ns));
              } else ns_eff = u8x8_inj_ns;

              uint8_t inj_bytes_local[4];
              uint8_t inj_len_local = u8x8_inj_len;
              for (uint8_t i = 0; i < inj_len_local && i < 4; ++i) inj_bytes_local[i] = u8x8_inj_bytes[i];
              if (u8x8_inj_rand_a0 && inj_len_local >= 1 && inj_bytes_local[0] == 0xA0) {
                uint32_t r = u8x8_inj_rand_next();
                uint8_t A = 0;
                if (r & 0x00000001) A |= 0x01;
                if (r & 0x00000002) A |= 0x02;
                if (r & 0x00000004) A |= 0x04;
                if (r & 0x00000008) A |= 0x10;
                if (r & 0x00000010) A |= 0x20;
                uint8_t B;
                if (u8x8_inj_randa0_force_b_en) B = u8x8_inj_randa0_force_b_val; else B = (r & 0x00000100) ? 0x11 : 0x01;
                if (B == 0x11) A &= (uint8_t)~0x20;
                inj_bytes_local[0] = 0xA0; inj_bytes_local[1] = A; inj_bytes_local[2] = B; inj_len_local = 3;
              }

              if (k > 0) u8x8_cad_SendData(u8x8, k, buf);
              u8x8_gpio_SetDC(u8x8, 0);
              if (ns_eff) u8x8_gpio_Delay(u8x8, U8X8_MSG_DELAY_NANO, ns_eff);
              u8x8_cad_SendData(u8x8, inj_len_local, (uint8_t*)inj_bytes_local);
              if (ns_eff) u8x8_gpio_Delay(u8x8, U8X8_MSG_DELAY_NANO, ns_eff);
              u8x8_gpio_SetDC(u8x8, 1);
              if (ns_eff) u8x8_gpio_Delay(u8x8, U8X8_MSG_DELAY_NANO, ns_eff);
              u8x8_cad_SendData(u8x8, 32 - k, buf + k);
              u8x8_inj_band_fired = 1;
              if (u8x8_inj_once) u8x8_inj_enable = 0;
            } else {
              u8x8_cad_SendData(u8x8, 32, buf);
            }
#else
            u8x8_cad_SendData(u8x8, 32, buf);
#endif
          }
	  
	  //u8x8_cad_SendData(u8x8, 32, u8x8_ssd1322_8to32(u8x8, ptr));

    // FIXME: theory: DC is not flipped here so corruption happens

    //u8x8_cad_SendCmd(u8x8, 0x015 );	/* set column address */
	  //u8x8_cad_SendArg(u8x8, x );	/* start */
	  //u8x8_cad_SendArg(u8x8, x+1 );	/* end */
	  
          ptr += 8;
          x += 2;
          c--;
        } while( c > 0 );
        arg_int--;
      } while( arg_int > 0 );
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

/*=========================================================*/

static const uint8_t u8x8_d_ssd1322_256x64_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAA(0x0a0, 0x006, 0x011),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1322_256x64_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAA(0x0a0, 0x014, 0x011),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_ssd1322_256x64_display_info_original =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100,        /* SSD1322: 2 us */
  /* post_reset_wait_ms = */ 100,          /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,            /* SSD1322: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,           /* SSD1322: 20ns, but cycle time is 100ns, so use 100/2 */
  /* sck_clock_hz = */ 10000000UL,         /* default 10 MHz */
  /* spi_mode = */ 0,                      /* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,        /* SSD1322 cycle 300ns → 300/2 */
  /* tile_width = */ 32,                   /* 256 pixel, so we require 32 bytes for this */
  /* tile_height = */ 8,
  /* default_x_offset = */ 0x01c,          /* two pixels per byte (4bpp) */
  /* flipmode_x_offset = */ 0x01c,
  /* pixel_width = */ 256,
  /* pixel_height = */ 64
};

static const u8x8_display_info_t u8x8_ssd1322_256x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  /* post_chip_enable_wait_ns = */ 0,
  /* pre_chip_disable_wait_ns = */ 0,
  /* reset_pulse_width_ms = */ 0,        /* SSD1322: 2 us */
  /* post_reset_wait_ms = */ 0,          /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 0,            /* SSD1322: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 0,           /* SSD1322: 20ns, but cycle time is 100ns, so use 100/2 */
  /* sck_clock_hz = */ 10000000UL,         /* default 10 MHz */
  /* spi_mode = */ 0,                      /* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 0,
  /* write_pulse_width_ns = */ 0,        /* SSD1322 cycle 300ns → 300/2 */
  /* tile_width = */ 32,                   /* 256 pixel, so we require 32 bytes for this */
  /* tile_height = */ 8,
  /* default_x_offset = */ 0x01c,          /* two pixels per byte (4bpp) */
  /* flipmode_x_offset = */ 0x01c,
  /* pixel_width = */ 256,
  /* pixel_height = */ 64
};


static const uint8_t u8x8_d_ssd1322_256x64_init_seq[] = {
    
  U8X8_DLY(1),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(1),
  
  U8X8_CA(0xfd, 0x12),            	/* unlock */
  U8X8_C(0xae),		                /* display off */
  U8X8_CA(0xb3, 0x91),			/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
  U8X8_CA(0xca, 0x3f),			/* multiplex ratio 1/64 Duty (0x0F~0x3F) */  
  U8X8_CA(0xa2, 0x00),			/* display offset, shift mapping ram counter */  
  U8X8_CA(0xa1, 0x00),			/* display start line */  
  //U8X8_CAA(0xa0, 0x14, 0x11),	/* Set Re-Map / Dual COM Line Mode */  
  U8X8_CAA(0xa0, 0x06, 0x011),	/* Set Re-Map / Dual COM Line Mode */  
  U8X8_CA(0xab, 0x01),			/* Enable Internal VDD Regulator */  
  U8X8_CAA(0xb4, 0xa0, 0x005|0x0fd),	/* Display Enhancement A */  
  U8X8_CA(0xc1, 0x9f),			/* contrast */  
  U8X8_CA(0xc7, 0x0f),			/* Set Scale Factor of Segment Output Current Control */  
  U8X8_C(0xb9),		                /* linear grayscale */
  U8X8_CA(0xb1, 0xe2),			/* Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment */  
  U8X8_CAA(0xd1, 0x082|0x020, 0x020),	/* Display Enhancement B */  
  U8X8_CA(0xbb, 0x1f),			/* precharge  voltage */  
  U8X8_CA(0xb6, 0x08),			/* precharge  period */  
  U8X8_CA(0xbe, 0x07),			/* vcomh */  
  U8X8_C(0xa6),		                /* normal display */
  U8X8_C(0xa9),		                /* exit partial display */


  U8X8_DLY(1),					/* delay 2ms */

  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1322_nhd_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1322_256x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_ssd1322_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

/*=========================================================*/
/*
https://github.com/olikraus/u8g2/issues/2386
*/


static const u8x8_display_info_t u8x8_ssd1322_zjy_256x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1322: 2 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1322: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1322: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 10000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215), 10 MHz (issue 301) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,	/* SSD1322: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 32,		/* 256 pixel, so we require 32 bytes for this */
  /* tile_height = */ 8,
  /* default_x_offset = */ 0x018,	/* this is the byte offset (there are two pixel per byte with 4 bit per pixel) */
  /* flipmode_x_offset = */ 0x018,
  /* pixel_width = */ 256,
  /* pixel_height = */ 64
};


static const uint8_t u8x8_d_ssd1322_zjy_256x64_init_seq[] = {
    
  U8X8_DLY(1),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(1),
  
  U8X8_CA(0xfd, 0x12),            	/* unlock */
  U8X8_C(0xae),		                /* display off */
  U8X8_CA(0xb3, 0x91),			/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
  U8X8_CA(0xca, 0x3f),			/* multiplex ratio 1/64 Duty (0x0F~0x3F) */  
  U8X8_CA(0xa2, 0x00),			/* display offset, shift mapping ram counter */  
  U8X8_CA(0xa1, 0x00),			/* display start line */  
  //U8X8_CAA(0xa0, 0x14, 0x11),	/* Set Re-Map / Dual COM Line Mode */  
  U8X8_CAA(0xa0, 0x16, 0x011),	/* Set Re-Map / Dual COM Line Mode */  
  U8X8_CA(0xab, 0x01),			/* Enable Internal VDD Regulator */  
  U8X8_CAA(0xb4, 0xa0, 0x005|0x0fd),	/* Display Enhancement A */  
  U8X8_CA(0xc1, 0x9f),			/* contrast */  
  U8X8_CA(0xc7, 0x0f),			/* Set Scale Factor of Segment Output Current Control */  
  U8X8_C(0xb9),		                /* linear grayscale */
  U8X8_CA(0xb1, 0xe2),			/* Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment */  
  U8X8_CAA(0xd1, 0x082|0x020, 0x020),	/* Display Enhancement B */  
  U8X8_CA(0xbb, 0x1f),			/* precharge  voltage */  
  U8X8_CA(0xb6, 0x08),			/* precharge  period */  
  U8X8_CA(0xbe, 0x07),			/* vcomh */  
  U8X8_C(0xa6),		                /* normal display */
  U8X8_C(0xa9),		                /* exit partial display */


  U8X8_DLY(1),					/* delay 2ms */

  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1322_zjy_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1322_zjy_256x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_zjy_256x64_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_ssd1322_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}


/*=========================================================*/
/*
https://github.com/olikraus/u8g2/issues/2092
*/


static const u8x8_display_info_t u8x8_ssd1322_240x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1322: 2 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1322: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1322: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 10000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215), 10 MHz (issue 301) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,	/* SSD1322: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 30,		/* 256 pixel, so we require 32 bytes for this */
  /* tile_height = */ 16,
  /* default_x_offset = */ 24,	/* this is the byte offset (there are two pixel per byte with 4 bit per pixel) */
  /* flipmode_x_offset = */ 0x000,
  /* pixel_width = */ 240,
  /* pixel_height = */ 128
};


static const uint8_t u8x8_d_ssd1322_240x128_init_seq[] = {
    
  U8X8_DLY(1),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(1),
  
  U8X8_CA(0xfd, 0x12),            	/* unlock */
  U8X8_C(0xae),		                /* display off */
  U8X8_CA(0xb3, 0x91),			/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
  U8X8_CA(0xca, 0x7f),			/* multiplex ratio 1/128 Duty (0x0F~0x7F) */  
  U8X8_CA(0xa2, 0x00),			/* display offset, shift mapping ram counter */  
  U8X8_CA(0xa1, 0x00),			/* display start line */  
  //U8X8_CAA(0xa0, 0x14, 0x11),	/* Set Re-Map / Dual COM Line Mode */  
  U8X8_CAA(0xa0, 0x36, 0x011),	/* Set Re-Map / Dual COM Line Mode */  
  U8X8_CA(0xab, 0x01),			/* Enable Internal VDD Regulator */  
  U8X8_CAA(0xb4, 0xa0, 0x005|0x0fd),	/* Display Enhancement A */  
  U8X8_CA(0xc1, 0x9f),			/* contrast */  
  U8X8_CA(0xc7, 0x0f),			/* Set Scale Factor of Segment Output Current Control */  
  U8X8_C(0xb9),		                /* linear grayscale */
  U8X8_CA(0xb1, 0xe2),			/* Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment */  
  U8X8_CAA(0xd1, 0x082|0x020, 0x020),	/* Display Enhancement B */  
  U8X8_CA(0xbb, 0x1f),			/* precharge  voltage */  
  U8X8_CA(0xb6, 0x08),			/* precharge  period */  
  U8X8_CA(0xbe, 0x07),			/* vcomh */  
  U8X8_C(0xa6),		                /* normal display */
  U8X8_C(0xa9),		                /* exit partial display */


  U8X8_DLY(1),					/* delay 2ms */

  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1322_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1322_240x128_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_240x128_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_ssd1322_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}



/*=========================================================*/
/*

  Top Win OLED 240x128

  Discussion: https://github.com/olikraus/u8g2/discussions/2308
  Issue: https://github.com/olikraus/u8g2/issues/2310
  
  The main difference to the previous device seems to be the dual com line mode
  (0x0a0 command)

*/

static const uint8_t u8x8_d_ssd1322_topwin_240x128_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAA(0x0a0, 0x036, 0x001),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1322_topwin_240x128_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAA(0x0a0, 0x024, 0x001),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const u8x8_display_info_t u8x8_ssd1322_topwin_240x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1322: 2 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1322: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1322: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 10000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215), 10 MHz (issue 301) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,	/* SSD1322: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 30,		/* 240 pixel, so we require 30 bytes for this */
  /* tile_height = */ 16,
  /* default_x_offset = */ 24,	/* this is the byte offset (there are two pixel per byte with 4 bit per pixel) */
  /* flipmode_x_offset = */ 0x000,
  /* pixel_width = */ 240,
  /* pixel_height = */ 128
};


static const uint8_t u8x8_d_ssd1322_topwin_240x128_init_seq[] = {
    
  U8X8_DLY(1),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(1),
  
  U8X8_CA(0xfd, 0x12),            	/* unlock */
  U8X8_C(0xae),		                /* display off */
  U8X8_CA(0xb3, 0x91),			/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
  U8X8_CA(0xca, 0x7f),			/* multiplex ratio 1/128 Duty (0x0F~0x7F) */  
  U8X8_CA(0xa2, 0x00),			/* display offset, shift mapping ram counter */  
  U8X8_CA(0xa1, 0x00),			/* display start line */  
  U8X8_CAA(0xa0, 0x36, 0x001),	/* Set Re-Map / Dual COM Line Mode, https://github.com/olikraus/u8g2/discussions/2308 */  
  U8X8_CA(0xab, 0x01),			/* Enable Internal VDD Regulator */  
  U8X8_CAA(0xb4, 0xa0, 0x005|0x0fd),	/* Display Enhancement A */  
  U8X8_CA(0xc1, 0x9f),			/* contrast */  
  U8X8_CA(0xc7, 0x0f),			/* Set Scale Factor of Segment Output Current Control */  
  U8X8_C(0xb9),		                /* linear grayscale */
  U8X8_CA(0xb1, 0xe2),			/* Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment */  
  U8X8_CAA(0xd1, 0x082|0x020, 0x020),	/* Display Enhancement B */  
  U8X8_CA(0xbb, 0x1f),			/* precharge  voltage */  
  U8X8_CA(0xb6, 0x08),			/* precharge  period */  
  U8X8_CA(0xbe, 0x07),			/* vcomh */  
  U8X8_C(0xa6),		                /* normal display */
  U8X8_C(0xa9),		                /* exit partial display */


  U8X8_DLY(1),					/* delay 2ms */

  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_ssd1322_topwin_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1322_topwin_240x128_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_topwin_240x128_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_topwin_240x128_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_topwin_240x128_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_ssd1322_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}


/*=========================================================*/
/* 
  NHD-2.7-12864WDW3-M 
  http://www.newhavendisplay.com/nhd2712864wdw3m-p-9546.html
  http://www.newhavendisplay.com/specs/NHD-2.7-12864WDW3-M.pdf

  It looks like that only every second pixel is connected to the OLED
*/

uint8_t u8x8_d_ssd1322_common2(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x; 
  uint8_t y, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* U8X8_MSG_DISPLAY_SETUP_MEMORY is handled by the calling function */
    /*
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_256x64_init_seq);
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_powersave1_seq);
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x0C1 );
      u8x8_cad_SendArg(u8x8, arg_int );	/* ssd1322 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 2;		// only every 4th col can be addressed
      x *= 2;		// only every second pixel is used in the 128x64 NHD OLED 
    
      x += u8x8->x_offset;
    
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      y *= 8;
          
      u8x8_cad_SendCmd(u8x8, 0x075 );	/* set row address, moved out of the loop (issue 302) */
      u8x8_cad_SendArg(u8x8, y);
      u8x8_cad_SendArg(u8x8, y+7);
      
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;

	do
	{
	  u8x8_cad_SendCmd(u8x8, 0x015 );	/* set column address */
	  u8x8_cad_SendArg(u8x8, x );	/* start */
	  u8x8_cad_SendArg(u8x8, x+1 );	/* end */
	  u8x8_cad_SendCmd(u8x8, 0x05c );	/* write to ram */	  
	  u8x8_cad_SendData(u8x8, 32, u8x8_ssd1322_4to32(u8x8, ptr));	  
	  ptr += 4;
	  x += 2;
	  
	  u8x8_cad_SendCmd(u8x8, 0x015 );	/* set column address */
	  u8x8_cad_SendArg(u8x8, x );	/* start */
	  u8x8_cad_SendArg(u8x8, x+1 );	/* end */
	  u8x8_cad_SendCmd(u8x8, 0x05c );	/* write to ram */	  
	  u8x8_cad_SendData(u8x8, 32, u8x8_ssd1322_4to32(u8x8, ptr));	  
	  ptr += 4;
	  x += 2;
	  
	  c--;
	} while( c > 0 );
	
	//x += 2;
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}


static const uint8_t u8x8_d_ssd1322_128x64_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAA(0x0a0, 0x016, 0x011),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_ssd1322_128x64_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAA(0x0a0, 0x004, 0x011),		/* remap */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const u8x8_display_info_t u8x8_ssd1322_128x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 10,
  /* reset_pulse_width_ms = */ 100, 	/* SSD1322: 2 us */
  /* post_reset_wait_ms = */ 100, /* far east OLEDs need much longer setup time */
  /* sda_setup_time_ns = */ 50,		/* SSD1322: 15ns, but cycle time is 100ns, so use 100/2 */
  /* sck_pulse_width_ns = */ 50,	/* SSD1322: 20ns, but cycle time is 100ns, so use 100/2, AVR: below 70: 8 MHz, >= 70 --> 4MHz clock */
  /* sck_clock_hz = */ 10000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns, increased to 8MHz (issue 215), 10 MHz (issue 301) */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 10,
  /* write_pulse_width_ns = */ 150,	/* SSD1322: cycle time is 300ns, so use 300/2 = 150 */
  /* tile_width = */ 16,		/* 128 pixel */
  /* tile_height = */ 8,
  /* default_x_offset = */ 28,	/* this is the byte offset (there are two pixel per byte with 4 bit per pixel) */
  /* flipmode_x_offset = */ 28,
  /* pixel_width = */ 128,
  /* pixel_height = */ 64
};


static const uint8_t u8x8_d_ssd1322_128x64_init_seq[] = {
    
  U8X8_DLY(1),
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(1),
  
  U8X8_CA(0xfd, 0x12),            	/* unlock */
  U8X8_C(0xae),		                /* display off */
  U8X8_CA(0xb3, 0x91),			/* set display clock divide ratio/oscillator frequency (set clock as 80 frames/sec)  */  
  U8X8_CA(0xca, 0x3f),			/* multiplex ratio 1/64 Duty (0x0F~0x3F) */  
  U8X8_CA(0xa2, 0x00),			/* display offset, shift mapping ram counter */  

  U8X8_CA(0xa1, 0x00),			/* display start line */  
  U8X8_CA(0xab, 0x01),			/* Enable Internal VDD Regulator */  
/*
	A[0]=0b, Horizontal address increment [reset]    ***
	A[0]=1b, Vertical address increment
	
	A[1]=0b, Disable Column Address Re-map [reset]
	A[1]=1b, Enable Column Address Re-map		***
	
	A[2]=0b, Disable Nibble Re-map [reset]
	A[2]=1b, Enable Nibble Re-map			***
	
	A[4]=0b, Scan from COM0 to COM[N –1] [reset]	
	A[4]=1b, Scan from COM[N-1] to COM0, where N is the	***
	
	Multiplex ratio
	A[5]=0b, Disable COM Split Odd Even [reset]	***
	A[5]=1b, Enable COM Split Odd Even
	
	B[4], Enable / disable Dual COM Line mode
	0b, Disable Dual COM mode [reset]
	1b, Enable Dual COM mode (MUX ≤ 63)
	
	0x16 = 00010110
*/
  //U8X8_CAA(0xa0, 0x14, 0x11),	/* Set Re-Map / Dual COM Line Mode */  
  //U8X8_CAA(0xa0, 0x06, 0x011),	/* Set Re-Map / Dual COM Line Mode */  
  U8X8_CAA(0xa0, 0x16, 0x011),	/* Set Re-Map / Dual COM Line Mode (NHD-2.7-12864WDW3-M datasheet) */  
  U8X8_CA(0xc7, 0x0f),			/* Set Scale Factor of Segment Output Current Control */  
  U8X8_CA(0xc1, 0x9f),			/* contrast */  
  //U8X8_CA(0xb1, 0xe2),			/* Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment */  
  U8X8_CA(0xb1, 0xf2),			/* Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment (NHD-2.7-12864WDW3-M datasheet) */  
  U8X8_CA(0xbb, 0x1f),			/* precharge  voltage */    
  //U8X8_CAA(0xb4, 0xa0, 0x005|0x0fd),	/* Display Enhancement A */  
  U8X8_CAA(0xb4, 0xa0, 0x0fd),	/* Display Enhancement A (NHD-2.7-12864WDW3-M datasheet) */  
  U8X8_CA(0xbe, 0x04),			/* vcomh (NHD-2.7-12864WDW3-M datasheet) */  
  U8X8_C(0xb9),		                /* linear grayscale */
  //U8X8_CAA(0xd1, 0x082|0x020, 0x020),	/* Display Enhancement B */  
  //U8X8_CA(0xb6, 0x08),			/* precharge  period */  
  U8X8_C(0xa6),		                /* normal display */
  U8X8_C(0xa9),		                /* exit partial display */


  U8X8_DLY(1),					/* delay 2ms */

  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_ssd1322_nhd_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_ssd1322_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_128x64_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_128x64_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_ssd1322_128x64_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }
      break;
    
    default:
      return u8x8_d_ssd1322_common2(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}

