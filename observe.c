#include <stdio.h>
#include <stdlib.h>

// binjgb
#include "src/emulator.h"
#include "src/common.h"

// pocketgb
#include "pocketgb/cpu.h"
#include "pocketgb/lcd.h"

#define AUDIO_FREQUENCY 44100
#define AUDIO_FRAMES ((AUDIO_FREQUENCY / 10) * SOUND_OUTPUT_COUNT)
#define PRIbyte "0x%02hhX"
#define PRIshort "0x%04hX"


struct Emulator* setup_reference (const char* const rom_fname) {
  FileData rom;
  file_read(rom_fname, &rom);
  struct EmulatorInit init = {
    .rom = rom,
    .audio_frequency = AUDIO_FREQUENCY,
    .audio_frames = AUDIO_FRAMES,
  };
  return emulator_new(&init);
}

struct cpu* setup_test (const char* const rom_fname) {
  struct cpu* cpu = calloc(1, sizeof(struct cpu));
  struct lcd* lcd = calloc(1, sizeof(struct lcd));
  struct mmu* mmu = init_memory(NULL, rom_fname);
  init_cpu(cpu, mmu);
  lcd->mmu = mmu;
  lcd->mode = 2;
  // leaks lcd
  return cpu;
}


void print_pc (struct Emulator* ref) {
  fprintf(stderr, PRIshort ",", get_regs(ref)->PC);
}

void print_pc2 (struct cpu* cpu) {
  fprintf(stderr, PRIshort ",", cpu->registers.pc);
}

int main (int argc, char** argv) {
  /*struct Emulator* ref = setup_reference(argv[1]);*/
  struct cpu* test = setup_test(argv[1]);

  for (unsigned long i = 0;; ++i) {
    // step forward
    /*emulator_step(ref);*/
    /*print_pc(ref);*/
    tick_once(test);
    print_pc2(test);
  }
}
