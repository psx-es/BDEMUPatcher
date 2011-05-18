/*
 *    Copyright (C) 2011 Noltari
 *
 *    This file is part of BDEMUPatcher.
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ppu-types.h>

#include <rsx/rsx.h>

#include <sys/file.h>
#include <sys/thread.h>
#include <sys/process.h>

#include <sysutil/msg.h>
#include <sysutil/sysutil.h>
#include <sysutil/video.h>

#include <sysmodule/sysmodule.h>

#include "rsxutil.h"

#include "common.h"

int appstate = 0;

msgType mt_ok = (MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);

SYS_PROCESS_PARAM(1001, 0x100000);

static void dialog_handler(msgButton button, void *usrdata) {
	appstate = 1;
}

void sysutil_callback(u64 status, u64 param, void *usrdata) {
	if(status == SYSUTIL_EXIT_GAME) {
		appstate = 1;
	}
}

int main() {
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sysutil_callback, NULL);

	void *host_addr = memalign(1024*1024, HOST_SIZE);
	init_screen(host_addr, HOST_SIZE);

	setRenderTarget(curr_fb);

	int payload = payload_status();
	/*
	char dlgmsg[256];
	sprintf(dlgmsg, "Payload: %d", payload);
	showDialog(dlgmsg);
	*/
	if(payload == ZERO_PAYLOAD) {
		s32 fd = 0;
		int bdemu = bdemu_version(&fd);

		if(bdemu == 0) {
			//Wrong or inexistent file.
			lv2buzzer(0x1004, 0xa, 0x1b6);
			showDialog("Couldn't find correct \"BDEMU.BIN\".");
		}
		else {
			int fw_version = get_version();

			if(fw_version == FW_355_VALUE) {
				if(bdemu == 2) {
					payload_sky(fd);
				}
				else {
					payload_dean(fd);
				}
			}
			else if(fw_version == FW_341_VALUE && bdemu == 2) {
				payload_hermes(fd);
			}
			else {
				lv2buzzer(0x1004, 0xa, 0x1b6);
				showDialog("BDEMU.BIN doesn't suppoort your firmware.");
			}
		}
	}

	msgDialogAbort();

	gcmSetWaitFlip(context);
	rsxFinish(context, 1);

	//Launch Application on exit.
	sysProcessExitSpawn2(APP_LAUNCH, NULL, NULL, NULL, 0, 1001, SYS_PROCESS_SPAWN_STACK_SIZE_1M);

	return 0;
}

int bdemu_version(s32* bdemu_fd) {
	s32 fd, fd_mm;
	u64 size = 0, size_mm = 0;

	//Open possible BDEMU.BIN files.
	if(sysLv2FsOpen(BDEMU, SYS_O_RDONLY, &fd, 0660, NULL, 0) == 0) {
		sysFSStat stat;
		sysLv2FsFStat(fd, &stat);
		size = stat.st_size;
	}
	if(sysLv2FsOpen(BDEMU_MM, SYS_O_RDONLY, &fd_mm, 0660, NULL, 0) == 0) {
		sysFSStat stat;
		sysLv2FsFStat(fd_mm, &stat);
		size_mm = stat.st_size;
	}

	//Try to get newest version.
	if(size == BDEMU2_SIZE || size_mm == BDEMU2_SIZE) {
		if(size == BDEMU2_SIZE) {
			*bdemu_fd = fd;
		}
		else if(size_mm == BDEMU2_SIZE) {
			*bdemu_fd = fd_mm;
		}

		return 2;
	}
	else if(size == BDEMU1_SIZE || size_mm == BDEMU1_SIZE) {
		if(size == BDEMU1_SIZE) {
			*bdemu_fd = fd;
		}
		else if(size_mm == BDEMU1_SIZE) {
			*bdemu_fd = fd_mm;
		}

		return 1;
	}
	else {
		return 0;
	}
}
u8* bdemu_payload(s32 fd, u64 offset, u64 size) {
	//Payload memory.
	u8* payload = (u8*) malloc(size);

	//Go to payload position.
	u64 pos;
	sysLv2FsLSeek64(fd, offset, SEEK_SET, &pos);

	//Read payload.
	u64 read;
	sysLv2FsRead(fd, payload, size, &read);

	//Close descriptor.
	sysLv2FsClose(fd);

	return payload;
}

