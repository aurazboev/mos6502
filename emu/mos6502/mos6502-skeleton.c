#include <rc.h>
#include <base.h>
#include <membus.h>
#include <timekeeper.h>
#include <mos6502/vmcall.h>
#include <mos6502/mos6502.h>

#include <string.h>

static const uint8_t instr_cycles[256] = {
	7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	6, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
	2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
	2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
	2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
	2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

static inline uint8_t
read8 (mos6502_t * cpu, uint16_t addr)
{
	return membus_read(cpu->bus, addr);
}

static inline void
write8 (mos6502_t * cpu, uint16_t addr, uint8_t val)
{
	membus_write(cpu->bus, addr, val);
}

static inline uint16_t
read16 (mos6502_t * cpu, uint16_t addr)
{
	uint16_t lo = (uint16_t)read8(cpu, addr);
	uint16_t hi = (uint16_t)read8(cpu, addr + 1);
	uint16_t val = lo | (uint16_t)(hi << 8);
	return val;
}

size_t
mos6502_instr_repr(mos6502_t *cpu, uint16_t addr, char *buffer, size_t buflen)
{
	uint8_t opcode = read8(cpu, addr);
	switch (opcode)
	{
	case 0xA9: // LDA Immediate
		snprintf(buffer, buflen, "LDA #$%02X", read8(cpu, addr + 1));
		break;
	case 0xA5: // LDA ZeroPage
		snprintf(buffer, buflen, "LDA $%02X", read8(cpu, addr + 1));
		break;
	case 0xB5: // LDA ZeroPage,X
		snprintf(buffer, buflen, "LDA $%02X,X", read8(cpu, addr + 1));
		break;
	case 0xAD: // LDA Absolute
		snprintf(buffer, buflen, "LDA $%04X", read16(cpu, addr + 1));
		break;
	case 0xBD: // LDA Absolute,X
		snprintf(buffer, buflen, "LDA $%04X,X", read16(cpu, addr + 1));
		break;
	case 0xB9: // LDA Absolute,Y
		snprintf(buffer, buflen, "LDA $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0xA1: // LDA (Indirect,X)
		snprintf(buffer, buflen, "LDA ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0xB1: // LDA (Indirect),Y
		snprintf(buffer, buflen, "LDA ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0x85: // STA ZeroPage
		snprintf(buffer, buflen, "STA $%02X", read8(cpu, addr + 1));
		break;
	case 0x95: // STA ZeroPage,X
		snprintf(buffer, buflen, "STA $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x8D: // STA Absolute
		snprintf(buffer, buflen, "STA $%04X", read16(cpu, addr + 1));
		break;
	case 0x9D: // STA Absolute,X
		snprintf(buffer, buflen, "STA $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x99: // STA Absolute,Y
		snprintf(buffer, buflen, "STA $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0x81: // STA (Indirect,X)
		snprintf(buffer, buflen, "STA ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0x91: // STA (Indirect),Y
		snprintf(buffer, buflen, "STA ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0x69: // ADC Immediate
		snprintf(buffer, buflen, "ADC #$%02X", read8(cpu, addr + 1));
		break;
	case 0x65: // ADC ZeroPage
		snprintf(buffer, buflen, "ADC $%02X", read8(cpu, addr + 1));
		break;
	case 0x75: // ADC ZeroPage,X
		snprintf(buffer, buflen, "ADC $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x6D: // ADC Absolute
		snprintf(buffer, buflen, "ADC $%04X", read16(cpu, addr + 1));
		break;
	case 0x7D: // ADC Absolute,X
		snprintf(buffer, buflen, "ADC $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x79: // ADC Absolute,Y
		snprintf(buffer, buflen, "ADC $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0x61: // ADC (Indirect,X)
		snprintf(buffer, buflen, "ADC ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0x71: // ADC (Indirect),Y
		snprintf(buffer, buflen, "ADC ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0xA2: // LDX Immediate
		snprintf(buffer, buflen, "LDX #$%02X", read8(cpu, addr + 1));
		break;
	case 0xA6: // LDX ZeroPage
		snprintf(buffer, buflen, "LDX $%02X", read8(cpu, addr + 1));
		break;
	case 0xB6: // LDX ZeroPage,Y
		snprintf(buffer, buflen, "LDX $%02X,Y", read8(cpu, addr + 1));
		break;
	case 0xAE: // LDX Absolute
		snprintf(buffer, buflen, "LDX $%04X", read16(cpu, addr + 1));
		break;
	case 0xBE: // LDX Absolute,Y
		snprintf(buffer, buflen, "LDX $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0xA0: // LDY Immediate
		snprintf(buffer, buflen, "LDY #$%02X", read8(cpu, addr + 1));
		break;
	case 0xA4: // LDY ZeroPage
		snprintf(buffer, buflen, "LDY $%02X", read8(cpu, addr + 1));
		break;
	case 0xB4: // LDY ZeroPage,X
		snprintf(buffer, buflen, "LDY $%02X,X", read8(cpu, addr + 1));
		break;
	case 0xAC: // LDY Absolute
		snprintf(buffer, buflen, "LDY $%04X", read16(cpu, addr + 1));
		break;
	case 0xBC: // LDY Absolute,X
		snprintf(buffer, buflen, "LDY $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x86: // STX ZeroPage
		snprintf(buffer, buflen, "STX $%02X", read8(cpu, addr + 1));
		break;
	case 0x96: // STX ZeroPage,Y
		snprintf(buffer, buflen, "STX $%02X,Y", read8(cpu, addr + 1));
		break;
	case 0x8E: // STX Absolute
		snprintf(buffer, buflen, "STX $%04X", read16(cpu, addr + 1));
		break;
	case 0x84: // STY ZeroPage
		snprintf(buffer, buflen, "STY $%02X", read8(cpu, addr + 1));
		break;
	case 0x94: // STY ZeroPage,X
		snprintf(buffer, buflen, "STY $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x8C: // STY Absolute
		snprintf(buffer, buflen, "STY $%04X", read16(cpu, addr + 1));
		break;
	case 0x29: // AND Immediate
		snprintf(buffer, buflen, "AND #$%02X", read8(cpu, addr + 1));
		break;
	case 0x25: // AND ZeroPage
		snprintf(buffer, buflen, "AND $%02X", read8(cpu, addr + 1));
		break;
	case 0x35: // AND ZeroPage,X
		snprintf(buffer, buflen, "AND $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x2D: // AND Absolute
		snprintf(buffer, buflen, "AND $%04X", read16(cpu, addr + 1));
		break;
	case 0x3D: // AND Absolute,X
		snprintf(buffer, buflen, "AND $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x39: // AND Absolute,Y
		snprintf(buffer, buflen, "AND $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0x21: // AND (Indirect,X)
		snprintf(buffer, buflen, "AND ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0x31: // AND (Indirect),Y
		snprintf(buffer, buflen, "AND ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0x0A: // ASL Accumulator
		snprintf(buffer, buflen, "ASL A");
		break;
	case 0x06: // ASL ZeroPage
		snprintf(buffer, buflen, "ASL $%02X", read8(cpu, addr + 1));
		break;
	case 0x16: // ASL ZeroPage,X
		snprintf(buffer, buflen, "ASL $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x0E: // ASL Absolute
		snprintf(buffer, buflen, "ASL $%04X", read16(cpu, addr + 1));
		break;
	case 0x1E: // ASL Absolute,X
		snprintf(buffer, buflen, "ASL $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x38: //SEC
		snprintf(buffer, buflen, "SEC");
		break;
	case 0x18: //CSC
		snprintf(buffer, buflen, "CLC");
		break;
	case 0xF8: //SED
		snprintf(buffer, buflen, "SED");
		break;
	case 0x78: //SEI
		snprintf(buffer, buflen, "SEI");
		break;
	case 0xEA: //NOP
		snprintf(buffer, buflen, "NOP");
		break;
	case 0x09: // ORA Immediate
		snprintf(buffer, buflen, "ORA #$%02X", read8(cpu, addr + 1));
		break;
	case 0x05: // ORA ZeroPage
		snprintf(buffer, buflen, "ORA $%02X", read8(cpu, addr + 1));
		break;
	case 0x15: // ORA ZeroPage,X
		snprintf(buffer, buflen, "ORA $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x0D: // ORA Absolute
		snprintf(buffer, buflen, "ORA $%04X", read16(cpu, addr + 1));
		break;
	case 0x1D: // ORA Absolute,X
		snprintf(buffer, buflen, "ORA $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x19: // ORA Absolute,Y
		snprintf(buffer, buflen, "ORA $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0x01: // ORA (Indirect,X)
		snprintf(buffer, buflen, "ORA ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0x11: // ORA (Indirect),Y
		snprintf(buffer, buflen, "ORA ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0xD8: // CLD
		snprintf(buffer, buflen, "CLD");
		break;
	case 0xE6: // INC ZeroPage
		snprintf(buffer, buflen, "INC $%02X", read8(cpu, addr + 1));
		break;
	case 0xF6: // INC ZeroPage,X
		snprintf(buffer, buflen, "INC $%02X,X", read8(cpu, addr + 1));
		break;
	case 0xEE: // INC Absolute
		snprintf(buffer, buflen, "INC $%04X", read16(cpu, addr + 1));
		break;
	case 0xFE: // INC Absolute,X
		snprintf(buffer, buflen, "INC $%04X,X", read16(cpu, addr + 1));
		break;
	case 0xC6: // DEC ZeroPage
		snprintf(buffer, buflen, "DEC $%02X", read8(cpu, addr + 1));
		break;
	case 0xD6: // DEC ZeroPage,X
		snprintf(buffer, buflen, "DEC $%02X,X", read8(cpu, addr + 1));
		break;
	case 0xCE: // DEC Absolute
		snprintf(buffer, buflen, "DEC $%04X", read16(cpu, addr + 1));
		break;
	case 0xDE: // DEC Absolute,X
		snprintf(buffer, buflen, "DEC $%04X,X", read16(cpu, addr + 1));
		break;
	case 0xE8: // INX
		snprintf(buffer, buflen, "INX");
		break;
	case 0xCA: // DEX
		snprintf(buffer, buflen, "DEX");
		break;
	case 0xC8: // INY
		snprintf(buffer, buflen, "INY");
		break;
	case 0x88: // DEY
		snprintf(buffer, buflen, "DEY");
		break;
	case 0xAA: // TAX
		snprintf(buffer, buflen, "TAX");
		break;
	case 0xA8: // TAY
		snprintf(buffer, buflen, "TAY");
		break;
	case 0x8A: // TXA
		snprintf(buffer, buflen, "TXA");
		break;
	case 0x98: // TYA
		snprintf(buffer, buflen, "TYA");
		break;
	case 0x48: // PHA
		snprintf(buffer, buflen, "PHA");
		break;
	case 0x68: // PLA
		snprintf(buffer, buflen, "PLA");
		break;
	case 0xBA: // TSX
		snprintf(buffer, buflen, "TSX");
		break;
	case 0x9A: // TXS
		snprintf(buffer, buflen, "TXS");
		break;
	case 0xE9: // SBC Immediate
		snprintf(buffer, buflen, "SBC #$%02X", read8(cpu, addr + 1));
		break;
	case 0xE5: // SBC ZeroPage
		snprintf(buffer, buflen, "SBC $%02X", read8(cpu, addr + 1));
		break;
	case 0xF5: // SBC ZeroPage,X
		snprintf(buffer, buflen, "SBC $%02X,X", read8(cpu, addr + 1));
		break;
	case 0xED: // SBC Absolute
		snprintf(buffer, buflen, "SBC $%04X", read16(cpu, addr + 1));
		break;
	case 0xFD: // SBC Absolute,X
		snprintf(buffer, buflen, "SBC $%04X,X", read16(cpu, addr + 1));
		break;
	case 0xF9: // SBC Absolute,Y
		snprintf(buffer, buflen, "SBC $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0xE1: // SBC (Indirect,X)
		snprintf(buffer, buflen, "SBC ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0xF1: // SBC (Indirect),Y
		snprintf(buffer, buflen, "SBC ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0x28: // PLP
		snprintf(buffer, buflen, "PLP");
		break;
	case 0x2A: // ROL Accumulator
		snprintf(buffer, buflen, "ROL A");
		break;
	case 0x26: // ROL ZeroPage
		snprintf(buffer, buflen, "ROL $%02X", read8(cpu, addr + 1));
		break;
	case 0x36: // ROL ZeroPage,X
		snprintf(buffer, buflen, "ROL $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x2E: // ROL Absolute
		snprintf(buffer, buflen, "ROL $%04X", read16(cpu, addr + 1));
		break;
	case 0x3E: // ROL Absolute,X
		snprintf(buffer, buflen, "ROL $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x6A: // ROR Accumulator
		snprintf(buffer, buflen, "ROR A");
		break;
	case 0x66: // ROR ZeroPage
		snprintf(buffer, buflen, "ROR $%02X", read8(cpu, addr + 1));
		break;
	case 0x76: // ROR ZeroPage,X
		snprintf(buffer, buflen, "ROR $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x6E: // ROR Absolute
		snprintf(buffer, buflen, "ROR $%04X", read16(cpu, addr + 1));
		break;
	case 0x7E: // ROR Absolute,X
		snprintf(buffer, buflen, "ROR $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x4C: // JMP Absolute
		snprintf(buffer, buflen, "JMP $%04X", read16(cpu, addr + 1));
		break;
	case 0x6C: // JMP Indirect
		snprintf(buffer, buflen, "JMP ($%04X)", read16(cpu, addr + 1));
		break;
	case 0x20: // JSR
		snprintf(buffer, buflen, "JSR $%04X", read16(cpu, addr + 1));
		break;
	case 0x60: // RTS
		snprintf(buffer, buflen, "RTS");
		break;
	case 0xC9: // CMP Immediate
		snprintf(buffer, buflen, "CMP #$%02X", read8(cpu, addr + 1));
		break;
	case 0xC5: // CMP ZeroPage
		snprintf(buffer, buflen, "CMP $%02X", read8(cpu, addr + 1));
		break;
	case 0xD5: // CMP ZeroPage,X
		snprintf(buffer, buflen, "CMP $%02X,X", read8(cpu, addr + 1));
		break;
	case 0xCD: // CMP Absolute
		snprintf(buffer, buflen, "CMP $%04X", read16(cpu, addr + 1));
		break;
	case 0xDD: // CMP Absolute,X
		snprintf(buffer, buflen, "CMP $%04X,X", read16(cpu, addr + 1));
		break;
	case 0xD9: // CMP Absolute,Y
		snprintf(buffer, buflen, "CMP $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0xC1: // CMP (Indirect,X)
		snprintf(buffer, buflen, "CMP ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0xD1: // CMP (Indirect),Y
		snprintf(buffer, buflen, "CMP ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0xE0: // CPX Immediate
		snprintf(buffer, buflen, "CPX #$%02X", read8(cpu, addr + 1));
		break;
	case 0xE4: // CPX ZeroPage
		snprintf(buffer, buflen, "CPX $%02X", read8(cpu, addr + 1));
		break;
	case 0xEC: // CPX Absolute
		snprintf(buffer, buflen, "CPX $%04X", read16(cpu, addr + 1));
		break;
	case 0xC0: // CPY Immediate
		snprintf(buffer, buflen, "CPY #$%02X", read8(cpu, addr + 1));
		break;
	case 0xC4: // CPY ZeroPage
		snprintf(buffer, buflen, "CPY $%02X", read8(cpu, addr + 1));
		break;
	case 0xCC: // CPY Absolute
		snprintf(buffer, buflen, "CPY $%04X", read16(cpu, addr + 1));
		break;
	case 0x90: //BCC
		snprintf(buffer, buflen, "BCC $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0xB0: // BCS
		snprintf(buffer, buflen, "BCS $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0xF0: // BEQ
		snprintf(buffer, buflen, "BEQ $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0xD0: // BNE
		snprintf(buffer, buflen, "BNE $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0x10: // BPL
		snprintf(buffer, buflen, "BPL $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0x30: // BMI
		snprintf(buffer, buflen, "BMI $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0x50: // BVC
		snprintf(buffer, buflen, "BVC $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0x70: // BVS
		snprintf(buffer, buflen, "BVS $%04X", cpu->pc + 2 + (int8_t)read8(cpu, addr + 1));
		break;
	case 0x24: // BIT ZeroPage
		snprintf(buffer, buflen, "BIT $%02X", read8(cpu, addr + 1));
		break;
	case 0x2C: // BIT Absolute
		snprintf(buffer, buflen, "BIT $%04X", read16(cpu, addr + 1));
		break;
	case 0x49: // EOR Immediate
		snprintf(buffer, buflen, "EOR #$%02X", read8(cpu, addr + 1));
		break;
	case 0x45: // EOR ZeroPage
		snprintf(buffer, buflen, "EOR $%02X", read8(cpu, addr + 1));
		break;
	case 0x55: // EOR ZeroPage,X
		snprintf(buffer, buflen, "EOR $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x4D: // EOR Absolute
		snprintf(buffer, buflen, "EOR $%04X", read16(cpu, addr + 1));
		break;
	case 0x5D: // EOR Absolute,X
		snprintf(buffer, buflen, "EOR $%04X,X", read16(cpu, addr + 1));
		break;
	case 0x59: // EOR Absolute,Y
		snprintf(buffer, buflen, "EOR $%04X,Y", read16(cpu, addr + 1));
		break;
	case 0x41: // EOR (Indirect,X)
		snprintf(buffer, buflen, "EOR ($%02X,X)", read8(cpu, addr + 1));
		break;
	case 0x51: // EOR (Indirect),Y
		snprintf(buffer, buflen, "EOR ($%02X),Y", read8(cpu, addr + 1));
		break;
	case 0x40: // RTI
		snprintf(buffer, buflen, "RTI");
		break;
	case 0x00: // BRK
		snprintf(buffer, buflen, "BRK");
		break;
	case 0x4A: // LSR Accumulator
		snprintf(buffer, buflen, "LSR A");
		break;
	case 0x46: // LSR ZeroPage
		snprintf(buffer, buflen, "LSR $%02X", read8(cpu, addr + 1));
		break;
	case 0x56: // LSR ZeroPage,X
		snprintf(buffer, buflen, "LSR $%02X,X", read8(cpu, addr + 1));
		break;
	case 0x4E: // LSR Absolute
		snprintf(buffer, buflen, "LSR $%04X", read16(cpu, addr + 1));
		break;
	case 0x5E: // LSR Absolute,X
		snprintf(buffer, buflen, "LSR $%04X,X", read16(cpu, addr + 1));
		break;
	}
	return strlen(buffer); // Return the buffer length
}

mos6502_step_result_t
mos6502_step(mos6502_t *cpu)
{
	uint8_t opcode = read8(cpu, cpu->pc++);
	uint16_t addr, baseAddr, effectiveAddr, temp, lowByte, highByte;
	uint8_t value, operand, oldCarry, result, zeroPageAddr;
	int8_t offset;

	switch (opcode)
	{
	case 0x80: //VMCALL
		handle_vmcall(cpu, read8(cpu, cpu->pc++));
		break;
	case 0xA9: // LDA Immediate
		value = read8(cpu, cpu->pc++);
		cpu->a = value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xA5: // LDA ZeroPage
		addr = read8(cpu, cpu->pc++);
		cpu->a = read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xB5: // LDA ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		cpu->a = read8(cpu, addr & 0xFF);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xAD: // LDA Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		cpu->a = read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xBD: // LDA Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		cpu->a = read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xB9: // LDA Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		cpu->a = read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xA1: // LDA (Indirect,X)
		addr = read8(cpu, cpu->pc++) + cpu->x;
		addr = read16(cpu, addr & 0xFF);
		cpu->a = read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xB1: // LDA (Indirect),Y
		addr = read8(cpu, cpu->pc++);
		addr = read16(cpu, addr) + cpu->y;
		cpu->a = read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x85: // STA ZeroPage
		addr = read8(cpu, cpu->pc++);
		write8(cpu, addr, cpu->a);
		break;
	case 0x95: // STA ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		write8(cpu, addr & 0xFF, cpu->a);
		break;
	case 0x8D: // STA Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		write8(cpu, addr, cpu->a);
		break;
	case 0x9D: // STA Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		write8(cpu, addr, cpu->a);
		break;
	case 0x99: // STA Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		write8(cpu, addr, cpu->a);
		break;
	case 0x81: // STA (Indirect,X)
		operand = read8(cpu, cpu->pc++);
		effectiveAddr = read16(cpu, (operand + cpu->x) & 0xFF);
		write8(cpu, effectiveAddr, cpu->a);
		break;
	case 0x91: // STA (Indirect,Y)
		operand = read8(cpu, cpu->pc++);
		baseAddr = read16(cpu, operand & 0xFF);
		effectiveAddr = baseAddr + cpu->y;
		write8(cpu, effectiveAddr, cpu->a);
		break;
	case 0x69: // ADC Immediate
		operand = read8(cpu, cpu->pc++);
		temp = cpu->a + operand + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ operand) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0x65: // ADC ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		temp = cpu->a + value + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0x75: // ADC ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		value = read8(cpu, addr & 0xFF);
		temp = cpu->a + value + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0x6D: // ADC Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->a + value + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0; // Overflow check
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0x7D: // ADC Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->a + value + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0x79: // ADC Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->a + value + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0x61: // ADC (Indirect,X)
		operand = read8(cpu, cpu->pc++) + cpu->x;
		addr = read16(cpu, operand & 0xFF);
		value = read8(cpu, addr);
		temp = cpu->a + value + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0x71: // ADC (Indirect),Y
		operand = read8(cpu, cpu->pc++);
		baseAddr = read16(cpu, operand & 0xFF);
		addr = baseAddr + cpu->y;
		value = read8(cpu, addr);
		temp = cpu->a + value + cpu->p.c;
		cpu->p.c = temp > 0xFF;
		cpu->p.v = (~(cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->a = temp;
		cpu->p.z = cpu->a == 0;
		cpu->p.n = cpu->a & 0x80;
		break;
	case 0xA2: // LDX Immediate
		cpu->x = read8(cpu, cpu->pc++);
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xA6: // LDX ZeroPage
		addr = read8(cpu, cpu->pc++);
		cpu->x = read8(cpu, addr);
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xB6: // LDX ZeroPage,Y
		addr = read8(cpu, cpu->pc++) + cpu->y;
		cpu->x = read8(cpu, addr & 0xFF);
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xAE: // LDX Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		cpu->x = read8(cpu, addr);
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xBE: // LDX Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		cpu->x = read8(cpu, addr);
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xA0: // LDY Immediate
		cpu->y = read8(cpu, cpu->pc++);
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0xA4: // LDY ZeroPage
		addr = read8(cpu, cpu->pc++);
		cpu->y = read8(cpu, addr);
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0xB4: // LDY ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		cpu->y = read8(cpu, addr & 0xFF);
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0xAC: // LDY Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		cpu->y = read8(cpu, addr);
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0xBC: // LDY Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		cpu->y = read8(cpu, addr);
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0x86: // STX ZeroPage
		addr = read8(cpu, cpu->pc++);
		write8(cpu, addr, cpu->x);
		break;
	case 0x96: // STX ZeroPage,Y
		addr = read8(cpu, cpu->pc++) + cpu->y;
		write8(cpu, addr & 0xFF, cpu->x);
		break;
	case 0x8E: // STX Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2; 
		write8(cpu, addr, cpu->x);
		break;
	case 0x84: // STY ZeroPage
		addr = read8(cpu, cpu->pc++);
		write8(cpu, addr, cpu->y);
		break;
	case 0x94: // STY ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		write8(cpu, addr & 0xFF, cpu->y);
		break;
	case 0x8C: // STY Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		write8(cpu, addr, cpu->y);
		break;
	case 0x29: // AND Immediate
		value = read8(cpu, cpu->pc++);
		cpu->a &= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x25: // AND ZeroPage
		addr = read8(cpu, cpu->pc++);
		cpu->a &= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x35: // AND ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		cpu->a &= read8(cpu, addr & 0xFF);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x2D: // AND Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2; 
		cpu->a &= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x3D: // AND Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		cpu->a &= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x39: // AND Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		cpu->a &= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x21: // AND (Indirect,X)
		operand = read8(cpu, cpu->pc++) + cpu->x;
		addr = read16(cpu, operand & 0xFF);
		cpu->a &= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x31: // AND (Indirect),Y
		operand = read8(cpu, cpu->pc++);
		baseAddr = read16(cpu, operand & 0xFF);
		addr = baseAddr + cpu->y;
		cpu->a &= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x0A: // ASL Accumulator
		cpu->p.c = (cpu->a >> 7) & 1;
		cpu->a <<= 1;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x06: // ASL ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		cpu->p.c = (value >> 7) & 1;
		value <<= 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0x16: // ASL ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		value = read8(cpu, addr & 0xFF);
		cpu->p.c = (value >> 7) & 1;
		value <<= 1;
		write8(cpu, addr & 0xFF, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0x0E: // ASL Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->p.c = (value >> 7) & 1;
		value <<= 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0x1E: // ASL Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->p.c = (value >> 7) & 1;
		value <<= 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0x38: // SEC
		cpu->p.c = 1;
		break;
	case 0x18: // CLC
		cpu->p.c = 0;
		break;
	case 0xF8: // SED
		cpu->p.d = 1;
		break;
	case 0x78: // SEI
		cpu->p.i = 1;
		break;
	case 0xEA: // NOP
		break;
	case 0x09: // ORA Immediate
		operand = read8(cpu, cpu->pc++);
		cpu->a |= operand;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x05: // ORA ZeroPage
		addr = read8(cpu, cpu->pc++);
		cpu->a |= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x15: // ORA ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		cpu->a |= read8(cpu, addr & 0xFF);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x0D: // ORA Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		cpu->a |= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x1D: // ORA Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		cpu->a |= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x19: // ORA Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		cpu->a |= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x01: // ORA (Indirect,X)
		operand = read8(cpu, cpu->pc++) + cpu->x;
		addr = read16(cpu, operand & 0xFF);
		cpu->a |= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x11: // ORA (Indirect),Y
		operand = read8(cpu, cpu->pc++);
		baseAddr = read16(cpu, operand & 0xFF);
		addr = baseAddr + cpu->y;
		cpu->a |= read8(cpu, addr);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xD8: // CLD
		cpu->p.d = 0;
		break;
	case 0xE6: // INC ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr) + 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xF6: // INC ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		value = read8(cpu, addr & 0xFF) + 1;
		write8(cpu, addr & 0xFF, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xEE: // INC Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr) + 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xFE: // INC Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr) + 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xC6: // DEC ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr) - 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xD6: // DEC ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		value = read8(cpu, addr & 0xFF) - 1;
		write8(cpu, addr & 0xFF, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xCE: // DEC Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr) - 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xDE: // DEC Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr) - 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = (value & 0x80) != 0;
		break;
	case 0xE8: // INX
		cpu->x += 1;
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xCA: // DEX
		cpu->x -= 1;
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xC8: // INY
		cpu->y += 1;
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0x88: // DEY
		cpu->y -= 1;
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0xAA: // TAX
		cpu->x = cpu->a;
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0xA8: // TAY
		cpu->y = cpu->a;
		cpu->p.z = (cpu->y == 0);
		cpu->p.n = (cpu->y & 0x80) != 0;
		break;
	case 0x8A: // TXA
		cpu->a = cpu->x;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x98: // TYA
		cpu->a = cpu->y;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x48: // PHA
		write8(cpu, 0x0100 + cpu->sp, cpu->a);
		if (cpu->sp == 0x00)
			cpu->sp = 0xFF;
		else
			cpu->sp--;
		break;
	case 0x68: // PLA
		if (cpu->sp == 0xFF)
			cpu->sp = 0x00;
		else
			cpu->sp++;
		cpu->a = read8(cpu, 0x0100 + cpu->sp);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xBA: // TSX
		cpu->x = cpu->sp;
		cpu->p.z = (cpu->x == 0);
		cpu->p.n = (cpu->x & 0x80) != 0;
		break;
	case 0x9A: // TXS
		cpu->sp = cpu->x;
		break;
	case 0xE9: // SBC Immediate
		operand = read8(cpu, cpu->pc++);
		temp = (uint16_t)cpu->a - (uint16_t)operand - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ operand) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xE5: // SBC ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		temp = (uint16_t)cpu->a - (uint16_t)value - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xF5: // SBC ZeroPage,X
		addr = read8(cpu, cpu->pc++) + cpu->x;
		value = read8(cpu, addr & 0xFF);
		temp = (uint16_t)cpu->a - (uint16_t)value - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xED: // SBC Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = (uint16_t)cpu->a - (uint16_t)value - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xFD: // SBC Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = (uint16_t)cpu->a - (uint16_t)value - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xF9: // SBC Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = (uint16_t)cpu->a - (uint16_t)value - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xE1: // SBC (Indirect,X)
		operand = read8(cpu, cpu->pc++) + cpu->x;
		addr = read16(cpu, operand & 0xFF);
		value = read8(cpu, addr);
		temp = (uint16_t)cpu->a - (uint16_t)value - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0xF1: // SBC (Indirect),Y
		operand = read8(cpu, cpu->pc++);
		baseAddr = read16(cpu, operand & 0xFF);
		addr = baseAddr + cpu->y;
		value = read8(cpu, addr);
		temp = (uint16_t)cpu->a - (uint16_t)value - (uint16_t)(cpu->p.c ? 0 : 1);
		cpu->p.v = ((cpu->a ^ value) & (cpu->a ^ temp) & 0x80) != 0;
		cpu->p.c = temp <= 0xFF;
		cpu->a = temp;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x28: // PLP
		cpu->sp++;
		uint8_t statusFromStack = read8(cpu, 0x0100 + cpu->sp);
		uint8_t preservedBits = cpu->p.val & 0x30;
		cpu->p.val = (statusFromStack & ~0x30) | preservedBits;
		break;
	case 0x2A: // ROL Accumulator
		oldCarry = cpu->p.c;
		cpu->p.c = (cpu->a >> 7) & 1;
		cpu->a = (cpu->a << 1) | oldCarry;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x26: // ROL ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = (value >> 7) & 1;
		result = (value << 1) | oldCarry;
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x36: // ROL ZeroPage,X
		addr = (read8(cpu, cpu->pc++) + cpu->x) & 0xFF;
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = (value >> 7) & 1;
		result = (value << 1) | oldCarry;
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x2E: // ROL Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = (value >> 7) & 1;
		result = (value << 1) | oldCarry;
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x3E: // ROL Absolute,X
		baseAddr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		addr = baseAddr + cpu->x;
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = (value >> 7) & 1;
		result = (value << 1) | oldCarry;
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x6A: // ROR Accumulator
		oldCarry = cpu->p.c;
		cpu->p.c = cpu->a & 1;
		cpu->a = (cpu->a >> 1) | (oldCarry << 7);
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x66: // ROR ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = value & 1;
		result = (value >> 1) | (oldCarry << 7);
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x76: // ROR ZeroPage,X
		addr = (read8(cpu, cpu->pc++) + cpu->x) & 0xFF;
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = value & 1;
		result = (value >> 1) | (oldCarry << 7);
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x6E: // ROR Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = value & 1;
		result = (value >> 1) | (oldCarry << 7);
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x7E: // ROR Absolute,X
		baseAddr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		addr = baseAddr + cpu->x;
		value = read8(cpu, addr);
		oldCarry = cpu->p.c;
		cpu->p.c = value & 1;
		result = (value >> 1) | (oldCarry << 7);
		write8(cpu, addr, result);
		cpu->p.z = (result == 0);
		cpu->p.n = (result & 0x80) != 0;
		break;
	case 0x4C: // JMP Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc = addr;
		break;
	case 0x6C: // JMP Indirect
		uint16_t ptrAddr = read16(cpu, cpu->pc);
		uint16_t lo = read8(cpu, ptrAddr);
		uint16_t hi = read8(cpu, (ptrAddr & 0xFF00) | ((ptrAddr + 1) & 0x00FF));
		cpu->pc = (hi << 8) | lo;
		break;
	case 0x20: // JSR
		uint16_t targetAddress = read16(cpu, cpu->pc);
		uint16_t returnAddress = cpu->pc + 1;
		write8(cpu, 0x0100 + cpu->sp, (returnAddress >> 8) & 0xFF);
		cpu->sp--;
		write8(cpu, 0x0100 + cpu->sp, returnAddress & 0xFF);
		cpu->sp--;
		cpu->pc = targetAddress;
		break;
	case 0x60: // RTS
		cpu->sp++;
		lowByte = read8(cpu, 0x0100 + cpu->sp);
		cpu->sp++;
		highByte = read8(cpu, 0x0100 + cpu->sp);
		cpu->pc = (highByte << 8) | lowByte;
		cpu->pc += 1;
		break;
	case 0xC9: // CMP Immediate
		value = read8(cpu, cpu->pc++);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xC5: // CMP ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xD5: // CMP ZeroPage,X
		addr = (read8(cpu, cpu->pc++) + cpu->x) & 0xFF;
		value = read8(cpu, addr);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xCD: // CMP Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xDD: // CMP Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xD9: // CMP Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xC1: // CMP (Indirect,X)
		addr = read8(cpu, cpu->pc++) + cpu->x;
		addr = read16(cpu, addr & 0xFF);
		value = read8(cpu, addr);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xD1: // CMP (Indirect),Y
		addr = read8(cpu, cpu->pc++);
		addr = read16(cpu, addr) + cpu->y;
		value = read8(cpu, addr);
		temp = cpu->a - value;
		cpu->p.c = cpu->a >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xE0: // CPX Immediate
		value = read8(cpu, cpu->pc++);
		temp = cpu->x - value;
		cpu->p.c = cpu->x >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xE4: // CPX ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		temp = cpu->x - value;
		cpu->p.c = cpu->x >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xEC: // CPX Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->x - value;
		cpu->p.c = cpu->x >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xC0: // CPY Immediate
		value = read8(cpu, cpu->pc++);
		temp = cpu->y - value;
		cpu->p.c = cpu->y >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xC4: // CPY ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		temp = cpu->y - value;
		cpu->p.c = cpu->y >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0xCC: // CPY Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		temp = cpu->y - value;
		cpu->p.c = cpu->y >= value;
		cpu->p.z = (temp & 0xFF) == 0;
		cpu->p.n = temp & 0x80;
		break;
	case 0x90: // BCC
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (!cpu->p.c)
		{
			uint16_t newPc = cpu->pc + offset;
			cpu->pc = newPc;
		}
		break;
	case 0xB0: // BCS
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (cpu->p.c)
		{
			uint16_t newPc = cpu->pc + offset;
			cpu->pc = newPc;
		}
		break;
	case 0xF0: // BEQ
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (cpu->p.z)
		{
			uint16_t newPc = cpu->pc + offset;
			cpu->pc = newPc;
		}
		break;
	case 0xD0: // BNE
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (!cpu->p.z)
		{
			uint16_t newPc = cpu->pc + offset;
			cpu->pc = newPc;
		}
		break;
	case 0x10: // BPL
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (!cpu->p.n)
		{
			uint16_t newPc = cpu->pc + offset;
			cpu->pc = newPc;
		}
		break;
	case 0x30: // BMI
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (cpu->p.n)
		{
			cpu->pc += offset;
		}
		break;
	case 0x50: // BVC
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (!cpu->p.v)
		{
			cpu->pc += offset;
		}
		break;
	case 0x70: // BVS
		offset = (int8_t)read8(cpu, cpu->pc++);
		if (cpu->p.v)
		{
			cpu->pc += offset;
		}
		break;
	case 0x24: // BIT Zero Page
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		cpu->p.z = (cpu->a & value) == 0;
		cpu->p.v = (value & (1 << 6)) != 0;
		cpu->p.n = (value & (1 << 7)) != 0;
		break;
	case 0x2C: // BIT Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->p.z = (cpu->a & value) == 0;
		cpu->p.v = (value & (1 << 6)) != 0;
		cpu->p.n = (value & (1 << 7)) != 0;
		break;
	case 0x49: // EOR Immediate
		value = read8(cpu, cpu->pc++);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x45: // EOR Zero Page
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x55: // EOR Zero Page,X
		addr = (read8(cpu, cpu->pc++) + cpu->x) & 0xFF;
		value = read8(cpu, addr);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;

	case 0x4D: // EOR Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x5D: // EOR Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x59: // EOR Absolute,Y
		addr = read16(cpu, cpu->pc) + cpu->y;
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x41: // EOR (Indirect,X)
		zeroPageAddr = read8(cpu, cpu->pc++) + cpu->x;
		addr = read16(cpu, zeroPageAddr & 0xFF);
		value = read8(cpu, addr);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x51: // EOR (Indirect),Y
		zeroPageAddr = read8(cpu, cpu->pc++);
		baseAddr = read16(cpu, zeroPageAddr & 0xFF);
		addr = baseAddr + cpu->y;
		value = read8(cpu, addr);
		cpu->a ^= value;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = (cpu->a & 0x80) != 0;
		break;
	case 0x40: // RTI
		cpu->sp++;
		cpu->p.val = (read8(cpu, 0x0100 + cpu->sp) & 0xEF) | (cpu->p.val & 0x10);
		cpu->sp++;
		lowByte = read8(cpu, 0x0100 + cpu->sp);
		cpu->sp++;
		highByte = read8(cpu, 0x0100 + cpu->sp);
		cpu->pc = (highByte << 8) | lowByte;
		break;
	case 0x00: // BRK
		cpu->pc++;
		write8(cpu, 0x0100 + cpu->sp, (cpu->pc >> 8) & 0xFF);
		cpu->sp--;
		write8(cpu, 0x0100 + cpu->sp, cpu->pc & 0xFF);
		cpu->sp--;

		uint8_t status = (cpu->p.val | 0x10) | (1 << 4);
		write8(cpu, 0x0100 + cpu->sp, status);
		cpu->sp--;
		cpu->p.i = 1;
		cpu->pc = read16(cpu, 0xFFFE);
		break;
	case 0x4A: // LSR Accumulator
		cpu->p.c = cpu->a & 0x01;
		cpu->a >>= 1;
		cpu->p.z = (cpu->a == 0);
		cpu->p.n = 0;
		break;
	case 0x46: // LSR ZeroPage
		addr = read8(cpu, cpu->pc++);
		value = read8(cpu, addr);
		cpu->p.c = value & 0x01;
		value >>= 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = 0;
		break;
	case 0x56: // LSR ZeroPage,X
		addr = (read8(cpu, cpu->pc++) + cpu->x) & 0xFF;
		value = read8(cpu, addr);
		cpu->p.c = value & 0x01;
		value >>= 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = 0;
		break;
	case 0x4E: // LSR Absolute
		addr = read16(cpu, cpu->pc);
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->p.c = value & 0x01;
		value >>= 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = 0;
		break;
	case 0x5E: // LSR Absolute,X
		addr = read16(cpu, cpu->pc) + cpu->x;
		cpu->pc += 2;
		value = read8(cpu, addr);
		cpu->p.c = value & 0x01;
		value >>= 1;
		write8(cpu, addr, value);
		cpu->p.z = (value == 0);
		cpu->p.n = 0;
		break;
	default:
		break;
	}

	mos6502_advance_clk(cpu, instr_cycles[opcode]);
	return MOS6502_STEP_RESULT_SUCCESS;
}
