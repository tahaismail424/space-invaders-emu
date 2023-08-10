#include <cstdint>
#include <cstdio>
#include <stdlib.h>
#include "functions.h"

struct ConditionCodes {
    uint8_t z:1;
    uint8_t s:1;
    uint8_t p:1;
    uint8_t cy:1;
    uint8_t ac:1;
    uint8_t pad:3;
};

struct State8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t sp;
    uint16_t pc;
    uint8_t* memory;
    ConditionCodes cc;
    uint8_t int_enable;
};

void UnimplementedInstruction(State8080* state)
{
    // pc will have advanced one, so undo that
    state->pc -= 1;
    printf("Error: Unimplemented instruction\n");
    Disassemble8080Op(state->memory, state->pc);
    printf("\n");
    exit(1);
}

int parity(int x, int size)
{
    int i;
    int p = 0;
    x = (x & ((1<<size) - 1));
    for (i = 0; i < size; i++)
    {
        if (x & 0x1) p++;
        x = x >> 1;
    }
    return (0 == (p & 0x1));
}

void LogicFlagsA(State8080* state)
{
    state->cc.cy = state->cc.ac = 0;
    state->cc.z = (state->a == 0);
    state->cc.s = (state->a & 0x80) == 0x80;
    state->cc.p = parity(state->a, 8);
}

void ArithFlagsA(State8080* state, uint16_t answer16) {
    state->cc.cy = answer16 > 0xff;
    state->cc.z = (answer16 & 0xff) == 0;
    state->cc.s = (answer16 & 0x80) == 0x80;
    state->cc.p = parity(answer16 & 0xff, 8);
}

