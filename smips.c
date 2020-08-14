#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE_SIZE 10
#define MAX_INSTRUCTION_SIZE 1000
#define uint32 __uint32_t
#define int16_t __uint16_t



// HELPERS
int check_instruction(long long instruction);
void perform_instruction(int registers[], long long instruction);
long long htod (char hex[]);
int extractSregister(uint32 instruction);
int extractTregister(uint32 instruction); 
int extractDregister(uint32 instruction);
int extractImmediate(uint32 instruction);
void changedRegisters(int reg[]);
void print_instruction(long long instruction);



// INSTRUCTIONS
void add(int reg[], int16_t d, int16_t s, int16_t t);
void sub(int reg[], int16_t d, int16_t s, int16_t t);
void and(int reg[], int16_t d, int16_t s, int16_t t);
void or(int reg[], int16_t d, int16_t s, int16_t t);
void slt(int reg[], int16_t d, int16_t s, int16_t t);
void mul(int reg[], int16_t d, int16_t s, int16_t t);
void beq(int reg[], int16_t s, int16_t t, int I);
void bne(int reg[], int16_t s, int16_t t, int I);
void addi(int reg[], int16_t t, int16_t s, int I);
void slti(int reg[], int16_t t, int16_t s, int I);
void andi(int reg[], int16_t t, int16_t s, int I);
void ori(int reg[], int t, int s, uint32 I);
void lui(int reg[], int16_t t, int I);
void syscall(int reg[]);

int main(int argc, char* argv[]){
    // create an array for integers
    // add if statements for if first register is $zero then it will do nothing
    
    
    char line[MAX_LINE_SIZE];
    int reg[33] = {0};
    char commands[MAX_INSTRUCTION_SIZE][MAX_LINE_SIZE];

    int i = 0;
    FILE* fp = fopen(argv[1], "r");
    while (fgets(line, sizeof(line), fp)){      // load the hex into line[]
            int len = strlen(line);
            if (line[len-1]=='\n'){
                   line[len-1]='\0';
            }

            strcpy(commands[i], line);
            if (check_instruction(htod(commands[i])) == -1){        // error check for invalid instructions
                printf("%s:%d: invalid instruction code: %08X\n", argv[1], i, (int) htod(commands[i]));
                exit(1);
            }
            i++;
    }

    // i is now the number of commands that was passed


    printf("Program\n");
    for (int a = 0; a < i; a++){
        if (a < 10){
            printf("  %d: ", a);
        }else if (a >= 10){
            printf(" %d: ", a);
        }
        print_instruction(htod(commands[a]));
    }

    printf("Output\n");
    while (reg[32] < i){
        perform_instruction(reg, htod(commands[reg[32]]));
    }

    changedRegisters(reg);

    return 0;
}

long long htod (char hex[]){
    if (strcmp(hex,"c") == 0){          // checks for syscall command
        return -1337;
    }

    int len = strlen(hex) - 1;
    int value = 0;
    long long decimal = 0;
    for(int i=0; hex[i]!='\0'; i++)
    {
 
        /* Find the decimal representation of hex[i] */
        if(hex[i]>='0' && hex[i]<='9')
        {
            value = hex[i] - 48;
        }
        else if(hex[i]>='a' && hex[i]<='f')
        {
            value = hex[i] - 97 + 10;
        }
        else if(hex[i]>='A' && hex[i]<='F')
        {
            value = hex[i] - 65 + 10;
        }

        decimal += value * pow(16, len);
        len--;
    }

    return decimal;
}

