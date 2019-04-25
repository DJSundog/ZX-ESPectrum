#include "Arduino.h"
#include "FS.h"
#include "Emulator/Keyboard/PS2Kbd.h"
#include "SPIFFS.h"
#include "Emulator/z80emu/z80emu.h"
#include "msg.h"

extern byte *bank0;
extern byte borderTemp;
extern boolean cfg_slog_on;
extern void log(String);
extern Z80_STATE _zxCpu;

byte specrom[16384];

typedef int32_t dword;
typedef signed char offset;

void mount_spiffs() {
    while (!SPIFFS.begin()) {
        log(MSG_MOUNT_FAIL);
        sleep(5);
    }
}

File open_read_file(String filename) {
    File f;
    mount_spiffs();
    log(MSG_LOADING + filename);
    f = SPIFFS.open(filename, FILE_READ);
    while (!f) {
        log(MSG_READ_FILE_FAIL + filename);
        sleep(10);
        f = SPIFFS.open(filename, FILE_READ);
    }
    return f;
}

void load_ram(String sna_file) {
    File lhandle;
    uint16_t size_read;
    byte sp_h,sp_l;

    log(MSG_FREE_HEAP_BEFORE + "SNA: " + (String)system_get_free_heap_size());

    lhandle = open_read_file(sna_file);
    size_read = 0;
    // Read in the registers
    _zxCpu.i = lhandle.read();
    _zxCpu.registers.byte[Z80_H]=lhandle.read();
    _zxCpu.registers.byte[Z80_L]=lhandle.read();
    _zxCpu.registers.byte[Z80_D]=lhandle.read();
    _zxCpu.registers.byte[Z80_E]=lhandle.read();
    _zxCpu.registers.byte[Z80_B]=lhandle.read();
    _zxCpu.registers.byte[Z80_C]=lhandle.read();
    _zxCpu.registers.byte[Z80_A]=lhandle.read();
    _zxCpu.registers.byte[Z80_F]=lhandle.read();

    _zxCpu.registers.byte[Z80_H]=lhandle.read();
    _zxCpu.registers.byte[Z80_L]=lhandle.read();
    _zxCpu.registers.byte[Z80_D]=lhandle.read();
    _zxCpu.registers.byte[Z80_E]=lhandle.read();
    _zxCpu.registers.byte[Z80_B]=lhandle.read();
    _zxCpu.registers.byte[Z80_C]=lhandle.read();
    _zxCpu.registers.byte[Z80_IYH]=lhandle.read();
    _zxCpu.registers.byte[Z80_IYL]=lhandle.read();
    _zxCpu.registers.byte[Z80_IXH]=lhandle.read();
    _zxCpu.registers.byte[Z80_IXL]=lhandle.read();


    byte inter = lhandle.read();
    _zxCpu.iff2 = (inter & 0x04) ? 1 : 0;

    _zxCpu.r = lhandle.read();

    _zxCpu.registers.byte[Z80_A]=lhandle.read();
    _zxCpu.registers.byte[Z80_F]=lhandle.read();
    sp_h=lhandle.read();
    sp_l=lhandle.read();

    _zxCpu.im = lhandle.read();
    byte bordercol = lhandle.read();
    _zxCpu.registers.word[Z80_SP]=sp_l + sp_h * 0x100;

    borderTemp = bordercol;

    _zxCpu.iff1 = _zxCpu.iff2;

    uint16_t thestack = _zxCpu.registers.word[Z80_SP];
    uint16_t buf_p = 0;
    while (lhandle.available()) {
        bank0[buf_p] = lhandle.read();
        buf_p++;
    }
    lhandle.close();

    uint16_t offset = thestack - 0x4000;
    uint16_t retaddr = bank0[offset] + 0x100 * bank0[offset + 1];

    _zxCpu.registers.word[Z80_SP]++;
    _zxCpu.registers.word[Z80_SP]++;

    _zxCpu.pc = retaddr;
    
    log(MSG_FREE_HEAP_AFTER + "SNA: " + (String)system_get_free_heap_size());
}

void load_rom(String rom_file) {
    File rom_f = open_read_file(rom_file);
    for (int i = 0; i < rom_f.size(); i++) {
        specrom[i] = (byte)rom_f.read();
    }
}