int Emulate8080p(State8080* state)
{
    int cycles = 4;
    unsigned char *opcode = &state->memory[state->pc];

    Disassemble8080Op(state->memory, state->pc);
    state->pc += 1;

    uint8_t answer8;
    uint16_t answer16;
    uint32_t answer32;
    uint16_t offset;
    uint32_t hl;
    uint32_t bc;
    uint32_t de;

    switch(*opcode)
    {
        case 0x00: break;                       // NOP 
        case 0x01:                              // LXI B, word
            state->c = opcode[1]; 
            state->b = opcode[2];
            state->pc += 2;
            break;
        case 0x02: UnimplementedInstruction(state); break;
        case 0x03:                              // INX B
            UnimplementedInstruction(state);
            // uint16_t offset = (state->b<<8) | (state->c);
            // state->memory[offset] = (state->memory[offset] + 1) & 0xff;
            break;
        case 0x04:                              // INR B
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->b + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->b = answer & 0xff;
            break;
        case 0x05:                              // DCR B
            answer8 = state->b - 1; 
            state->cc.z =  (answer8 == 0);    
            state->cc.s = ((answer8 & 0x80) == 0x80);    
            state->cc.p = parity(answer8, 8);
            state->b = answer8;
            break;
        case 0x06:                              // MVI B
            state->b = opcode[1];
            state->pc += 2;
            break;
        case 0x07: UnimplementedInstruction(state); break;
        case 0x08: UnimplementedInstruction(state); break;
        case 0x09:                              // DAD B
            hl = (state->h<<8) | (state->l);
            bc = (state->b<<8) | (state->c);
            answer32 = hl + bc;
            state->h = (answer32 & 0xff00) >> 8;
            state->l = answer32 & 0xff;
            state->cc.cy = (answer32 & 0xffff0000) > 0; 
            break;
        case 0x0a:                              // LDAX B
            UnimplementedInstruction(state);
            // uint16_t offset = (state->b<<8) | (state->c);
            // state->a = state->memory[offset];
            break;
        case 0x0b:                              // DCX B
            UnimplementedInstruction(state);
            // uint16_t offset = (state->b<<8) | (state->c);
            // state->memory[offset] = (state->memory[offset] - 1) & 0xff;
            break;
        case 0x0c:                              // INR C
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->c + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->c = answer & 0xff;
            break;
        case 0x0d:                              // DCR C
            answer8 = state->c - 1; 
            state->cc.z =  (answer8 == 0);    
            state->cc.s = (answer8 & 0x80) == 0x80;    
            state->cc.p = parity(answer8, 8);
            state->c = answer8;
            break;
        case 0x0e:                              // MVI C
            state->c = opcode[1];
            state->pc++;
            break;
        case 0x0f:                              // RRC
            uint8_t x = state->a;
            state->a = ((x & 1) << 7) | (x >> 1);
            state->cc.cy = (1 == (x&1));
            break;
        case 0x11:                              // LXI D
            state->d = opcode[2];
            state->e = opcode[1];
            state->pc += 2;
            break;   
        case 0x12: UnimplementedInstruction(state); break;               
        case 0x13:                              // INX D
            state->e++;
            if (state->e == 0) state->d++;
            break;
        case 0x14:                              // INR D
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->d + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->d = answer & 0xff;
            break;
        case 0x15:                              // DCR D
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->d - 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->d = answer & 0xff;
            break;
        case 0x16: UnimplementedInstruction(state); break;
        case 0x17: UnimplementedInstruction(state); break;
        case 0x18: UnimplementedInstruction(state); break;
        case 0x19:                              // DAD D
            hl = (state->h<<8) | (state->l);
            de = (state->d<<8) | (state->e);
            answer32 = hl + de;
            state->h = (answer32 & 0xff00) >> 8;
            state->l = answer32 & 0xff;
            state->cc.cy = ((answer32 & 0xffff0000) != 0); 
            break;
        case 0x1a:                              // LDAX D
            offset = (state->d<<8) | (state->e);
            state->a = state->memory[offset];
            break;
        case 0x1b:                              // DCX D
            UnimplementedInstruction(state);
            // uint16_t offset = (state->d<<8) | (state->e);
            // state->memory[offset] = (state->memory[offset] - 1) & 0xff;
            break;
        case 0x1c:                              // INR E
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->e + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->e = answer & 0xff;
            break;
        case 0x1d:                              // DCR E
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->e - 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->e = answer & 0xff;
            break;
        case 0x1e: UnimplementedInstruction(state); break;
        case 0x1f:                              // RAR
            UnimplementedInstruction(state);
            // uint8_t x = state->a;
            // state->a = (state->cc.cy << 7) | (x >> 1);
            // state->cc.cy = (1 == (x & 1));
            break;
        case 0x21:                              // LXI H
            state->h = opcode[2];
            state->l = opcode[1];
            state->pc += 2;
            break;
        case 0x22: UnimplementedInstruction(state); break;
        case 0x23:                              // INX H
            state->l++;
            if (state->l == 0) state->h++;
            break;
        case 0x24:                              // INR H
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->h + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->h = answer & 0xff;
            break;
        case 0x25:                              // DCR H
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->h - 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->h = answer & 0xff;
            break;
        case 0x26:                              // MVI H
            state->h = opcode[1];
            state->pc++;
            break;
        case 0x27: UnimplementedInstruction(state); break;
        case 0x28: UnimplementedInstruction(state); break;
        case 0x29:                              // DAD H
            hl = (state->h<<8) | (state->l);
            answer32 = hl + hl;
            state->h = (answer32 & 0xff00) >> 8;
            state->l = answer32 & 0xff;
            state->cc.cy = ((answer32 & 0xffff0000) != 0); 
            break;
        case 0x2a: UnimplementedInstruction(state); break;
        case 0x2b:                              // DCX H
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // state->memory[offset] = (state->memory[offset] - 1) & 0xff;
            break;
        case 0x2c:                              // INR L
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->l + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->l = answer & 0xff;
            break;
        case 0x2d:                              // DCR L
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->l - 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->l = answer & 0xff;
            break;
        case 0x2e: UnimplementedInstruction(state); break;
        case 0x2f:                              // CMA
            UnimplementedInstruction(state);
            // state->a = ~state->a;
            break;
        case 0x30: UnimplementedInstruction(state); break;
        case 0x31:                              // LXI SP
            state->sp = (opcode[2] << 8) | opcode[1];
            state->pc += 2;
            break;
        case 0x32:                              // STA address
            offset = (opcode[2] << 8) | opcode[1];
            state->memory[offset] = state->a;
            state->pc += 2;
            break;
        case 0x33:
            UnimplementedInstruction(state);
            // state->sp = (state->sp + 1) & 0xff;
            break;
        case 0x34:                              // INR M
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint16_t answer = (uint16_t) state->memory[offset] + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->memory[offset] = answer & 0xff;
            break;
        case 0x35:                              // DCR M
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint16_t answer = (uint16_t) state->memory[offset] - 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->memory[offset] = answer & 0xff;
            break;  
        case 0x36:                              // MVI M
            offset = (state->h<<8) | (state->l);
            state->memory[offset] = opcode[1];
            state->pc++;
            break;
        case 0x37:                              // STC
            UnimplementedInstruction(state);
            // state->cc.cy = 1;
            break;
        case 0x38: UnimplementedInstruction(state); break;
        case 0x39:                              // DAD SP
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint16_t answer = state->memory[offset] + state->sp;
            // state->cc.cy = (answer > 0xff); 
            // state->memory[offset] = answer & 0xff;
            break;
        case 0x3a:                              // LDA address
            offset = (opcode[2] << 8) | opcode[1];
            state->a = state->memory[offset];
            state->pc+=2;
            break;
        case 0x3b:                              // DCX SP
            UnimplementedInstruction(state);
            // state->sp = (state->sp - 1) & 0xff;
            break;
        case 0x3c:                              // INR A
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x3d:                              // DCR A
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - 1; 
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x3e:                              // MVI A
            state->a = opcode[1];
            state->pc += 2;
            break;
        case 0x3f:                              // CMC
            UnimplementedInstruction(state);
            // state->cc.cy = (state->cc.cy == 1);
            break;
        case 0x41:                              // MOV B,C
            UnimplementedInstruction(state);
            // state->b = state->c; 
            break;  
        case 0x42:                              // MOV B,D
            UnimplementedInstruction(state);
            // state->b = state->d;
            break;  
        case 0x43:                              // MOV B,E
            UnimplementedInstruction(state);
            // state->b = state->e; 
            break;  
        case 0x44: UnimplementedInstruction(state); break;
        case 0x45: UnimplementedInstruction(state); break;
        case 0x46: UnimplementedInstruction(state); break;
        case 0x47: UnimplementedInstruction(state); break;
        case 0x48: UnimplementedInstruction(state); break;
        case 0x49: UnimplementedInstruction(state); break;
        case 0x4a: UnimplementedInstruction(state); break;
        case 0x4b: UnimplementedInstruction(state); break;
        case 0x4c: UnimplementedInstruction(state); break;
        case 0x4d: UnimplementedInstruction(state); break;
        case 0x4e: UnimplementedInstruction(state); break;
        case 0x4f: UnimplementedInstruction(state); break;
        case 0x50: UnimplementedInstruction(state); break;
        case 0x51: UnimplementedInstruction(state); break;
        case 0x52: UnimplementedInstruction(state); break;
        case 0x53: UnimplementedInstruction(state); break;
        case 0x54: UnimplementedInstruction(state); break;
        case 0x55: UnimplementedInstruction(state); break;
        case 0x56:                              // MOV D, M
            offset = (state->h<<8) | (state->l);
            state->d = state->memory[offset];
            break;
        case 0x57: UnimplementedInstruction(state); break;
        case 0x58: UnimplementedInstruction(state); break;
        case 0x59: UnimplementedInstruction(state); break;
        case 0x5a: UnimplementedInstruction(state); break;
        case 0x5b: UnimplementedInstruction(state); break;
        case 0x5c: UnimplementedInstruction(state); break;
        case 0x5d: UnimplementedInstruction(state); break;
        case 0x5e:                              // MOV E, M
            offset = (state->h<<8) | (state->l);
            state->e = state->memory[offset];
            break;
        case 0x5f: UnimplementedInstruction(state); break;
        case 0x60: UnimplementedInstruction(state); break;
        case 0x61: UnimplementedInstruction(state); break;
        case 0x62: UnimplementedInstruction(state); break;
        case 0x63: UnimplementedInstruction(state); break;
        case 0x64: UnimplementedInstruction(state); break;
        case 0x65: UnimplementedInstruction(state); break;
        case 0x66:                              // MOV H, M
            offset = (state->h<<8) | (state->l);
            state->h = state->memory[offset];
            break;
        case 0x67: UnimplementedInstruction(state); break;
        case 0x68: UnimplementedInstruction(state); break;
        case 0x69: UnimplementedInstruction(state); break;
        case 0x6a: UnimplementedInstruction(state); break;
        case 0x6b: UnimplementedInstruction(state); break;
        case 0x6c: UnimplementedInstruction(state); break;
        case 0x6d: UnimplementedInstruction(state); break;
        case 0x6e: UnimplementedInstruction(state); break;
        case 0x6f:                              // MOV L, A   
            state->l = state->a;
            break;
        case 0x70: UnimplementedInstruction(state); break;
        case 0x71: UnimplementedInstruction(state); break;
        case 0x72: UnimplementedInstruction(state); break;
        case 0x73: UnimplementedInstruction(state); break;
        case 0x74: UnimplementedInstruction(state); break;
        case 0x75: UnimplementedInstruction(state); break;
        case 0x76: UnimplementedInstruction(state); break;
        case 0x77:                              // MOV M, A
            offset = (state->h<<8) | (state->l);
            state->memory[offset] = state->a;
            break;
        case 0x78: UnimplementedInstruction(state); break;
        case 0x79: UnimplementedInstruction(state); break;
        case 0x7a:                              // MOV A, D
            state->a = state->d;
            break;
        case 0x7b:                              // MOV A, E
            state->a = state->e;
            break;
        case 0x7c:                              // MOV A, H
            state->a = state->h;
            break;
        case 0x7d: UnimplementedInstruction(state); break;
        case 0x7e:                              // MOV A, M
            offset = (state->h<<8) | (state->l);
            state->a = state->memory[offset];
            break;
        case 0x7f: UnimplementedInstruction(state); break;
        case 0x80:                              // ADD B
            UnimplementedInstruction(state);
            // do the math with higher precision so we can capture the
            // carry out
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->b;

            // Zero flag: if the result is zero,
            // set the flag to zero
            // else clear the flag
            // if ((answer & 0xff) == 0)
            //     state->cc.z = 1;
            // else
            //     state->cc.z = 0;
            
            // Sign flag: if bit 7 is set,
            // set the sign flag
            // else clear the sign flag
            // if (answer & 0x80)
            //     state->cc.s = 1;
            // else
            //     state->cc.s = 0;
            // Carry flag
            // if (answer > 0xff)
            //     state->cc.cy = 1;
            // else
            //     state->cc.cy = 0;

            // Parity flag: if parity is even,
            // set the parity flag
            // else clear the parity flag
            // if (answer & 0x01 == 0)
            //     state->cc.p = 1;
            // else
            //     state->cc.p = 0;
            
            // state->a = answer & 0xff;
            break;
        case 0x81:                              // ADD C
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->c;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x82:                              // ADD D
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->d;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x83:                              // ADD E
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->e;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x84:                              // ADD H
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->h;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x85:                              // ADD L
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->h;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x86:                              // ADD M
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint16_t answer = (uint16_t) state->a + state->memory[offset];
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x87:                              // ADD A
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->a;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x88:                              // ADC B
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->b + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x89:                              // ADC C
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->c + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x8a:                              // ADC D
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->d + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x8b:                              // ADC E
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->e + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x8c:                              // ADC H
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->h + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x8d:                              // ADC L
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->l + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x8e:                              // ADC M
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->memory[offset] + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x8f:                              // ADC A
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) state->a + (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x90:                              // SUB B
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->b;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x91:                              // SUB C
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->c;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x92:                              // SUB D
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->d;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x93:                              // SUB E
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->e;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x94:                              // SUB H
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->h;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break; 
        case 0x95:                              // SUB L
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->l;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x96:                              // SUB M
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->memory[offset];   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x97:                              // SUB A
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->a;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x98:                              // SBB B
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->b - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x99:                              // SBB C
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->c - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x9a:                              // SBB D
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->d - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x9b:                              // SBB E
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->e - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x9c:                              // SBB H
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->h - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x9d:                              // SBB L
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->l - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x9e:                              // SBB M
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->memory[offset] - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0x9f:                              // SBB A
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) state->a - (uint16_t) state->cc.cy;   
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0xa0: UnimplementedInstruction(state); break;
        case 0xa1: UnimplementedInstruction(state); break;
        case 0xa2: UnimplementedInstruction(state); break;
        case 0xa3: UnimplementedInstruction(state); break;
        case 0xa4: UnimplementedInstruction(state); break;
        case 0xa5: UnimplementedInstruction(state); break;
        case 0xa6: UnimplementedInstruction(state); break;
        case 0xa7:                              // ANA A
            answer8 = state->a & state->a;
            LogicFlagsA(state);
            break;
        case 0xa8: UnimplementedInstruction(state); break;
        case 0xa9: UnimplementedInstruction(state); break;
        case 0xaa: UnimplementedInstruction(state); break;
        case 0xab: UnimplementedInstruction(state); break;
        case 0xac: UnimplementedInstruction(state); break;
        case 0xad: UnimplementedInstruction(state); break;
        case 0xae: UnimplementedInstruction(state); break;
        case 0xaf:                              // XRA A
            answer8 = state->a ^ state->a;
            LogicFlagsA(state);
            break;
        case 0xb0: UnimplementedInstruction(state); break;
        case 0xb1: UnimplementedInstruction(state); break;
        case 0xb2: UnimplementedInstruction(state); break;
        case 0xb3: UnimplementedInstruction(state); break;
        case 0xb4: UnimplementedInstruction(state); break;
        case 0xb5: UnimplementedInstruction(state); break;
        case 0xb6: UnimplementedInstruction(state); break;
        case 0xb7: UnimplementedInstruction(state); break;
        case 0xb8:                              // CMP B
            UnimplementedInstruction(state);
            // uint8_t x = state->a - state->b;
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xb9:                              // CMP C
            UnimplementedInstruction(state);
            // uint8_t x = state->a - state->c;
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xba:                              // CMP D
            UnimplementedInstruction(state);
            // uint8_t x = state->a - state->d;
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xbb:                              // CMP E
            UnimplementedInstruction(state);
            // uint8_t x = state->a - state->e;
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xbc:                              // CMP H
            UnimplementedInstruction(state);
            // uint8_t x = state->a - state->h;
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xbd:                              // CMP L
            UnimplementedInstruction(state);
            // uint8_t x = state->a - state->l;
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xbe:                              // CMP M
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // uint8_t x = state->a - state->memory[offset];
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xbf:                              // CMP A
            UnimplementedInstruction(state);
            // uint8_t x = state->a - state->a;
            // state->cc.z = (x == 0);
            // state->cc.s = ((x & 0x80) == 0x80);
            // state->cc.p = ((x & 0x01) == 0); 
            // state->cc.cy = (state->a < opcode[1]);
            // state->pc++;
            break;
        case 0xc0:                              // RNZ
            UnimplementedInstruction(state);
            // if (0 == state->cc.z) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xc1:                              // POP B
            state->c = state->memory[state->sp];
            state->b = state->memory[state->sp+1];
            state->sp += 2;
            break;
        case 0xc2:                              // JNZ address
            if (0 == state->cc.z)
                state->pc = (opcode[2] << 8) | opcode[1];
            else 
                // branch not taken
                state->pc += 2;
            break;
        case 0xc3:                              // JMP address
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        case 0xc4:                              // CNZ address
            UnimplementedInstruction(state);
            // if (0 == state->cc.z) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xc5:                              // PUSH B
            state->memory[state->sp - 1] = state->b;
            state->memory[state->sp - 2] = state->c;
            state->sp = state->sp - 2;
            break;
        case 0xc6:                              // ADI byte
            answer16 = (uint16_t) state->a + (uint16_t) opcode[1];
            state->cc.z =  ((answer16 & 0xff) == 0);    
            state->cc.s = ((answer16 & 0x80) == 0x80);    
            state->cc.cy = (answer16 > 0xff);    
            state->cc.p = parity((answer16 & 0xff), 8);
            state->a = (uint8_t) answer16;
            state->pc++;
            break;
        case 0xc7: UnimplementedInstruction(state); break;
        case 0xc8:                              // RZ address
            UnimplementedInstruction(state);
            // if (1 == state->cc.z) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xc9:                              // RET
            state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            state->sp += 2;
            break;
        case 0xca:                              // JZ address
            UnimplementedInstruction(state);
            // if (1 == state->cc.z)
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // else 
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xcb: UnimplementedInstruction(state); break;
        case 0xcc:                              // CZ address
            UnimplementedInstruction(state);
            // if (1 == state->cc.z) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xcd:                              // CALL address
            uint16_t ret = state->pc+2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        case 0xce:                              // ACI byte
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a + (uint16_t) opcode[1] + state->cc.cy;
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0xcf: UnimplementedInstruction(state); break;
        case 0xd0:                              // RNC
            UnimplementedInstruction(state);
            // if (0 == state->cc.cy) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xd1:                              // POP D
            state->e = state->memory[state->sp];
            state->d = state->memory[state->sp+1];
            state->sp += 2;
            break;
        case 0xd2:                              // JNC address
            UnimplementedInstruction(state);
            // if (0 == state->cc.cy)
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // else 
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xd3:                              // OUT
            state->pc++;
            break;
        case 0xd4:                              // CNC address
            UnimplementedInstruction(state);
            // if (0 == state->cc.cy) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xd5:                              // PUSH D
            state->memory[state->sp - 1] = state->d;
            state->memory[state->sp - 2] = state->e;
            state->sp = state->sp - 2;
            break;
        case 0xd6:                              // SUI D8
            UnimplementedInstruction(state);
            // uint16_t answer = (uint16_t) state->a - (uint16_t) opcode[1];
            // state->cc.z =  ((answer & 0xff) == 0);    
            // state->cc.s = ((answer & 0x80) != 0);    
            // state->cc.cy = (answer > 0xff);    
            // state->cc.p = ((answer & 0x01) == 0);
            // state->a = answer & 0xff;
            break;
        case 0xd7: UnimplementedInstruction(state); break;
        case 0xd8:                              // RC
            UnimplementedInstruction(state);
            // if (1 == state->cc.cy) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xd9: UnimplementedInstruction(state); break;
        case 0xda:                              // JC address
            UnimplementedInstruction(state);
            // if (1 == state->cc.cy)
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // else 
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xdb: UnimplementedInstruction(state); break;
        case 0xdc:                              // CC address
            UnimplementedInstruction(state);
            // if (1 == state->cc.cy) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xdd: UnimplementedInstruction(state); break;
        case 0xde: UnimplementedInstruction(state); break;
        case 0xdf: UnimplementedInstruction(state); break;
        case 0xe0:                              // RPO
            UnimplementedInstruction(state);
            // if (0 == state->cc.p) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xe1:                              // POP H
            state->l = state->memory[state->sp];
            state->h = state->memory[state->sp+1];
            state->sp += 2;
            break;
        case 0xe2:                              // JPO address
            UnimplementedInstruction(state);
            // if (0 == state->cc.p)
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // else 
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xe3:                              // XTHL
            UnimplementedInstruction(state);
            // uint8_t temp = state->memory[state->sp];
            // state->memory[state->sp] = state->l;
            // state->l = temp;
            // temp = state->memory[state->sp + 1];
            // state->memory[state->sp + 1] = state->h;
            // state->h = temp;
            break;
        case 0xe4:                              // CPO address
            UnimplementedInstruction(state);
            // if (0 == state->cc.p) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xe5:                              // PUSH H
            state->memory[state->sp - 1] = state->h;
            state->memory[state->sp - 2] = state->l;
            state->sp = state->sp - 2;
            break;
        case 0xe6:                              // ANI
            state->a = state->a & opcode[1];
            LogicFlagsA(state);
            state->pc++;
            break;
        case 0xe7: UnimplementedInstruction(state); break;
        case 0xe8:                              // RPE
            UnimplementedInstruction(state);
            // if (0 == state->cc.cy) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xe9: UnimplementedInstruction(state); break;
        case 0xea:                              // JPE address
            UnimplementedInstruction(state);
            // if (1 == state->cc.p)
            //         state->pc = (opcode[2] << 8) | opcode[1];
            // else 
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xeb:                              // XCHG
            uint8_t temp = state->d;
            state->d = state->h;
            state->h = temp;
            temp = state->e;
            state->e = state->l;
            state->l = temp;
            break;
        case 0xec:                              // CPE address
            UnimplementedInstruction(state);
            // if (1 == state->cc.p) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xed: UnimplementedInstruction(state); break;
        case 0xef: UnimplementedInstruction(state); break;
        case 0xf0:                              // RP
            UnimplementedInstruction(state);
            // if (0 == state->cc.s) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xf1:                              // POP PSW
            state->a = state->memory[state->sp + 1];
            uint8_t psw = state->memory[state->sp];
            state->cc.z = (0x01 == (psw & 0x01));
            state->cc.s = (0x02 == (psw & 0x02));
            state->cc.p = (0x04 == (psw & 0x04));
            state->cc.cy = (0x05 == (psw & 0x08));
            state->cc.ac = (0x10 == (psw & 0x10));
            state->sp += 2;
            break;
        case 0xf2:                              // JP address
            UnimplementedInstruction(state);
            // if (0 == state->cc.s)
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // else 
            //     // branch not taken
            //     state->pc += 2;
            // break;
        case 0xf3:                              // DI
            UnimplementedInstruction(state);
            // state->int_enable = 0;
            break;
        case 0xf4:                              // CP address
            UnimplementedInstruction(state);
            // if (0 == state->cc.s) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xf5:                              // PUSH PSW
            state->memory[state->sp-1] = state->a;
            uint8_t psw = (state->cc.z |
                            state->cc.s << 1 |
                            state->cc.p << 2 |
                            state->cc.cy << 3|
                            state->cc.ac << 4 );
            state->memory[state->sp - 2] = psw;
            state->sp = state-> sp - 2;
            break;
        case 0xf6: UnimplementedInstruction(state); break;
        case 0xf7: UnimplementedInstruction(state); break;
        case 0xf8:                              // RM
            UnimplementedInstruction(state);
            // if (0 == state->cc.cy) {
            //     state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            //     state->sp += 2;
            // }
            // else
            //     state->pc += 2;
            break;
        case 0xf9:                              // SPHL
            UnimplementedInstruction(state);
            // uint16_t offset = (state->h<<8) | (state->l);
            // state->memory[state->sp] = state->memory[offset];
            break;
        case 0xfa:                              // JM address
            UnimplementedInstruction(state);
            // if (1 == state->cc.s)
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // else 
            //     // branch not taken
            //     state->pc += 2;
            break; 
        case 0xfb:                              // EI
            state->int_enable = 1;
            break;
        case 0xfc:                              // CM addresss
            UnimplementedInstruction(state);
            // if (1 == state->cc.s) {
            //     uint16_t ret = state->pc+2;
            //     state->memory[state->sp-1] = (ret >> 8) & 0xff;
            //     state->memory[state->sp-2] = (ret & 0xff);
            //     state->sp = state->sp - 2;
            //     state->pc = (opcode[2] << 8) | opcode[1];
            // }
            // else
            //     // branch not taken
            //     state->pc += 2;
            break;
        case 0xfd: UnimplementedInstruction(state); break;
        case 0xfe:                              // CPI byte
            answer8 = state->a - opcode[1];
            state->cc.z = (answer8 == 0);
            state->cc.s = ((answer8 & 0x80) == 0x80);
            state->cc.p = parity(answer8, 8); 
            state->cc.cy = (state->a < opcode[1]);
            state->pc++;
            break;
        case 0xff: UnimplementedInstruction(state); break;
    }
    printf("\t");
	printf("%c", state->cc.z ? 'z' : '.');
	printf("%c", state->cc.s ? 's' : '.');
	printf("%c", state->cc.p ? 'p' : '.');
	printf("%c", state->cc.cy ? 'c' : '.');
	printf("%c  ", state->cc.ac ? 'a' : '.');
	printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->a, state->b, state->c,
				state->d, state->e, state->h, state->l, state->sp);
	return 0;
}

void ReadFileIntoMemoryAt(State8080* state, char* filename, uint32_t offset)
{
    FILE *f= fopen(filename, "rb");
    if (f==NULL)
    {
        printf("error: Couldn't open %s\n", filename);
        exit(1);
    }
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    uint8_t *buffer = &state->memory[offset];
    fread(buffer, fsize, 1, f);
    fclose(f);
}

State8080* Init8080()
{
    State8080* state = (State8080*) calloc(1, sizeof(State8080));
    state->memory = (uint8_t*) malloc(0x10000); //16K
    return state;
}

int main (int argc, char**argv)
{
    int done = 0;
    int vblankcycles = 0;
    State8080* state = Init8080();

    ReadFileIntoMemoryAt(state, "invaders", 0);

    while (done == 0)
    {
        done = Emulate8080p(state);
    }
    return 0;
}