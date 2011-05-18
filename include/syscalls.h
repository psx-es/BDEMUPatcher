#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

#include <ppu-lv2.h>
#include <hvcall.h>

#include "mm.h"

void lv2buzzer(u64 loudness, u8 quantity, u32 type);

void lv2launch(u64 addr);
int lv2sc35(char* src, char* des);
int lv2sc36(char* path);

#define SC35_PAYLOAD 2
#define SC36_PAYLOAD 1
#define ZERO_PAYLOAD 0

#define FW_341_ADDR  0x80000000002d7580
#define FW_341_VALUE 0x0000000000008534
#define FW_355_ADDR  0x80000000003329b8
#define FW_355_VALUE 0x0000000000008aac
#define FW_UNK_VALUE 0

#define BDEMU1_SIZE 0x1E8
#define BDEMU2_SIZE 0x1380

#define BDEMU_Dean36_OFFSET 0x0
#define BDEMU_Dean36_SIZE 0x1E8
#define BDEMU_Skywalk_OFFSET 0x1E8
#define BDEMU_Skywalk_SIZE 0x400
#define BDEMU_Hermes_OFFSET 0x5E8
#define BDEMU_Hermes_SIZE 0xD98

#define LV2_Dean36_ADDR 0x80000000002be4a0ULL
#define LV2_Dean36_PAD_OFFSET 0xd8
#define LV2_Dean36_PAD_SIZE 0x19eb8
#define LV2_Skywalk_ADDR 0x800000000000ef48ULL
#define LV2_Skywalk_PAD_OFFSET 0x200
#define LV2_Skywalk_PAD_SIZE 0xa0
#define LV2_Hermes_ADDR 0x80000000007e0000ULL

#define BDEMU_DECRYPT 0xFFFFFFFFFFFFFFFFULL

u64 mmap_lpar_addr;

u64 lv2peek(u64 addr);
void lv2poke(u64 addr, u64 value);
void lv2poke32(u64 addr, u32 value);

void lv1poke(u64 addr, u64 value);

int map_lv1();
void unmap_lv1(void);

void patch_lv2_protection();

void install_new_poke();
void remove_new_poke();

int remove_protection();

int payload_status();

int get_version();

#endif