void perform_instruction(int registers[], long long instruction){
    if (instruction == -1337){
        syscall(registers);
        registers[32]++;
        return;
    }

    uint32 lead_bits = instruction >> 26;             // check first 5 bits of bitpattern

    uint32 trail_bits = (instruction) & 2047;            // check last 11 bits of bitpattern

    switch (lead_bits) {
        case 0:                                  // leading 5 bits are 00000, sort lagging 11 bits
            switch (trail_bits) {
                case 32:
                    add(registers, extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 34:
                    sub(registers, extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 36:
                    and(registers, extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 37:
                    or(registers, extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 42:
                    slt(registers, extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                default:
                    return;
            }
            break;
        case 28:                                 // leading 5 bits are 011100: mul $d, $s, $t
            if (trail_bits == 2){
                mul(registers, extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
            }
            break;
        case 4:                                  // leading 5 bits are 000100: beq $s, $t, I
            beq(registers, extractSregister(instruction), extractTregister(instruction), extractImmediate(instruction));
            break;
        case 5:                                  // leading 5 bits are 000101: bne $s, $t, I
            bne(registers, extractSregister(instruction), extractTregister(instruction), extractImmediate(instruction));
            break;
        case 8:                                  // leading 5 bits are 001000: addi $t, $s, I
            addi(registers, extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 10:                                 // leading 5 bits are 001010: slti $t, $s, I
            slti(registers, extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 12:                                 // leading 5 bits are 001100: andi $t, $s, I
            andi(registers, extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 13:                                 // leading 5 bits are 001101: ori $t, $s, I
            ori(registers, extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 15:                                 // leading 5 bits are 001111: lui $t, I
            lui(registers, extractTregister(instruction), extractImmediate(instruction));
            break;
        default:                                 // error with hex
            return;
    }
    
}

void add(int reg[], int16_t d, int16_t s, int16_t t){
    if (d == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[d] = reg[s] + reg[t];
    }
    reg[32]++;
}
void sub(int reg[], int16_t d, int16_t s, int16_t t){
    if (d == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[d] = reg[s] - reg[t];
    }
    reg[32]++;
}
void and(int reg[], int16_t d, int16_t s, int16_t t){
    if (d == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[d] = reg[s] & reg[t];
    }
    reg[32]++;
}
void or(int reg[], int16_t d, int16_t s, int16_t t){
    if (d == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[d] = reg[s] | reg[t];
    }
    reg[32]++;
}
void slt(int reg[], int16_t d, int16_t s, int16_t t){
    if (d == 0)
    {
        reg[32]++;
        return;
    }
    else {
        if (reg[s] < reg[t]){
            reg[d] = 1;
        } else {
            reg[d] = 0;
        }
    }
    reg[32]++;
}

void mul(int reg[], int16_t d, int16_t s, int16_t t){
    if (d == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[d] = reg[s] * reg[t];
    }
    reg[32]++;
}

void beq(int reg[], int16_t s, int16_t t, int I){
    if (reg[s] == reg[t]){
        reg[32] += I;
    } else {
        reg[32]++;
    }
    
}

void bne(int reg[], int16_t s, int16_t t, int I){
    if (reg[s] != reg[t]){
        reg[32] += I;
    } else {
        reg[32]++;
    }
}

void addi(int reg[], int16_t t, int16_t s, int I){
    if (t == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[t] = reg[s] + I;
    }
    reg[32]++;
}

void slti(int reg[], int16_t t, int16_t s, int I){
    if (t == 0)
    {
        reg[32]++;
        return;
    }
    else {
        if (reg[s] < I){
            reg[t] = 1;
        } else {
            reg[t] = 0;
        }
    }
    reg[32]++;
}

void andi(int reg[], int16_t t, int16_t s, int I){
    if (t == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[t] = reg[s] & I;
    }
    reg[32]++;
}

void ori(int reg[], int t, int s, uint32 I){


    if (t == 0) 
    {
        reg[32]++;
        return;
    }
    else {
        reg[t] = reg[s] | I;

    }
    reg[32]++;   
}

void lui(int reg[], int16_t t, int I){
    if (t == 0)
    {
        reg[32]++;
        return;
    }
    else {
        reg[t] = I << 16;
    }
    reg[32]++;
}

void syscall(int reg[]){
    switch(reg[2]){
        case 1:
            printf("%d", reg[4]);
            break;
        case 10:
            changedRegisters(reg);
            exit(0);
            break;
        case 11:
            printf("%c", reg[4]);
            break;
        default:
            printf("Unknown system call: %d\n", reg[2]);
            changedRegisters(reg);
            exit(1);
            break;
    }
}

int extractSregister(uint32 instruction){
    int exp = 0;
    int b = instruction >> 21;
    int binaryCounter = 1;
    for (int i = 0; i < 5; i++){
        if (b & 1){
            exp += binaryCounter;
        }
        binaryCounter *= 2;
        b = b >> 1;
    }

    return exp;
}

int extractTregister(uint32 instruction){
    int exp = 0;
    int b = instruction >> 16;
    int binaryCounter = 1;
    for (int i = 0; i < 5; i++){
        if (b & 1){
            exp += binaryCounter;
        }
        binaryCounter *= 2;
        b = b >> 1;
    }

    return exp;
}

int extractDregister(uint32 instruction){
    int16_t exp = 0;
    int b = instruction >> 11;
    int binaryCounter = 1;
    for (int i = 0; i < 5; i++){
        if (b & 1){
            exp += binaryCounter;
        }
        binaryCounter *= 2;
        b = b >> 1;
    }

    return exp;
}

int extractImmediate(uint32 instruction){
    int exp = 0;
    int binaryCounter = 1;
    for (int i = 0; i < 16; i++){
        if (instruction & 1){
            exp += binaryCounter;
        }
        binaryCounter *= 2;
        instruction = instruction >> 1;
    }

    if (exp > 65000){
        return exp - 65536;
    }


    return exp;
}

void changedRegisters(int reg[]){
    printf("Registers After Execution\n");
    for (int i = 0; i < 32; i++){
        if (reg[i] != 0 && i < 10){
            printf("$%d  = %d\n", i, reg[i]);
        } else if (reg[i] != 0 && i >= 10){
            printf("$%d = %d\n", i, reg[i]);
        }
    }
}

void print_instruction(long long instruction){
    if (instruction == -1337){
        printf("syscall\n");
        return;
    }

    uint32 lead_bits = instruction >> 26;             // check first 5 bits of bitpattern

    uint32 trail_bits = (instruction) & 2047;            // check last 11 bits of bitpattern

    switch (lead_bits) {
        case 0:                                  // leading 5 bits are 00000, need to sort lagging 11 bits
            switch (trail_bits) {
                case 32:
                    printf("add  $%d, $%d, $%d\n", extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 34:
                    printf("sub  $%d, $%d, $%d\n", extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 36:
                    printf("and  $%d, $%d, $%d\n", extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 37:
                    printf("or   $%d, $%d, $%d\n", extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                case 42:
                    printf("slt  $%d, $%d, $%d\n", extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
                    break;
                default:
                    break;
            }
            break;
        case 28:                                 // leading 5 bits are 011100: mul $d, $s, $t
            if (trail_bits == 2){
                printf("mul  $%d, $%d, $%d\n", extractDregister(instruction), extractSregister(instruction), extractTregister(instruction));
            }
            break;
        case 4:                                  // leading 5 bits are 000100: beq $s, $t, I
            // TODO: Set command index
            printf("beq  $%d, $%d, %d\n", extractSregister(instruction), extractTregister(instruction), extractImmediate(instruction));
            break;
        case 5:                                  // leading 5 bits are 000101: bne $s, $t, I
            // TODO: Set command index
            printf("bne  $%d, $%d, %d\n", extractSregister(instruction), extractTregister(instruction), extractImmediate(instruction));
            break;
        case 8:                                  // leading 5 bits are 001000: addi $t, $s, I
            printf("addi $%d, $%d, %d\n", extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 10:                                 // leading 5 bits are 001010: slti $t, $s, I
            printf("slti $%d, $%d, %d\n", extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 12:                                 // leading 5 bits are 001100: andi $t, $s, I
            printf("andi $%d, $%d, %d\n", extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 13:                                 // leading 5 bits are 001101: ori $t, $s, I
            printf("ori  $%d, $%d, %d\n", extractTregister(instruction), extractSregister(instruction), extractImmediate(instruction));
            break;
        case 15:                                 // leading 5 bits are 001111: lui $t, I
            printf("lui  $%d, %d\n", extractTregister(instruction), extractImmediate(instruction));
            break;
        default:                                 // error with hex
            break;
    }
}

int check_instruction(long long instruction){
    if (instruction == -1337){
        return 0;
    }

    uint32 lead_bits = instruction >> 26;             // check first 5 bits of bitpattern

    uint32 trail_bits = (instruction) & 2047;            // check last 11 bits of bitpattern

    switch (lead_bits) {
        case 0:                                  // leading 5 bits are 00000, need to sort lagging 11 bits
            switch (trail_bits) {
                case 32:
                    return 0;
                case 34:
                    return 0;
                case 36:
                    return 0;
                case 37:
                    return 0;
                case 42:
                    return 0;
                default:
                    return -1;
            }
            break;
        case 28:                                 // leading 5 bits are 011100: mul $d, $s, $t
            if (trail_bits == 2){
            }
            return 0;
        case 4:                                  // leading 5 bits are 000100: beq $s, $t, I
            return 0;
        case 5:                                  // leading 5 bits are 000101: bne $s, $t, I
            return 0;
        case 8:                                  // leading 5 bits are 001000: addi $t, $s, I
            return 0;
        case 10:                                 // leading 5 bits are 001010: slti $t, $s, I
            return 0;
        case 12:                                 // leading 5 bits are 001100: andi $t, $s, I
            return 0;
        case 13:                                 // leading 5 bits are 001101: ori $t, $s, I
            return 0;
        case 15:                                 // leading 5 bits are 001111: lui $t, I
            return 0;
        default:                                 // error with hex
            return -1;
    }
    
}