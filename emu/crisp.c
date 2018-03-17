/*********************************************************************************
Copyright (c) 2018, mintsuki
All rights reserved.

Redistribution and use in source form, or binary form with accompanying source,
with or without modification, are permitted provided that the following conditions
are met:
    * The redistribution of the source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Modified versions of the source code MUST be licensed under a verbatim
      copy of the present License Agreement.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define EQU_FLAG ((uint64_t)1 << 0)

typedef struct {
    int cpu_n;      /* CPU number */
    int cpl;        /* current protection level */
    int hlt;        /* halted flag */
    int pg;         /* paging enabled */
    uint64_t sv_sp; /* kernel stack */
    uint64_t sv_pc0; /* kernel entry points */
    uint64_t sv_pc1;
    uint64_t sv_pc2;
    uint64_t sv_pc3;
    uint64_t sv_pc4;
    uint64_t sv_pc5;
    uint64_t sv_pc6;
    uint64_t sv_pc7;
    uint64_t sv_pc8;
    uint64_t sv_pc9;
    uint64_t sv_pc10;
    uint64_t sv_pc11;
    uint64_t sv_pc12;
    uint64_t sv_pc13;
    uint64_t sv_pc14;
    uint64_t sv_pc15;
    uint64_t flags; /* flags */
    uint64_t pc;    /* program counter */
    uint64_t pp;    /* pagemap pointer */
    uint64_t sp;    /* stack pointer */
    uint64_t bp;    /* base pointer */
    uint64_t dr0;   /* exception registers */
    uint64_t dr1;
    uint64_t dr2;
    uint64_t dr3;
    uint64_t r0;    /* 32 GPRs */
    uint64_t r1;
    uint64_t r2;
    uint64_t r3;
    uint64_t r4;
    uint64_t r5;
    uint64_t r6;
    uint64_t r7;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t r16;
    uint64_t r17;
    uint64_t r18;
    uint64_t r19;
    uint64_t r20;
    uint64_t r21;
    uint64_t r22;
    uint64_t r23;
    uint64_t r24;
    uint64_t r25;
    uint64_t r26;
    uint64_t r27;
    uint64_t r28;
    uint64_t r29;
    uint64_t r30;
    uint64_t r31;
} CRISP_core;

void dbg_console(CRISP_core *cpu);
int emu_cycle(CRISP_core *cpu, int dbg);

int core_count = 1;

unsigned long ram_size = 0x4000000;

char *bios_image = "bios.bin";

CRISP_core *cpu0;

uint8_t *system_ram;

int main(int argc, char *argv[]) {

    printf("Starting emulation\n");
    printf("Core count = %d\n", core_count);
    printf("RAM size = %lu\n", ram_size);
    printf("BIOS image = %s\n", bios_image);

    FILE *bios_fd = fopen(bios_image, "r");
    if (!bios_fd) {
        fprintf(stderr, "Error opening \"%s\"\n", bios_image);
        abort();
    }

    system_ram = malloc(ram_size);

    fseek(bios_fd, 0, SEEK_END);
    unsigned long size_of_bios = ftell(bios_fd);
    rewind(bios_fd);
    printf("Loading BIOS, size = %lu\n", size_of_bios);
    fread(system_ram, 1, size_of_bios, bios_fd);
    fclose(bios_fd);

    cpu0 = calloc(sizeof(CRISP_core), core_count);
    cpu0->cpu_n = 0;

    dbg_console(cpu0);

    free(cpu0);
    free(system_ram);

    puts("Goodbye!");
    return 0;

}

char *get_cpl_str(int cpl) {
    switch (cpl) {
        case 0:
            return "SV";
        case 1:
            return "US";
        default:
            return "??";
    }
}

