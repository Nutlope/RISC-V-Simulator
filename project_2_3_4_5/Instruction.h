#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include <stdint.h>

typedef uint64_t Addr;
typedef uint64_t Tick;

typedef struct
{
    // Byte-addressable address
    Addr addr;

    // This is the translated binary format of assembly input
    unsigned int instruction;

}Instruction;

#endif
