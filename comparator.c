#include <stdio.h>
#include <stdlib.h>

// binjgb
#include "src/emulator.h"
#include "src/common.h"

// pocketgb
#include "pocketgb/cpu.h"
#include "pocketgb/lcd.h"
#include "pocketgb/timer.h"

#define AUDIO_FREQUENCY 44100
#define AUDIO_FRAMES ((AUDIO_FREQUENCY / 10) * SOUND_OUTPUT_COUNT)
#define PRIbyte "0x%02hhX"
#define PRIshort "0x%04hX"

void print_regs(Registers* const reg) {
  printf("expected:\n");
  printf("a: " PRIbyte "\n", reg->A);
  printf("b: " PRIbyte "\n", reg->B);
  printf("c: " PRIbyte "\n", reg->C);
  printf("d: " PRIbyte "\n", reg->D);
  printf("e: " PRIbyte "\n", reg->E);
  printf("h: " PRIbyte "\n", reg->H);
  printf("l: " PRIbyte "\n", reg->L);
  printf("sp: " PRIshort "\n", reg->SP);
  printf("pc: " PRIshort "\n", reg->PC);
  printf("z: %d\n", reg->F.Z);
  printf("n: %d\n", reg->F.N);
  printf("h: %d\n", reg->F.H);
  printf("c: %d\n", reg->F.C);
}

void print_regs_test(struct registers* reg) {
  printf("actual:\n");
  printf("a: " PRIbyte "\n", reg->a);
  printf("b: " PRIbyte "\n", reg->b);
  printf("c: " PRIbyte "\n", reg->c);
  printf("d: " PRIbyte "\n", reg->d);
  printf("e: " PRIbyte "\n", reg->e);
  printf("h: " PRIbyte "\n", reg->h);
  printf("l: " PRIbyte "\n", reg->l);
  printf("sp: " PRIshort "\n", reg->sp);
  printf("pc: " PRIshort "\n", reg->pc);
  printf("z: %d\n", reg->f.z);
  printf("n: %d\n", reg->f.n);
  printf("h: %d\n", reg->f.h);
  printf("c: %d\n", reg->f.c);
}

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

static struct timer* timer;
struct cpu* setup_test (const char* const rom_fname) {
  struct cpu* cpu = calloc(1, sizeof(struct cpu));
  struct lcd* lcd = calloc(1, sizeof(struct lcd));
  timer = calloc(1, sizeof(struct timer));
  struct mmu* mmu = init_memory(NULL, rom_fname);
  init_cpu(cpu, mmu);
  init_timer(timer, mmu);
  lcd->mmu = mmu;
  lcd->mode = 2;
  // leaks lcd
  return cpu;
}

int compare (Registers* expected, struct registers* actual) {
  return (actual->a == expected->A) &&
    (actual->b == expected->B) &&
    (actual->c == expected->C) &&
    (actual->d == expected->D) &&
    (actual->e == expected->E) &&
    (actual->h == expected->H) &&
    (actual->l == expected->L) &&
    (actual->sp == expected->SP) &&
    /*(actual->pc == expected->PC);*/
    (actual->pc == expected->PC) &&
    (actual->f.z == expected->F.Z) &&
    (actual->f.n == expected->F.N) &&
    (actual->f.h == expected->F.H) &&
    (actual->f.c == expected->F.C);
}

void sync_to (struct Emulator* ref, struct cpu* test, const uint16_t sync_to) {
  while (get_regs(ref)->PC != sync_to) {
    emulator_step(ref);
  }
  printf("synced to " PRIshort "\n", sync_to);
  while (test->registers.pc != sync_to) {
    tick_once(test);
  }
  printf("synced to " PRIshort "\n", sync_to);
}

int main (int argc, char** argv) {
  struct Emulator* ref = setup_reference(argv[1]);
  struct cpu* test = setup_test(argv[1]);

  Registers* ref_reg = get_regs(ref);
  struct registers* test_reg = &test->registers;

  // sync up
  sync_to(ref, test, 0xC25F);

  uint16_t last_pc = 0x0000U;
  for (unsigned long i = 0;; ++i) {
    last_pc = test_reg->pc;
    // step forward
    emulator_step(ref);
    tick_once(test);
    timer_tick(timer, test->tick_cycles);
    // get regs
    ref_reg = get_regs(ref);
    test_reg = &test->registers;
    // compare
    int matches = compare(ref_reg, test_reg);
    if (matches) {
      printf("   match @ " PRIshort ": " PRIbyte "\n", last_pc,
          rb(test->mmu, last_pc));
    } else {
      printf("mismatch @ " PRIshort ": " PRIbyte " " PRIbyte " " PRIbyte "\n",
          last_pc,
          rb(test->mmu, last_pc),
          rb(test->mmu, last_pc + 1),
          rb(test->mmu, last_pc + 2));
      print_regs(ref_reg);
      print_regs_test(test_reg);
      /*break;*/
      getchar();
    }
  }
}
