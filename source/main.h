// ##############
// # HEIN/HAPPE #
// ##############

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>       // nanosleep, gettime
#include <inttypes.h>   // uint8_t, ...
#include <sys/stat.h>   // mkdir, stat
#include <sys/types.h>  // mkdir, stat
#include <unistd.h>    // stat
#include <string.h>     // memcpy

#include "display.h"
#include "ops.h"

// 4194304Hz -> ~238ns * 70224 = 16740000ns
#define cycle_duration 16740000
#define NTH_CYCLE 70224
#define HEIGHT 144      // 18x8 px
#define WIDTH 160       // 20x8 px
#define VRAM_WIDTH 256  // 32x8 px
#define VRAM_HEIGHT 256 // 32x8 px

// BackGround Debug Window
#define BG_DBG_WINDOW_WIDTH 128
#define BG_DBG_WINDOW_HEIGHT 192

int64_t cpu_cycle;
int64_t ppu_cycle;
int64_t current_line_cycles;

void* reg_ptr[12];
uint8_t opcode;             // todo: remove
uint8_t extended_opcode;    // todo: remove

// temp registers

uint8_t t8[8];
uint16_t t16[8];
uint32_t t32[8];
uint8_t* t8p;

// the amount of pixels that have been pushed 
// out of the FiFo within the current line
uint32_t pixelcounter;

uint32_t fifo;
uint8_t VISIBLE_SPRITE_ARRAY[10];
uint32_t OUTPUT_ARRAY[WIDTH * HEIGHT];
uint8_t* MEM;
char* GAME_NAME;
FILE* LOG_OUTPUT;

#define F (*((uint8_t*) reg_ptr + 0))
#define A (*((uint8_t*) reg_ptr + 1))
#define C (*((uint8_t*) reg_ptr + 2))
#define B (*((uint8_t*) reg_ptr + 3))
#define E (*((uint8_t*) reg_ptr + 4))
#define D (*((uint8_t*) reg_ptr + 5))
#define L (*((uint8_t*) reg_ptr + 6))
#define H (*((uint8_t*) reg_ptr + 7))

#define SP (*((uint16_t*) reg_ptr + 8/2))
#define PC (*((uint16_t*) reg_ptr + 10/2))

#define BC (*((uint16_t*) reg_ptr + 2/2))
#define DE (*((uint16_t*) reg_ptr + 4/2))
#define HL (*((uint16_t*) reg_ptr + 6/2))

// Zero Flag
#define FLAG_Z ((F&0x80)>>7)
#define SET_FLAG_Z(X) F&=0x7F; F|=(X)<<7;

// Add/Sub Flag (BCD)
#define FLAG_N ((F&0x40)>>6)
#define SET_FLAG_N(X) F&=0xBF; F|=(X)<<6;

//Half Carry Flag
#define FLAG_H ((F&0x20)>>5)
#define SET_FLAG_H(X) F&=0xDF; F|=(X)<<5;

// Carry Flag
#define FLAG_C ((F&0x10)>>4)
#define SET_FLAG_C(X) F&=0xEF; F|=(X)<<4;

// 0x9800 - 0x9FFF
// todo: #6
#define BG_CUR_FRAME MEM[0x9800]
#define BG_MEM_MAP MEM[0x8000]

#define LY MEM[0xFF44]
#define LX pixelcounter

// SCY and SCX
#define SCY MEM[0xFF42]
#define SCX MEM[0xFF43]

#define BG_PALETTE_DATA MEM[0xFF47]

#define WINDOW_Y_COORDINATE MEM[0xFF4A]
#define WINDOW_X_COORDINATE MEM[0xFF4B]
#define SPRITE_Y_COORDINATE(x) (MEM[0xFE00+(x)*4])
#define SPRITE_X_COORDINATE(x) (MEM[0xFE01+(x)*4])
#define SPRITE_CHR_CODE(x) (MEM[0xFE02+(x)*4])
#define SPRITE_ATTR_FLAG(x) (MEM[0xFE03+(x)*4])

#define PRIORITY(x) MEM[0xFE03+(x)*4]&128
// flipped vertically
#define IS_FLIPPED_V(x) MEM[0xFE03+(x)*4]&64
// flipped horizontal
#define IS_FLIPPED_H(x) MEM[0xFE03+(x)*4]&32
#define PALETTE(x) MEM[0xFE03+(x)*4]&16

uint8_t interrupt_master_enable;

#define IME             (interrupt_master_enable&0x01)
#define ENABLE_IME      interrupt_master_enable|=0x01;
#define DISABLE_IME     interrupt_master_enable&=0xFE;

