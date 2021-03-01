#include "Core.h"

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    // Initialize base addresses
    core->reg_file[20] = 30; // Base address of mat[16]
    core->reg_file[5] = 158; // Base address of out[4]

    // Initialize data memory here.
    core->data_mem[0] = 16;
    core->data_mem[8] = 128;
    core->data_mem[16] = 8;
    core->data_mem[24] = 4;

    // Initialize mat[16]
    core->data_mem[30] = 0;
    core->data_mem[38] = 1;
    core->data_mem[46] = 2;
    core->data_mem[54] = 3;
    core->data_mem[62] = 4;
    core->data_mem[70] = 5;
    core->data_mem[78] = 6;
    core->data_mem[86] = 7;
    core->data_mem[94] = 8;
    core->data_mem[102] = 9;
    core->data_mem[110] = 10;
    core->data_mem[118] = 11;
    core->data_mem[126] = 12;
    core->data_mem[134] = 13;
    core->data_mem[142] = 14;
    core->data_mem[150] = 15;

    // Initialize out[4]
    core->data_mem[158] = 0;
    core->data_mem[166] = 0;
    core->data_mem[174] = 0;
    core->data_mem[182] = 0;
    
    return core;
}

bool tickFunc(Core *core)
{
    Signal *ALU_result;
    Signal *zero;
    ControlSignals *signals;
    int modified_ALU_result;
    
    // (Step 1) Reading instruction from instruction memory
    unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    
    // (Step 2) Decode first seven bits of Instruction (opcode)
    int opcode = (instruction & 127);
    ControlUnit(opcode, signals);
    
    // (Step 3) Execute
    // (Step 3.1) Extract rs1, rs2, ect from instruction
    int rs1 = ((instruction & 1015808) >> 15);
    int rs2 = ((instruction & 32505856) >> 20);
    int rd = ((instruction & 3968) >> 7);
    int func3 = ((instruction & 28672) >> 12); // ALL
    int func7 = ((instruction & 4261412864) >> 25); // Rtype & some Itype ONLY 

    printf("=======ITERATION=======\n");
    // printf("op: %d\n", opcode);
    // printf("rd: %d\n", rd);
    // printf("rs1: %d\n", rs1);
    // printf("rs2: %d\n", rs2);

    // (Step 3.2) Feed it into ALU which feeds into rd
    int rs1_ALU_input = core->reg_file[rs1];
    int rs2_ALU_input = MUX(signals->ALUSrc, core->reg_file[rs2], ImmeGen(instruction));
    ALU(rs1_ALU_input, rs2_ALU_input, ALUControlUnit(signals->ALUOp, func7, func3), ALU_result, zero);
    // printf("*zero: %ld\n", *zero);
    // printf("func3: %d\n", func3);
    // printf("func7: %d\n", func7);
    // printf("ALUOp: %ld\n", signals->ALUOp);
    // printf("signals->Branch: %ld\n", signals->Branch);

    if ((*zero == 1) && (signals->Branch == 1)) {
      core->PC += ShiftLeft1(ImmeGen(instruction));
      // printf("Branching (SB-type)\n");
    }
    else {
      modified_ALU_result = (int) *ALU_result;
      if (signals->MemWrite == 1) {
        core->data_mem[modified_ALU_result] = core->reg_file[rs2];
        // printf("ALU_result: %d\n", modified_ALU_result);
        // printf("rs2_ALU_input: %d\n", rs2_ALU_input);
        // printf("rs1_ALU_input: %d\n", rs1_ALU_input);
        // printf("Memory Writing (S-type)\n");
      } 
      else {
        // printf("R/I-type stuff\n");
        // printf("ALU_result: %d\n", modified_ALU_result);
        // printf("rs2_ALU_input: %d\n", rs2_ALU_input);
        // printf("rs1_ALU_input: %d\n", rs1_ALU_input);
        Signal memory_data = core->data_mem[modified_ALU_result];
        core->reg_file[rd] = MUX(signals->MemtoReg, *ALU_result, memory_data);
      }
      core->PC += 4;
    }
    
    printf("core->PC: %ld\n", core->PC);
    printf("out[0]: %ld\n", core->data_mem[158]);
    printf("out[1]: %ld\n", core->data_mem[166]);
    printf("out[2]: %ld\n", core->data_mem[174]);
    printf("out[3]: %ld\n", core->data_mem[182]);

    ++core->clk;
    if (core->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

void ControlUnit(Signal input,
                 ControlSignals *signals)
{
    // For R-type: add and sll
    if (input == 51)
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
    // For Load: ld
    else if (input == 3)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // for I-type: addi and slli
    else if (input == 19) {
        signals->ALUSrc = 1; 
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // for SB-type: bne and beq
    else if (input == 103) {
        signals->ALUSrc = 0; 
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 1;
    }
    // For S-Type: sd
    else if (input == 35) {
        signals->ALUSrc = 1;
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 1;
        signals->MemWrite = 1;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
}

Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3)
{
    // For add
    if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0) {
        return 2;
    }

    // For ld; returns 3
    else if (ALUOp == 0 && Funct3 == 3) {
        return 3;
    }

    // For addi; returns 4
    else if (ALUOp == 0 && Funct3 == 0) {
      return 4;
    }

    // For slli; returns 5
    else if (ALUOp == 0 && Funct3 == 1 & Funct7 == 0) {
      return 5;
    }

    // For bne; returns 6
    else if (ALUOp == 1 && Funct3 == 1) {
      return 6;
    }

    // For sll; return 7
    else if (ALUOp == 2 && Funct3 == 1 && Funct7 == 2) {
      return 7;
    }

    // For beq; return 8
    else if (ALUOp == 1 && Funct3 == 0) {
      return 8;
    }
    // For beq; return 9
    else if (ALUOp == 0 && Funct3 == 7) {
      return 9;
    }
}

Signal ImmeGen(Signal input)
{
  Signal immediate;
  Signal twelveBit_sign = 0xFFFFFFFFFFFFF000;  // 18446744073709547520
  Signal thirteenBit_sign = 0xFFFFFFFFFFFFE000; // 18446744073709543424
  int opcode = (input & 127);

  if (opcode == 51) {
    // do R-type stuff here
    immediate = 0;
  } 
  else if (opcode == 103) {
    // Do SB-type stuff here; 13 bits total
    immediate = (input & 3840) >> 7;                // immd[4:1] 5 bits long
    immediate |= (input & 2113929216) >> 20;        // immd[10:5] 6 bits long
    immediate |= (input & 128) << 4;                // immd[11] 1 bit
    immediate |= (input & 2147483648) >> 19;       // immd[12] 1 bit
    if ((immediate & 4096) == 4096)  { // if immediate value is negative 
      // printf("The immediate is NEGATIVE\n");
      immediate |= thirteenBit_sign; // signing the 64bit ImmeGen output
    }
  }
  else if (opcode == 35) {
    // do S-type stuff here; 12 bits total
    immediate = (input & 3968) >> 7; // immd[4:0] 5 bits
    immediate |= (input & 4261412864) >> 18; // immd [11:5] 7 bits
    if ((immediate & 2048) == 2048)  { // if immediate value is negative
      // printf("The immediate is NEGATIVE\n");
      immediate |= twelveBit_sign; // signing the 64bit ImmeGen output
    }
  }
  else {
    // do I-type stuff here; 12 bits total
    immediate = (input & 4293918720) >> 20;          // immd[11:0]
    if ((immediate & 2048) == 2048)  { // if immediate value is negative
      // printf("The immediate is NEGATIVE\n");
      immediate |= twelveBit_sign; // signing the 64bit ImmeGen output
    }
  }
  return immediate;
}

void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal, // ALUControlUnit
         Signal *ALU_result,
         Signal *zero)
{
    // For addition
    if (ALU_ctrl_signal == 2)
    {
        *ALU_result = input_0 + input_1;
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }

    // For load double word (ld)
    else if (ALU_ctrl_signal == 3)
    {
        *ALU_result = input_0 + input_1;
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }

    }
    // For Add immediate (addi)
    else if (ALU_ctrl_signal == 4)
    {
        *ALU_result = input_0 + input_1;
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // For shift left logical immediate (slli)
    else if (ALU_ctrl_signal == 5) 
    {
      *ALU_result = input_0 << input_1;
      if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
    // For bne
    else if (ALU_ctrl_signal == 6)
    { 
      if ((input_0 - input_1) == 0) {
        *zero = 0;
      }
      else {
        *ALU_result = 0;
        *zero = 1;
      }
    }
    // For sll
    else if (ALU_ctrl_signal == 7) 
    {
      *ALU_result = input_0 << input_1;
      *zero = 0;
    }
    // for beq
    else if (ALU_ctrl_signal == 8) 
    {
      if ((input_0 - input_1) == 0) {
        *ALU_result = 0;
        *zero = 1;
      }
      else {
        *zero = 0;
      }
    }
    // for sd
    else if (ALU_ctrl_signal == 9) 
    {
        *ALU_result = input_0 + input_1;
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
}

Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1)
{
    if (sel == 0) { return input_0; } else { return input_1; }
}

Signal Add(Signal input_0,
           Signal input_1)
{
    return (input_0 + input_1);
}

Signal ShiftLeft1(Signal input)
{
    return input << 1;
}