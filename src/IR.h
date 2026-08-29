#pragma once

#include "Types.h"


/**
 * This class represents one variable from program code.
 */
class Variable
{
public:
	enum VariableType
	{
		MEM_VAR,
		REG_VAR,
		NO_TYPE
	};

	Variable() : m_type(NO_TYPE), m_name(""), m_position(-1), m_assignment(no_assign), m_value(0) {}
	Variable(std::string name, int pos) : m_type(NO_TYPE), m_name(name), m_position(pos), m_assignment(no_assign), m_value(0) {}

	std::string getName() { return m_name; }
	int getPos() { return m_position; }
	VariableType getType() { return m_type; }
	void setType(VariableType t) { m_type = t; }
	void setPos(int pos) { m_position = pos; }
	Regs getAssignment() { return m_assignment; }
	void setAssignment(Regs r) { m_assignment = r; }
	int getValue() { return m_value; }
	void setValue(int v) { m_value = v; }

private:
	VariableType m_type;
	std::string m_name;
	int m_position;
	Regs m_assignment;
	int m_value;
};


/**
 * This type represents list of variables from program code.
 */
typedef std::list<Variable*> Variables;


/**
 * Proverava da li promenljiva 'var' vec postoji u listi 'vars'.
 * Koristi se za uniju skupova bez duplikata.
 */
inline bool variableExists(Variable* var, Variables& vars)
{
	for (Variables::iterator it = vars.begin(); it != vars.end(); ++it)
	{
		if (*it == var)
			return true;
	}
	return false;
}


/**
 * This class represents one instruction in program code.
 */
class Instruction
{
public:
	Instruction() : m_position(0), m_type(I_NO_TYPE) {}
	Instruction(int pos, InstructionType type, Variables& dst, Variables& src) :
		m_position(pos), m_type(type), m_dst(dst), m_src(src) {
	}

	int getPos() { return m_position; }
	InstructionType getType() { return m_type; }
	Variables& getDst() { return m_dst; }
	Variables& getSrc() { return m_src; }
	Variables& getUse() { return m_use; }
	Variables& getDef() { return m_def; }
	Variables& getIn() { return m_in; }
	Variables& getOut() { return m_out; }
	std::list<Instruction*>& getSucc() { return m_succ; }
	std::list<Instruction*>& getPred() { return m_pred; }

	// Labela koju instrukcija nosi (npr. "lab" kod  lab: add ...). Prazno ako nema.
	std::string getLabel() { return m_label; }
	void setLabel(std::string l) { m_label = l; }

	// Labela na koju instrukcija skace (b lab / bltz r,lab / j lab). Prazno ako nije skok.
	std::string getJumpLabel() { return m_jumpLabel; }
	void setJumpLabel(std::string l) { m_jumpLabel = l; }

	// Brojcana vrednost: konstanta (li/addi) ili offset (lw/sw).
	int getConstant() { return m_constant; }
	void setConstant(int c) { m_constant = c; }

private:
	int m_position;
	InstructionType m_type;

	Variables m_dst;
	Variables m_src;

	Variables m_use;
	Variables m_def;
	Variables m_in;
	Variables m_out;
	std::list<Instruction*> m_succ;
	std::list<Instruction*> m_pred;

	std::string m_label;		// labela koju instrukcija nosi
	std::string m_jumpLabel;	// labela na koju instrukcija skace
	int m_constant;				// konstanta (li/addi) ili offset (lw/sw)
};


/**
 * This type represents list of instructions from program code.
 */
typedef std::list<Instruction*> Instructions;