void dbg_regdump(CRISP_core *cpu) {
    printf(
        "CPU %d state:\n"
        "r0  = 0x%016" PRIx64 "    r1  = 0x%016" PRIx64 "\n"
        "r2  = 0x%016" PRIx64 "    r3  = 0x%016" PRIx64 "\n"
        "r4  = 0x%016" PRIx64 "    r5  = 0x%016" PRIx64 "\n"
        "r6  = 0x%016" PRIx64 "    r7  = 0x%016" PRIx64 "\n"
        "r8  = 0x%016" PRIx64 "    r9  = 0x%016" PRIx64 "\n"
        "r10 = 0x%016" PRIx64 "    r11 = 0x%016" PRIx64 "\n"
        "r12 = 0x%016" PRIx64 "    r13 = 0x%016" PRIx64 "\n"
        "r14 = 0x%016" PRIx64 "    r15 = 0x%016" PRIx64 "\n"
        "r16 = 0x%016" PRIx64 "    r17 = 0x%016" PRIx64 "\n"
        "r18 = 0x%016" PRIx64 "    r19 = 0x%016" PRIx64 "\n"
        "r20 = 0x%016" PRIx64 "    r21 = 0x%016" PRIx64 "\n"
        "r22 = 0x%016" PRIx64 "    r23 = 0x%016" PRIx64 "\n"
        "r24 = 0x%016" PRIx64 "    r25 = 0x%016" PRIx64 "\n"
        "r26 = 0x%016" PRIx64 "    r27 = 0x%016" PRIx64 "\n"
        "r28 = 0x%016" PRIx64 "    r29 = 0x%016" PRIx64 "\n"
        "r30 = 0x%016" PRIx64 "    r31 = 0x%016" PRIx64 "\n"
        "sp  = 0x%016" PRIx64 "    bp  = 0x%016" PRIx64 "\n"
        "pc  = 0x%016" PRIx64 "    pp  = 0x%016" PRIx64 "\n"
        "flags = 0x%016" PRIx64 "\n"
        "CPL = %s    HLT = %d    PG = %d\n"
        "SV_SP = 0x%016" PRIx64 "\n"
        "SV_PC0  = 0x%016" PRIx64 "  SV_PC1  = 0x%016" PRIx64 "\n"
        "SV_PC2  = 0x%016" PRIx64 "  SV_PC3  = 0x%016" PRIx64 "\n"
        "SV_PC4  = 0x%016" PRIx64 "  SV_PC5  = 0x%016" PRIx64 "\n"
        "SV_PC6  = 0x%016" PRIx64 "  SV_PC7  = 0x%016" PRIx64 "\n"
        "SV_PC8  = 0x%016" PRIx64 "  SV_PC9  = 0x%016" PRIx64 "\n"
        "SV_PC10 = 0x%016" PRIx64 "  SV_PC11 = 0x%016" PRIx64 "\n"
        "SV_PC12 = 0x%016" PRIx64 "  SV_PC13 = 0x%016" PRIx64 "\n"
        "SV_PC14 = 0x%016" PRIx64 "  SV_PC15 = 0x%016" PRIx64 "\n"
        "dr0 = 0x%016" PRIx64 "    dr1 = 0x%016" PRIx64 "\n"
        "dr2 = 0x%016" PRIx64 "    dr3 = 0x%016" PRIx64 "\n",
        cpu->cpu_n,
        cpu->r0, cpu->r1, cpu->r2, cpu->r3, cpu->r4, cpu->r5, cpu->r6, cpu->r7,
        cpu->r8, cpu->r9, cpu->r10, cpu->r11, cpu->r12, cpu->r13, cpu->r14, cpu->r15,
        cpu->r16, cpu->r17, cpu->r18, cpu->r19, cpu->r20, cpu->r21, cpu->r22, cpu->r23,
        cpu->r24, cpu->r25, cpu->r26, cpu->r27, cpu->r28, cpu->r29, cpu->r30, cpu->r31,
        cpu->sp, cpu->bp, cpu->pc, cpu->pp, cpu->flags,
        get_cpl_str(cpu->cpl), cpu->hlt, cpu->pg,
        cpu->sv_sp,
        cpu->sv_pc0, cpu->sv_pc1, cpu->sv_pc2, cpu->sv_pc3, cpu->sv_pc4, cpu->sv_pc5,
        cpu->sv_pc6, cpu->sv_pc7, cpu->sv_pc8, cpu->sv_pc9, cpu->sv_pc10, cpu->sv_pc11,
        cpu->sv_pc12, cpu->sv_pc13, cpu->sv_pc14, cpu->sv_pc15,
        cpu->dr0, cpu->dr1, cpu->dr2, cpu->dr3
    );
    return;
}

