#include "CodeGeneration.h"

#include <iostream>
#include <sstream>

using namespace std;


CodeGeneration::CodeGeneration(Variables& vars, Instructions& instr, std::string funcName)
	: variables(vars), instructions(instr), functionName(funcName)
{
}


std::string CodeGeneration::regToString(Variable* var)
{
	// Mapira dodeljeni registar (Regs enum) na MIPS string.
	switch (var->getAssignment())
	{
	case t0: return "$t0";
	case t1: return "$t1";
	case t2: return "$t2";
	case t3: return "$t3";
	default: return "$??";	// nedodeljen - ne bi smelo da se desi posle alokacije
	}
}


std::string CodeGeneration::instructionToString(Instruction* instr)
{
	std::stringstream ss;

	// Pomocni pokazivaci na operande (ako postoje).
	Variable* dst = instr->getDst().empty() ? nullptr : instr->getDst().front();

	// src moze imati 1 ili 2 operanda.
	Variable* src1 = nullptr;
	Variable* src2 = nullptr;
	if (instr->getSrc().size() >= 1) src1 = instr->getSrc().front();
	if (instr->getSrc().size() >= 2) src2 = instr->getSrc().back();

	switch (instr->getType())
	{
	case I_ADD:
		ss << "add\t" << regToString(dst) << ", " << regToString(src1) << ", " << regToString(src2);
		break;
	case I_SUB:
		ss << "sub\t" << regToString(dst) << ", " << regToString(src1) << ", " << regToString(src2);
		break;
	case I_AND:
		ss << "and\t" << regToString(dst) << ", " << regToString(src1) << ", " << regToString(src2);
		break;
	case I_OR:
		ss << "or\t" << regToString(dst) << ", " << regToString(src1) << ", " << regToString(src2);
		break;
	case I_ADDI:
		ss << "addi\t" << regToString(dst) << ", " << regToString(src1) << ", " << instr->getConstant();
		break;
	case I_LA:
		// la rdest, mem_ime  (ime memorije je sacuvano u jumpLabel polju)
		ss << "la\t" << regToString(dst) << ", " << instr->getJumpLabel();
		break;
	case I_LW:
		ss << "lw\t" << regToString(dst) << ", " << instr->getConstant() << "(" << regToString(src1) << ")";
		break;
	case I_LI:
		ss << "li\t" << regToString(dst) << ", " << instr->getConstant();
		break;
	case I_SW:
		ss << "sw\t" << regToString(src1) << ", " << instr->getConstant() << "(" << regToString(src2) << ")";
		break;
	case I_B:
		ss << "b\t" << instr->getJumpLabel();
		break;
	case I_BLTZ:
		ss << "bltz\t" << regToString(src1) << ", " << instr->getJumpLabel();
		break;
	case I_BEQ:
		ss << "beq\t" << regToString(src1) << ", " << regToString(src2) << ", " << instr->getJumpLabel();
		break;
	case I_NOP:
		ss << "nop";
		break;
	default:
		ss << "# nepoznata instrukcija";
		break;
	}

	return ss.str();
}


void CodeGeneration::generateDataSection(std::ofstream& out)
{
	out << ".data" << endl;

	// Sve memorijske promenljive: ime: .word vrednost
	for (Variables::iterator it = variables.begin(); it != variables.end(); ++it)
	{
		Variable* var = *it;
		if (var->getType() == Variable::MEM_VAR)
			out << var->getName() << ":\t.word " << var->getValue() << endl;
	}

	out << endl;
}


void CodeGeneration::generateTextSection(std::ofstream& out)
{
	out << ".text" << endl;

	// Labela funkcije (prva instrukcija).
	if (!functionName.empty())
		out << functionName << ":" << endl;

	// Sve instrukcije.
	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		Instruction* instr = *it;

		// Ako instrukcija nosi labelu (npr. lab:), ispisi je pre instrukcije.
		if (!instr->getLabel().empty())
			out << instr->getLabel() << ":" << endl;

		out << "\t" << instructionToString(instr) << endl;
	}
}


void CodeGeneration::generate(std::string outputFileName)
{
	std::ofstream out(outputFileName.c_str());
	if (!out.is_open())
	{
		cout << "Greska: ne mogu da otvorim izlaznu datoteku " << outputFileName << endl;
		return;
	}

	// .globl <ime funkcije>
	if (!functionName.empty())
		out << ".globl " << functionName << endl << endl;

	generateDataSection(out);
	generateTextSection(out);

	out.close();
	cout << "Generisan MIPS kod u: " << outputFileName << endl;
}