int payload_hermes(s32 fd) {
	u64 payload_size = BDEMU_Hermes_SIZE;
	u8* payload = bdemu_payload(fd, BDEMU_Hermes_OFFSET, payload_size);

	//Payload.
	int i;
	u64 addr, value;
	for(i = 0; i < payload_size; i += PAYLOAD_MOD) {
		memcpy(&value, &payload[i], PAYLOAD_MOD);
		value ^= BDEMU_DECRYPT;

		addr = LV2_Hermes_ADDR + i;

		lv2poke(addr, value);
	}

	//Patch LV2.
	lv2poke(0x8000000000017CE0ULL, 0x7C6903A64E800420); //Syscall 9

	__asm__("sync");
	sleep(1);

	//Launch payload.
	lv2launch(LV2_Hermes_ADDR);

	__asm__("sync");
	sleep(1);

	free(payload);
	return 0;
}
int payload_sky(s32 fd) {
	//Remove LV2 protection.
	if(remove_protection() == 0) {
		u64 payload_size = BDEMU_Skywalk_SIZE;
		u8* payload = bdemu_payload(fd, BDEMU_Skywalk_OFFSET, payload_size);

		//Payload.
		int i;
		u64 addr, value;
		for(i = 0; i < payload_size; i += PAYLOAD_MOD) {
			memcpy(&value, &payload[i], PAYLOAD_MOD);
			value ^= BDEMU_DECRYPT;

			addr = LV2_Skywalk_ADDR + i;
			if(i >= LV2_Skywalk_PAD_OFFSET) {
				addr += LV2_Skywalk_PAD_SIZE;
			}

			lv2poke(addr, value);
		}

		//Patch LV2.
		lv2poke32(0x8000000000055f14ULL, 0x60000000); //Syscall 36 Patches
		lv2poke32(0x8000000000055f1cULL, 0x48000098); //Syscall 36 Patches
		lv2poke32(0x800000000007af68ULL, 0x60000000); //Syscall 36 Patches
		lv2poke32(0x800000000007af7cULL, 0x60000000); //Syscall 36 Patches

		lv2poke32(0x8000000000055ea4ULL, 0x60000000); //Fix 8001003D error
		lv2poke32(0x8000000000055f68ULL, 0x3be00000); //Fix 8001003E error

		lv2poke(0x80000000002b3298ULL, 0x4bd5bda04bd9b411); //Jump Hook

		lv2poke(0x80000000003465b0ULL, 0x800000000000f2e0); //syscall_8_desc - sys8
		lv2poke(0x8000000000346690ULL, 0x800000000000f010); //syscall_map_open_desc - sys36

		free(payload);
		return 0;
	}
	else {
		return -1;
	}
}
int payload_dean(s32 fd) {
	//Remove LV2 protection.
	if(remove_protection() == 0) {
		u64 payload_size = BDEMU_Dean36_SIZE;
		u8* payload = bdemu_payload(fd, BDEMU_Dean36_OFFSET, payload_size);

		//Payload.
		int i;
		u64 addr, value;
		for(i = 0; i < payload_size; i += PAYLOAD_MOD) {
			memcpy(&value, &payload[i], PAYLOAD_MOD);
			value ^= BDEMU_DECRYPT;

			addr = LV2_Dean36_ADDR + i;
			if(i >= LV2_Dean36_PAD_OFFSET) {
				addr += LV2_Dean36_PAD_SIZE;
			}

			lv2poke(addr, value);
		}

		//Patch LV2.
		lv2poke32(0x8000000000055f14ULL, 0x60000000); //Syscall 36 Patches
		lv2poke32(0x8000000000055f1cULL, 0x48000098); //Syscall 36 Patches
		lv2poke32(0x800000000007af68ULL, 0x60000000); //Syscall 36 Patches
		lv2poke32(0x800000000007af7cULL, 0x60000000); //Syscall 36 Patches

		lv2poke32(0x8000000000055ea4ULL, 0x60000000); //Fix 8001003D error
		lv2poke32(0x8000000000055f68ULL, 0x3be00000); //Fix 8001003E error

		lv2poke(0x80000000002b3274ULL, 0x480251ec2ba30420); //Jump Hook

		lv2poke(0x8000000000346690ULL, 0x80000000002be570); //syscall_map_open_desc - sys36

		lv2sc36("/dev_bdvd");

		free(payload);
		return 0;
	}
	else {
		return -1;
	}
}

void showDialog(char* message) {
	appstate = 0;

	msgDialogOpen2(mt_ok, message, dialog_handler, NULL, NULL);

	while(appstate != 1) {
		sysUtilCheckCallback();
		flip();
	}
}
