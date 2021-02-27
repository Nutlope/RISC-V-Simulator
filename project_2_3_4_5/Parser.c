#include "Parser.h"

void loadInstructions(Instruction_Memory *i_mem, const char *trace)
{
    printf("Loading trace file: %s\n", trace);

    FILE *fd = fopen(trace, "r");
    if (fd == NULL)
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE);
    }

    // Iterate all the assembly instructions
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    Addr PC = 0; // program counter points to the zeroth location initially.
    int IMEM_index = 0;
    while ((read = getline(&line, &len, fd)) != -1)
    {
        // Assign program counter
        i_mem->instructions[IMEM_index].addr = PC;

        // Extract operation
        char *raw_instr = strtok(line, " ");
        if (strcmp(raw_instr, "add") == 0 ||
            strcmp(raw_instr, "sll") == 0)
        {
          parseRType(raw_instr, &(i_mem->instructions[IMEM_index]));
          i_mem->last = &(i_mem->instructions[IMEM_index]);
	      }
        else if (strcmp(raw_instr, "ld") == 0 ||
                 strcmp(raw_instr, "addi") == 0 ||
                 strcmp(raw_instr, "slli") == 0)
        {
          parseIType(raw_instr, &(i_mem->instructions[IMEM_index]));
          i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
        else if (strcmp(raw_instr, "bne") == 0 || 
                 strcmp(raw_instr, "beq") == 0) 
        {
          parseSBType(raw_instr, &(i_mem->instructions[IMEM_index]));
          i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
        else if (strcmp(raw_instr, "sd") == 0)
        {
          parseSType(raw_instr, &(i_mem->instructions[IMEM_index]));
          i_mem->last = &(i_mem->instructions[IMEM_index]);
        }

        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
}
void parseRType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "add") == 0)
    {
        opcode = 51;
        funct3 = 0;
        funct7 = 0;
    }
    else if (strcmp(opr, "sll") == 0)
    {
        opcode = 51;
        funct3 = 1;
        funct7 = 2;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);
    
    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_2;

    if (reg[strlen(reg) - 1] == '\n') {
      reg[strlen(reg)-1] = '\0';
      rs_2 = regIndex(reg);
    }
    else {
      rs_2 = regIndex(reg);
    }

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
}
void parseIType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;
    if (strcmp(opr, "addi") == 0)
    {
        opcode = 19;
        funct3 = 0;
        funct7 = 0;
    }
    else if (strcmp(opr, "slli") == 0)
    {
        opcode = 19;
        funct3 = 1;
        funct7 = 0;
    }
    else if (strcmp(opr, "ld") == 0)
    {
        opcode = 3;
        funct3 = 3;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    unsigned rs_1;
    unsigned imm;

    if (strcmp(opr, "ld") == 0)
    {
        reg = strtok(NULL, "(");
        char *pEnd;
        imm = strtol(reg, &pEnd, 10);

        reg = strtok(NULL, ")");
        rs_1 = regIndex(reg);
    }
    else
    {
        reg = strtok(NULL, ", ");
        rs_1 = regIndex(reg);

        reg = strtok(NULL, "\n");
        char *pEnd;
        imm = strtol(reg, &pEnd, 10);
    }

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (imm << (7 + 5 + 3 + 5));
}
void parseSBType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;

    if (strcmp(opr, "bne") == 0)
    {
        opcode = 103; 
        funct3 = 1; 
    }
    else if (strcmp(opr, "beq") == 0)
    {
        opcode = 103; 
        funct3 = 0;
    }
    char *reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg); 

    reg = strtok(NULL, ", ");
    unsigned rs_2 = regIndex(reg); 

    reg = strtok(NULL, "\n"); // char
    char *pEnd;
    unsigned immed = strtol(reg, &pEnd, 10);
    
    instr->instruction |= opcode;
    instr->instruction |= ((immed & 2048) >> 4); //immed[11]
    instr->instruction |= ((immed & 30) << 7); //immed[4:1] 
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= ((immed & 2016) << 20); //immed[10:5]
    instr->instruction |= ((immed & 4096) << 19); //immed[12]
}
void parseSType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    if (strcmp(opr, "sd") == 0)
    {
        opcode = 35;
        funct3 = 7;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rs_2 = regIndex(reg);

    unsigned rs_1;
    unsigned imm;

    reg = strtok(NULL, "(");
    char *pEnd;
    imm = strtol(reg, &pEnd, 10);

    reg = strtok(NULL, ")");
    rs_1 = regIndex(reg);
    
    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= ((imm & 31) << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= ((imm & 4064) << (7 + 5 + 3 + 5));
}


int regIndex(char *reg)
{
    unsigned i = 0;
    for (i; i < NUM_OF_REGS; i++)
    {
        if (strcmp(REGISTER_NAME[i], reg) == 0)
        {
            break;
        }
    }

    return i;
}