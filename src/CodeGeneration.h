#pragma once

#include "IR.h"
#include <string>
#include <fstream>

class CodeGeneration
{
public:

	/**
	 * Konstruktor - prima sve sto je potrebno za generisanje:
	 *  - listu promenljivih (memorijske idu u .data)
	 *  - listu instrukcija (idu u .text, sa dodeljenim registrima)
	 *  - ime funkcije (.globl i labela)
	 */
	CodeGeneration(Variables& variables, Instructions& instructions, std::string functionName);

	/**
	 * Generise kompletan MIPS kod i upisuje ga u izlaznu .s datoteku.
	 */
	void generate(std::string outputFileName);

private:

	Variables& variables;
	Instructions& instructions;
	std::string functionName;

	/**
	 * Vraca string registra koji je dodeljen promenljivoj (npr. "$t0").
	 */
	std::string regToString(Variable* var);

	/**
	 * Vraca tekstualni MIPS zapis jedne instrukcije (sa dodeljenim registrima).
	 */
	std::string instructionToString(Instruction* instr);

	/**
	 * Ispisuje .data sekciju (memorijske promenljive) u izlazni tok.
	 */
	void generateDataSection(std::ofstream& out);

	/**
	 * Ispisuje .text sekciju (instrukcije) u izlazni tok.
	 */
	void generateTextSection(std::ofstream& out);
};