#include "Parser.h"

// Q: How does loadInstructions even run?

// Here shows an example on how to translate "add x10, x10, x25"
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
            strcmp(raw_instr, "sub") == 0 ||
            strcmp(raw_instr, "sll") == 0 ||
            strcmp(raw_instr, "srl") == 0 ||
            strcmp(raw_instr, "xor") == 0 ||
            strcmp(raw_instr, "or") == 0 ||
            strcmp(raw_instr, "and") == 0)
        {
            parseRType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
        }
        // Q: How many of these do we need to add?
        else if (strcmp(raw_instr, "ld") == 0) ||
                 strcmp(raw_instr, "lbu") == 0) ||
                 strcmp(raw_instr, "addi") == 0) ||
                 strcmp(raw_instr, "slli") == 0) ||
                 strcmp(raw_instr, "xori") == 0) ||
                 strcmp(raw_instr, "srli") == 0) ||
            {
                parseIType(raw_instr, &(i_mem->instructions[IMEM_index]));
                i_mem->last = &(i_mem->instructions[IMEM_index]);
            }

        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
}
//parseIType
//parseSBType
void parseRType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    // Q: Do we need to implement the others?
    if (strcmp(opr, "add") == 0)
    {
        opcode = 51;
        funct3 = 0;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg) - 1] = '\0';
    unsigned rs_2 = regIndex(reg);

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

    if (strcmp(opr, "lb") == 0)
    {
        opcode = 3;
        funct3 = 0;
        funct7 = 0;
    }
    if (strcmp(opr, "lbu") == 0)
    {
        opcode = 3;
        funct3 = 0;
        funct7 = 0;
    }
    if (strcmp(opr, "addi") == 0)
    {
        opcode = 19;
        funct3 = 0;
        funct7 = 0;
    }
    if (strcmp(opr, "slli") == 0)
    {
        opcode = 19;
        funct3 = 0;
        funct7 = 0;
    }
    if (strcmp(opr, "xori") == 0)
    {
        opcode = 19;
        funct3 = 0;
        funct7 = 0;
    }
    if (strcmp(opr, "srli") == 0)
    {
        opcode = 19;
        funct3 = 0;
        funct7 = 0;
    }
    // Q: Why do we have NULL in strtok
    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg) - 1] = '\0';
    unsigned imm = regIndex(reg);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (imm << (7 + 5 + 3 + 10));
}

void parseSBType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;

    if (strcmp(opr, "bne") == 0)
    {
        opcode = 103; //Fixed FT 1/26
        funct3 = 1;   //Fixed FT 1/26
    }
    char *reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg); //Fixed FT 1/26

    reg = strtok(NULL, ", ");
    unsigned rs_2 = regIndex(reg); //Fixed FT 1/26

    reg = strtok(NULL, ", ");
    reg[strlen(reg) - 1] = '\0';
    unsigned immed = regIndex(reg); //Fixed FT 1/26

    // Contruct instruction
    //Q: In figure 2.19 for SB-type, immed[12,10:5] and immed[4:1, 11]?
    instr->instruction |= opcode;
    instr->instruction |= (immed & 31 << 7); //First five bits are maintained
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
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