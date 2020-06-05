#ifndef __CORE_H__
#define __CORE_H__

#include "Instruction_Memory.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define BOOL bool

struct Core;
typedef struct Core Core;
typedef struct Core
{
    Tick clk; // Keep track of core clock
    Addr PC; // Keep track of program counter

    // TODO, define your components here
    // What else you need? Data memory? Register file?
    Instruction_Memory *instr_mem;
    
    // TODO, simulation function
    bool (*tick)(Core *core);
}Core;

Core *initCore(Instruction_Memory *i_mem);
bool tickFunc(Core *core);

#endif
