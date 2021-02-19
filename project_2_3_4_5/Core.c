#include "Core.h"

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    // Initialize register file here.
    core->reg_file[25] = 4;
    core->reg_file[10] = 4;
    core->reg_file[22] = 1;

    // Initialize data memory here.
    core->data_mem[0] = 16;
    core->data_mem[8] = 128;
    core->data_mem[16] = 8;
    core->data_mem[24] = 4;

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

    printf("=======DEBUG=======\n");
    printf("op: %d\n", opcode);
    printf("rs1: %d\n", rs1);
    printf("rs2: %d\n", rs2);
    printf("rd: %d\n", rd);
    printf("func3: %d\n", func3);
    printf("func7: %d\n", func7);
    printf("ALUOp: %ld\n", signals->ALUOp);
    printf("Instr: %d\n", instruction);
    
    // (Step 3.2) Feed it into ALU which feeds into rd
    int rs1_ALU_input = core->reg_file[rs1];
    int rs2_ALU_input = MUX(signals->ALUSrc, core->reg_file[rs2], ImmeGen(instruction));
    ALU(rs1_ALU_input, rs2_ALU_input, ALUControlUnit(signals->ALUOp, func7, func3), ALU_result, zero);
    printf("*zero: %ln\n", zero);

    // TODO: FIX the Zero not responding
    // if ((*zero == 1) && (signals->Branch == 1)) {
    if (signals->Branch == 1) {
      // Target address = PC + Immed * 2
      printf("SBType Stuff is happening\n");
      printf("ImmeGen: %ld\n", ImmeGen(instruction));
      // TODO: Uncomment this line out and get rid of line above
      // core->PC += ShiftLeft1(ImmeGen(instruction));
      core->PC += 2 * ImmeGen(instruction);
    }
    else {
      modified_ALU_result = (int) *ALU_result;
      Signal memory_data = core->data_mem[modified_ALU_result];
      core->reg_file[rd] = MUX(signals->MemtoReg, *ALU_result, memory_data);
      core->PC += 4;
    }
    printf("Reg_file[%d]: %ld\n", rd, core->reg_file[rd]);
    printf("core->PC: %ld\n", core->PC);

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
    // For R-type
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
    // For Load (ld)
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
    // for SB-type: bne
    else if (input == 103) {
        signals->ALUSrc = 0; 
        signals->MemtoReg = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 1;
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

    // For ssli; returns 5
    else if (ALUOp == 0 && Funct3 == 1 & Funct7 == 0) {
      return 5;
    }

    // For bne; returns 6
    else if (ALUOp == 1 && Funct3 == 1) {
      return 6;
    }

}

Signal ImmeGen(Signal input)
{
  Signal immediate;
  int opcode = (input & 127);

  if (opcode == 51) {
    // do R-type stuff here
    immediate = 0;
  } else if (opcode == 103) {
    // Do SB-type stuff here; 13 bits total
    immediate = (input & 3840) >> 7;                // immd[4:0] 5 bits long
    immediate |= (input & 2113929216) >> 20;        // immd[10:5] 6 bits long
    immediate |= (input & 128) << 4;                // immd[11] 1 bit
    immediate |= (input & 2147483648) >> 19;        // immd[12] 1 bit

  } else {
    // do I-type stuff here; 12 bits total
    immediate = (input & 4293918720) >> 20;          // immd[11:0]
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
      *ALU_result = 0;
      *zero = 1;
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