#define INTERRUPT_ENABLE        (MEM[0xFFFF])
#define INTERRUPT_FLAGS         (MEM[0xFF0F])
#define INTERRUPT_VBLANK        (INTERRUPT_FLAGS&0x01)
#define SET_INTERRUPT_VBLANK(X) INTERRUPT_FLAGS&=0x01; INTERRUPT_FLAGS|=(X);
#define INTERRUPT_LCD           (INTERRUPT_FLAGS&0x02)
#define SET_INTERRUPT_LCD(X)    INTERRUPT_FLAGS&=0x02; INTERRUPT_FLAGS|=(X)<<1;
#define INTERRUPT_TIMER         (INTERRUPT_FLAGS&0x04)
#define SET_INTERRUPT_TIMER(X)  INTERRUPT_FLAGS&=0x04; INTERRUPT_FLAGS|=(X)<<2;
#define INTERRUPT_SERIAL        (INTERRUPT_FLAGS&0x08)
#define SET_INTERRUPT_SERIAL(X) INTERRUPT_FLAGS&=0x08; INTERRUPT_FLAGS|=(X)<<3;
#define INTERRUPT_JOYPAD        (INTERRUPT_FLAGS&0x10)
#define SET_INTERRUPT_JOYPAD(X) INTERRUPT_FLAGS&=0x10; INTERRUPT_FLAGS|=(X)<<4;

#define ENABLE_VBLANK           INTERRUPT_ENABLE|=0x01;
#define DISABLE_VBLANK          INTERRUPT_ENABLE&=0xFE;
#define ENABLE_LCD              INTERRUPT_ENABLE|=0x02;
#define DISABLE_LCD             INTERRUPT_ENABLE&=0xFD;
#define ENABLE_TIMER            INTERRUPT_ENABLE|=0x04;
#define DISABLE_TIMER           INTERRUPT_ENABLE&=0xFB;
#define ENABLE_SERIAL           INTERRUPT_ENABLE|=0x08;
#define DISABLE_SERIAL          INTERRUPT_ENABLE&=0xF7;
#define ENABLE_JOYPAD           INTERRUPT_ENABLE|=0x10;
#define DISABLE_JOYPAD          INTERRUPT_ENABLE&=0xEF;

#define GENERATE_VBLANK         INTERRUPT_FLAGS|=0x01;if(IME&&INTERRUPT_ENABLE&0x01){DISABLE_IME;SP-=2;(((uint16_t*)MEM)+SP)=PC;PC=0x40;}
#define GENERATE_LCD            INTERRUPT_FLAGS|=0x02;if(IME&&INTERRUPT_ENABLE&0x02){DISABLE_IME;SP-=2;(((uint16_t*)MEM)+SP)=PC;PC=0x48;}
#define GENERATE_TIMER          INTERRUPT_FLAGS|=0x04;if(IME&&INTERRUPT_ENABLE&0x04){DISABLE_IME;SP-=2;(((uint16_t*)MEM)+SP)=PC;PC=0x50;}
#define GENERATE_SERIAL         INTERRUPT_FLAGS|=0x08;if(IME&&INTERRUPT_ENABLE&0x08){DISABLE_IME;SP-=2;(((uint16_t*)MEM)+SP)=PC;PC=0x58;}
#define GENERATE_JOYPAD         INTERRUPT_FLAGS|=0x10;if(IME&&INTERRUPT_ENABLE&0x10){DISABLE_IME;SP-=2;(((uint16_t*)MEM)+SP)=PC;PC=0x60;}

// Memory Map
#define MEM_INTERRUPT   (MEM+0x00)
#define MEM_ROM         (MEM+0x100)
#define MEM_USER_PROG   (MEM+0x150)
#define MEM_BANK0       (MEM+0x8000)
#define MEM_EXT_RAM     (MEM+0xA000)
#define MEM_RAM         (MEM+0xC000)
#define MEM_ECHO_RAM    (MEM+0xE000)
#define MEM_OAM         (MEM+0xFE00)
#define MEM_IO_PORTS    (MEM+0xFF00)
#define MEM_RAM2        (MEM+0xFF80)

#define TIMER_DIV       MEM[0xFF04]
// Timer Flags
#define TIMER_TIMA      MEM[0xFF05]
#define TIMER_TMA       MEM[0xFF06]
#define TIMER_TAC       MEM[0xFF07]

// LCD DISPLAY REGISTERS

