#ifndef __INSTRUCTION_MEMORY_H__
#define __INSTRUCTION_MEMORY_H__

#include "Instruction.h"

#define IMEM_SIZE 256
typedef struct
{
    Instruction instructions[IMEM_SIZE];

    Instruction *last; // Points to the last instruction
}Instruction_Memory;

#endif