void dbg_console(CRISP_core *cpu) {
    char prompt[128];

    for (;;) {
        fprintf(stdout, "CPU=%d CPL=%s pc=0x%016" PRIx64 " >>> ", cpu->cpu_n, get_cpl_str(cpu->cpl), cpu->pc);
        fgets(prompt, 127, stdin);
        if (!strcmp(prompt, "r\n"))
            dbg_regdump(cpu);
        else if (!strcmp(prompt, "q\n"))
            return;
        else if (!strcmp(prompt, "c\n"))
            while (!emu_cycle(cpu, 0));
        else if (*prompt == '\n')
            emu_cycle(cpu, 1);
    }

}

void CRISP_outb(uint64_t port, uint8_t value, int dbg) {

    if (dbg)
        printf("outputting byte 0x%02" PRIx8 " to port 0x%04" PRIx64 "\n", value, port);

    switch (port) {
        case 0x40:
            /* debug output */
            putchar(value);
            fflush(stdout);
            break;
        default:
            /* unassigned port */
            if (dbg)
                printf("write to unassigned port, ignoring\n");
            break;
    }

    return;

}

int emu_cycle(CRISP_core *cpu, int dbg) {

    uint8_t cond_prefix = system_ram[cpu->pc];
    uint8_t opcode = system_ram[cpu->pc + 1];
    uint8_t operand0 = system_ram[cpu->pc + 2];
    uint8_t operand1 = system_ram[cpu->pc + 3];

    if (dbg)
        printf("fetched opcode: 0x%02" PRIx8 " 0x%02" PRIx8
                              " 0x%02" PRIx8 " 0x%02" PRIx8 "\n",
               cond_prefix, opcode, operand0, operand1);

    /* disassemble and evaluate */

    switch (cond_prefix) {
        case 0x00:
            break;
        case 0x01:
            if (dbg)
                printf("condition:      ifeq\n");
            if (!cpu->flags & EQU_FLAG) {
                if (dbg)
                    printf("condition not fullfilled, skipping instruction\n");
                if (operand1 == 0xe0)
                    cpu->pc += 12;
                else
                    cpu->pc += 4;
                return 0;
            }
            break;
        default:
            if (dbg)
                printf("condition:      unk\n");
            return 0;
    }

    if (dbg)
        printf("disassembly:    ");

    switch (opcode) {
        case 0x88:
            /* magic breakpoint */
            printf("\nmagic breakpoint\n");
            cpu->pc += 4;
            return 1;
        case 0x10:
            /* mov */
            if (dbg)
                printf("mov r%d, ", operand0);
            if (operand1 == 0xe0) {
                uint64_t imm = *((uint64_t *)&system_ram[cpu->pc + 4]);
                if (dbg)
                    printf("0x%016" PRIx64, imm);
                ((uint64_t *)&cpu->r0)[operand0] = imm;
                cpu->pc += 12;
            } else {
                if (dbg)
                    printf("r%d", operand1);
                ((uint64_t *)&cpu->r0)[operand0] = ((uint64_t *)&cpu->r0)[operand1];
                cpu->pc += 4;
            }
            if (dbg)
                putchar('\n');
            break;
        case 0x20:
            /* loadq */
            if (dbg)
                printf("loadq r%d, r%d\n", operand0, operand1);
            ((uint64_t *)&cpu->r0)[operand0] = *((uint64_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand1]));
            cpu->pc += 4;
            break;
        case 0x21:
            /* storeq */
            if (dbg)
                printf("storeq r%d, r%d\n", operand0, operand1);
            *((uint64_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand0])) = ((uint64_t *)&cpu->r0)[operand1];
            cpu->pc += 4;
            break;
        case 0x22:
            /* loadd */
            if (dbg)
                printf("loadd r%d, r%d\n", operand0, operand1);
            ((uint64_t *)&cpu->r0)[operand0] = *((uint32_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand1]));
            cpu->pc += 4;
            break;
        case 0x23:
            /* stored */
            if (dbg)
                printf("stored r%d, r%d\n", operand0, operand1);
            *((uint32_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand0])) = (uint32_t)(((uint64_t *)&cpu->r0)[operand1] & 0xffffffff);
            cpu->pc += 4;
            break;
        case 0x24:
            /* loadw */
            if (dbg)
                printf("loadw r%d, r%d\n", operand0, operand1);
            ((uint64_t *)&cpu->r0)[operand0] = *((uint16_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand1]));
            cpu->pc += 4;
            break;
        case 0x25:
            /* storew */
            if (dbg)
                printf("storew r%d, r%d\n", operand0, operand1);
            *((uint16_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand0])) = (uint16_t)(((uint64_t *)&cpu->r0)[operand1] & 0xffff);
            cpu->pc += 4;
            break;
        case 0x26:
            /* loadb */
            if (dbg)
                printf("loadb r%d, r%d\n", operand0, operand1);
            ((uint64_t *)&cpu->r0)[operand0] = *((uint8_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand1]));
            cpu->pc += 4;
            break;
        case 0x27:
            /* storeb */
            if (dbg)
                printf("storeb r%d, r%d\n", operand0, operand1);
            *((uint8_t *)(system_ram + ((uint64_t *)&cpu->r0)[operand0])) = (uint8_t)(((uint64_t *)&cpu->r0)[operand1] & 0xff);
            cpu->pc += 4;
            break;
        case 0x46:
            /* outb */
            if (dbg)
                printf("outb r%d, r%d\n", operand0, operand1);
            CRISP_outb(((uint64_t *)&cpu->r0)[operand0], ((uint64_t *)&cpu->r0)[operand1], dbg);
            cpu->pc += 4;
            break;
        case 0x50:
            /* lsp */
            if (dbg)
                printf("lsp r%d\n", operand1);
            cpu->sp = ((uint64_t *)&cpu->r0)[operand1];
            cpu->pc += 4;
            break;
        case 0x51:
            /* ssp */
            if (dbg)
                printf("ssp r%d\n", operand1);
            ((uint64_t *)&cpu->r0)[operand1] = cpu->sp;
            cpu->pc += 4;
            break;
        case 0x80:
            /* push */
            if (operand1 == 0xe0) {
                uint64_t imm = *((uint64_t *)&system_ram[cpu->pc + 4]);
                if (dbg)
                    printf("push 0x%016" PRIx64 "\n", imm);
                cpu->sp -= 8;
                *((uint64_t *)(system_ram + cpu->sp)) = imm;
                cpu->pc += 12;
            } else {
                if (dbg)
                    printf("push r%d\n", operand1);
                cpu->sp -= 8;
                *((uint64_t *)(system_ram + cpu->sp)) = ((uint64_t *)&cpu->r0)[operand1];
                cpu->pc += 4;
            }
            break;
        case 0x81:
            /* pop */
            if (dbg)
                printf("pop r%d\n", operand1);
            ((uint64_t *)&cpu->r0)[operand1] = *((uint64_t *)(system_ram + cpu->sp));
            cpu->sp += 8;
            cpu->pc += 4;
            break;
        case 0xe0:
            /* mov sv_pc */
            if (dbg)
                printf("mov sv_pc%d, r%d\n", operand0, operand1);
            ((uint64_t *)&cpu->sv_pc0)[operand0] = ((uint64_t *)&cpu->r0)[operand1];
            cpu->pc += 4;
            break;
        case 0xe2:
            /* lsvsp */
            if (dbg)
                printf("lsvsp r%d\n", operand1);
            cpu->sv_sp = ((uint64_t *)&cpu->r0)[operand1];
            cpu->pc += 4;
            break;
        case 0xe4:
            /* sysret */
            if (dbg)
                printf("sysret\n");
            if (!cpu->pg) {
                if (dbg)
                    printf("PG bit off, sending GPF\n");
                //CRISP_except(cpu, CRISP_EXC_GPF);
            }
            cpu->cpl = 1;
            cpu->pc = *((uint64_t *)(system_ram + cpu->sp));
            cpu->flags = *((uint64_t *)(system_ram + cpu->sp + 16));
            cpu->sp = *((uint64_t *)(system_ram + cpu->sp + 8));
            break;
        case 0xe5:
            /* syscall */
            if (dbg)
                printf("syscall\n");
            if (cpu->cpl != 0) {
                cpu->cpl = 0;
                uint64_t user_pc = cpu->pc + 4;
                uint64_t user_sp = cpu->sp;
                cpu->sp = cpu->sv_sp;
                cpu->pc = cpu->sv_pc8;
                cpu->sp -= 8;
                *((uint64_t *)(system_ram + cpu->sp)) = cpu->flags;
                cpu->sp -= 8;
                *((uint64_t *)(system_ram + cpu->sp)) = user_sp;
                cpu->sp -= 8;
                *((uint64_t *)(system_ram + cpu->sp)) = user_pc;
            }
            break;
        case 0xa0:
            /* call */
            if (dbg)
                printf("call ");
            if (operand1 == 0xe0) {
                uint64_t imm = *((uint64_t *)&system_ram[cpu->pc + 4]);
                if (dbg)
                    printf("0x%016" PRIx64 "\n", imm);
                cpu->sp -= 8;
                *((uint64_t *)(system_ram + cpu->sp)) = cpu->pc + 12;
                cpu->pc = imm;
            } else {
                if (dbg)
                    printf("r%d\n", operand1);
                cpu->sp -= 8;
                *((uint64_t *)(system_ram + cpu->sp)) = cpu->pc + 4;
                cpu->pc = ((uint64_t *)&cpu->r0)[operand1];
            }
            break;
        case 0xa1:
            /* ret */
            if (dbg)
                printf("ret\n");
            cpu->pc = *((uint64_t *)(system_ram + cpu->sp));
            cpu->sp += 8;
            break;
        case 0xa8:
            /* jmp */
            if (dbg)
                printf("jmp ");
            if (operand1 == 0xe0) {
                uint64_t imm = *((uint64_t *)&system_ram[cpu->pc + 4]);
                if (dbg)
                    printf("0x%016" PRIx64 "\n", imm);
                cpu->pc = imm;
            } else {
                if (dbg)
                    printf("r%d\n", operand1);
                cpu->pc = ((uint64_t *)&cpu->r0)[operand1];
            }
            break;
        case 0xb0:
            /* cmp */
            if (dbg)
                printf("cmp r%d, ", operand0);
            {
            uint64_t val0;
            uint64_t val1;
            val0 = ((uint64_t *)&cpu->r0)[operand0];
            if (operand1 == 0xe0) {
                val1 = *((uint64_t *)&system_ram[cpu->pc + 4]);
                if (dbg)
                    printf("0x%016" PRIx64, val1);
                cpu->pc += 12;
            } else {
                if (dbg)
                    printf("r%d", operand1);
                val1 = ((uint64_t *)&cpu->r0)[operand1];
                cpu->pc += 4;
            }
            if (val0 == val1) {
                /* equality */
                cpu->flags |= EQU_FLAG;
            }
            }
            if (dbg)
                putchar('\n');
            break;
        case 0xc0:
            /* inc */
            if (dbg)
                printf("inc r%d\n", operand1);
            ((uint64_t *)&cpu->r0)[operand1]++;
            cpu->pc += 4;
            break;
        case 0xff:
            if (dbg)
                printf("hlt\n");
            if (cpu->cpl != 0) {
                if (dbg)
                    printf("instruction not allowed in user mode, sending GPF\n");
                //CRISP_except(cpu, CRISP_EXC_GPF);
            }
            cpu->hlt = 1;
            break;
        default:
            if (dbg)
                printf("undefined opcode\n");
            break;
    }

    return 0;

}