#define LCD_CONTROL_REGISTER MEM[0xFF40]
#define GB_DISPLAY_STATUS_FLAG                  (LCD_CONTROL_REGISTER&=0x1)
#define OBJ_ON_FLAG                             (LCD_CONTROL_REGISTER&=0x2)
#define OBJ_BLOCK_COMPOSITION_SELECTION_FLAG    (LCD_CONTROL_REGISTER&=0x4)
#define BG_CODE_AREA_SELECTION_FLAG             (LCD_CONTROL_REGISTER&=0x8)
#define BG_CHARACTER_DATA_SELECTION_FLAG        (LCD_CONTROL_REGISTER&=0x10)
#define WINDOWING_ON_FLAG                       (LCD_CONTROL_REGISTER&=0x20)
#define WINDOW_CODE_AREA_SELECTION_FLAG         (LCD_CONTROL_REGISTER&=0x40)
#define LCD_CONTROLLER_OPERATION_STOP_FLAG      (LCD_CONTROL_REGISTER&=0x80)

#define SET_LCD_MODE_FLAG(X)                    MEM[0xFF41] &= 0xFC; MEM[0xFF41] |= X

#define JOYPAD MEM[0xFF00]
#define JOYPAD_BUTTONS (JOYPAD&0x20)
#define JOYPAD_DIRECTIONS (JOYPAD&0x10)

extern uint8_t CYCLE_LENGTH[0x100];
extern uint8_t OPCODE_LENGTH[0x100];

extern void (* exec_opcode[0x100])(void);

extern void (* exec_cb[0x100])(void);

extern uint32_t colour[2][2];
extern uint32_t DEFAULT_PALETTE[4];

void print_regs(void);

void print_mem(uint16_t low, uint16_t high, char mode);

void read_header(const uint8_t* buf);

int readfff(uint8_t* buffer, const char* filename);

void create_coredump(uint32_t length, uint16_t coredumpnum);

void reset_coredump(uint32_t length, uint16_t coredumpnum);

void remove_all_coredumps(uint16_t coredumpnum);

void convert_tile(uint8_t* input_ptr, uint32_t* output_ptr);

void convert_line(const uint8_t* input_ptr, uint32_t* output_ptr);

void background_tiles();

typedef struct BANKS
{
    uint8_t** BANK_ARRAY;
    uint8_t number;         // the amount of banks the structure holds
    uint64_t start_addr;    // starting addr of the switchable section
    uint32_t length;         // length of the section
    uint8_t active;         // indicates the currently loaded bank

} BANKS;

int switch_banks(BANKS* banks, uint8_t target_bank);

// check if second or third char is "," and calculate accordingly (needed for reset_coredump)
#define CHAR_TO_INT8(R) comma=0;\
                        buf[0]=fgetc(coredump); \
                        buf[1]=fgetc(coredump); \
                        if(buf[1]==44){(R)=buf[0]-48; comma=1;} \
                        else{buf[2]=fgetc(coredump); \
                            if(buf[2]==44){(R)=buf[1]-48+(buf[0]-48)*10; comma=1;} \
                            else{(R)=buf[2]-48+(buf[1]-48)*10+(buf[0]-48)*100;}} \
                        if(!comma){fgetc(coredump);}

#define CHAR_TO_INT16(R) buf[0]=fgetc(coredump); \
                         buf[1]=fgetc(coredump); \
                         if(buf[1]==44){(R)=buf[0]-48; comma=1;} \
                         else{buf[2]=fgetc(coredump); \
                            if(buf[2]==44){(R)=buf[1]-48+(buf[0]-48)*10; comma=1;} \
                            else{buf[3]=fgetc(coredump);\
                                if(buf[3]==44){(R)=buf[2]-48+(buf[1]-48)*10+(buf[0]-48)*100; comma=1;}\
                                else{buf[4]=fgetc(coredump);\
                                    if(buf[4]==44){(R)=buf[3]-48+(buf[2]-48)*10+(buf[1]-48)*100+(buf[0]-48)*1000;comma=1;}\
                                    else{(R)=buf[4]-48+(buf[3]-48)*10+(buf[2]-48)*100+(buf[1]-48)*1000+(buf[0]-48)*10000;comma=1;}}}} \
                         if(!comma){fgetc(coredump); comma=0;}

// flags

#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

#define ERROR(expr) fprintf(stderr, "ERROR in function \"%s\" in %s:%d%s%s\nPC=%x\t\tOpcode=%x\n", __func__, __FILE__, __LINE__, (expr)?"\n> ":"", (expr)?(expr):"", PC, MEM[PC]);
