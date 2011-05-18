#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <unistd.h>

#include <ppu-lv2.h>
#include <sys/process.h>
#include <sys/file.h>

#define APP_LAUNCH "/dev_hdd0/game/BLES80608/USRDIR/RELOAD.SELF"
#define BDEMU_MM "/dev_hdd0/game/BLES80608/USRDIR/BDEMU.BIN"
#define BDEMU "/dev_hdd0/game/BDEMU0000/USRDIR/BDEMU.BIN"

#define PAYLOAD_MOD sizeof(u64)

int bdemu_version(s32* bdemu_fd);
u8* bdemu_payload(s32 fd, u64 offset, u64 size);

int payload_hermes(s32 fd);
int payload_sky(s32 fd);
int payload_dean(s32 fd);

void showDialog(char* message);

#include "syscalls.h"

#endif
