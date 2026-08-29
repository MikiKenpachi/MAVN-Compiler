#include "SyntaxAnalysis.h"

#include <iostream>

using namespace std;


SyntaxAnalysis::SyntaxAnalysis(LexicalAnalysis& lex)
	: lexicalAnalysis(lex), errorFound(false), instructionCounter(0), functionName("")
{
	// Iterator se postavlja na pocetak liste tokena.
	tokenIterator = lexicalAnalysis.getTokenList().begin();
}


Token SyntaxAnalysis::getNextToken()
{
	// Ako smo dosli do kraja liste, to je interna greska (nema vise tokena).
	if (tokenIterator == lexicalAnalysis.getTokenList().end())
	{
		cout << "Greska! Nema vise tokena za analizu!" << endl;
		exit(1);
	}

	// Vrati trenutni token i pomeri iterator na sledeci.
	return *tokenIterator++;
}


void SyntaxAnalysis::eat(TokenType t)
{
	if (errorFound)
		return;

	// Da li je trenutni token ocekivanog tipa?
	if (currentToken.getType() == t)
	{
		// Jeste - ispisi ga.
		printTokenInfo(currentToken);

		// Dobavi sledeci token samo ako nismo na kraju fajla.
		if (t != T_END_OF_FILE)
			currentToken = getNextToken();
	}
	else
	{
		// Nije - sintaksna greska.
		printSyntaxError(currentToken);
		errorFound = true;
	}
}


void SyntaxAnalysis::printSyntaxError(Token token)
{
	cout << "Sintaksna greska! Neocekivani token: ";
	token.printTokenInfo();
}


void SyntaxAnalysis::printTokenInfo(Token token)
{
	token.printTokenInfo();
}

Variables& SyntaxAnalysis::getVariables()
{
	return variables;
}


Instructions& SyntaxAnalysis::getInstructions()
{
	return instructions;
}

std::string SyntaxAnalysis::getFunctionName()
{
	return functionName;
}

Variable* SyntaxAnalysis::findVariable(std::string name)
{
	// Prolazi kroz listu promenljivih i trazi onu sa datim imenom.
	for (Variables::iterator it = variables.begin(); it != variables.end(); ++it)
	{
		if ((*it)->getName() == name)
			return *it;
	}
	// Nije pronadjena.
	return nullptr;
}

void SyntaxAnalysis::emit(Instruction* instr)
{
	// Dodaje gotovu instrukciju u listu instrukcija.
	instructions.push_back(instr);
}

void SyntaxAnalysis::printInstructions()
{
	cout << endl << "=== Lista instrukcija (" << instructions.size() << ") ===" << endl;

	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		Instruction* instr = *it;

		cout << "[" << instr->getPos() << "] tip=" << instr->getType() << "  dst: { ";
		for (Variables::iterator d = instr->getDst().begin(); d != instr->getDst().end(); ++d)
			cout << (*d)->getName() << " ";

		cout << "}  src: { ";
		for (Variables::iterator s = instr->getSrc().begin(); s != instr->getSrc().end(); ++s)
			cout << (*s)->getName() << " ";

		cout << "}" << endl;
	}
}

bool SyntaxAnalysis::Do()
{
	// Dobavi prvi token i pokreni analizu od pocetnog simbola Q.
	currentToken = getNextToken();
	Q();

	return !errorFound;
}


// ============================================================
// Funkcije gramatike (rekurzivni descent)
// ============================================================

// Q -> S ; L
void SyntaxAnalysis::Q()
{
	if (errorFound) return;

	S();
	eat(T_SEMI_COL);
	L();
}


// L -> eof | Q
void SyntaxAnalysis::L()
{
	if (errorFound) return;

	if (currentToken.getType() == T_END_OF_FILE)
	{
		eat(T_END_OF_FILE);
	}
	else
	{
		Q();
	}
}


// S -> _mem mid num | _reg rid | _func id | id: E | E
void SyntaxAnalysis::S()
{
	if (errorFound) return;

	switch (currentToken.getType())
	{
	case T_MEM:			// S -> _mem mid num
	{
		eat(T_MEM);
		Token memTok = currentToken;	// zapamti ime (npr. m1) pre nego sto eat pomeri dalje
		eat(T_M_ID);
		Token numTok = currentToken;	// zapamti vrednost (npr. 6) pre eat
		eat(T_NUM);

		// Napravi memorijsku promenljivu i dodaj je u listu.
		Variable* var = new Variable(memTok.getValue(), -1);
		var->setType(Variable::MEM_VAR);
		var->setValue(atoi(numTok.getValue().c_str()));	// sacuvaj vrednost za .data
		variables.push_back(var);
		break;
	}

	case T_REG:			// S -> _reg rid
	{
		eat(T_REG);
		Token regTok = currentToken;	// zapamti ime (npr. r1) pre eat
		eat(T_R_ID);

		// Napravi registarsku promenljivu i dodaj je u listu.
		Variable* var = new Variable(regTok.getValue(), -1);
		var->setType(Variable::REG_VAR);
		variables.push_back(var);
		break;
	}

	case T_FUNC:		// S -> _func id
	{
		eat(T_FUNC);
		Token funcTok = currentToken;	// zapamti ime funkcije (npr. main)
		eat(T_ID);
		functionName = funcTok.getValue();
		break;
	}

	case T_ID:			// S -> id: E   (labela pa instrukcija)
	{
		Token labelTok = currentToken;	// zapamti ime labele (npr. "lab")
		eat(T_ID);
		eat(T_COL);
		E();							// E() kreira i doda instrukciju u listu

		// Zalepi labelu na poslednju dodatu instrukciju.
		if (!instructions.empty())
			instructions.back()->setLabel(labelTok.getValue());
		break;
	}

	default:			// S -> E (sve instrukcije)
		E();
		break;
	}
}


// E -> jedna od instrukcija
void SyntaxAnalysis::E()
{
	if (errorFound) return;

	switch (currentToken.getType())
	{
	case T_ADD:			// E -> add rid, rid, rid
	{
		eat(T_ADD);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s1 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s2 = currentToken; eat(T_R_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_ADD, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->getSrc().push_back(findVariable(s1.getValue()));
		instr->getSrc().push_back(findVariable(s2.getValue()));
		emit(instr);
		break;
	}

	case T_SUB:			// E -> sub rid, rid, rid
	{
		eat(T_SUB);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s1 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s2 = currentToken; eat(T_R_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_SUB, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->getSrc().push_back(findVariable(s1.getValue()));
		instr->getSrc().push_back(findVariable(s2.getValue()));
		emit(instr);
		break;
	}

	case T_AND:			// E -> and rid, rid, rid
	{
		eat(T_AND);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s1 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s2 = currentToken; eat(T_R_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_AND, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->getSrc().push_back(findVariable(s1.getValue()));
		instr->getSrc().push_back(findVariable(s2.getValue()));
		emit(instr);
		break;
	}

	case T_OR:			// E -> or rid, rid, rid
	{
		eat(T_OR);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s1 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s2 = currentToken; eat(T_R_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_OR, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->getSrc().push_back(findVariable(s1.getValue()));
		instr->getSrc().push_back(findVariable(s2.getValue()));
		emit(instr);
		break;
	}

	case T_ADDI:		// E -> addi rid, rid, num
	{
		eat(T_ADDI);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s1 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token numTok = currentToken; eat(T_NUM);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_ADDI, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->getSrc().push_back(findVariable(s1.getValue()));
		instr->setConstant(atoi(numTok.getValue().c_str()));
		emit(instr);
		break;
	}

	case T_LA:			// E -> la rid, mid
	{
		eat(T_LA);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token memTok = currentToken; eat(T_M_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_LA, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->setJumpLabel(memTok.getValue());	// ime memorije (npr. m1) za la
		emit(instr);
		break;
	}

	case T_LW:			// E -> lw rid, num(rid)
	{
		eat(T_LW);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token numTok = currentToken; eat(T_NUM);
		eat(T_L_PARENT);
		Token s = currentToken; eat(T_R_ID);
		eat(T_R_PARENT);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_LW, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->getSrc().push_back(findVariable(s.getValue()));
		instr->setConstant(atoi(numTok.getValue().c_str()));
		emit(instr);
		break;
	}

	case T_LI:			// E -> li rid, num
	{
		eat(T_LI);
		Token d = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token numTok = currentToken; eat(T_NUM);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_LI, dst, src);
		instr->getDst().push_back(findVariable(d.getValue()));
		instr->setConstant(atoi(numTok.getValue().c_str()));
		emit(instr);
		break;
	}

	case T_SW:			// E -> sw rid, num(rid)  -- nema dst, oba registra su src
	{
		eat(T_SW);
		Token s1 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token numTok = currentToken; eat(T_NUM);
		eat(T_L_PARENT);
		Token s2 = currentToken; eat(T_R_ID);
		eat(T_R_PARENT);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_SW, dst, src);
		instr->getSrc().push_back(findVariable(s1.getValue()));
		instr->getSrc().push_back(findVariable(s2.getValue()));
		instr->setConstant(atoi(numTok.getValue().c_str()));
		emit(instr);
		break;
	}

	case T_B:			// E -> b id
	{
		eat(T_B);
		Token lab = currentToken; eat(T_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_B, dst, src);
		instr->setJumpLabel(lab.getValue());
		emit(instr);
		break;
	}

	case T_BLTZ:		// E -> bltz rid, id
	{
		eat(T_BLTZ);
		Token s = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token lab = currentToken; eat(T_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_BLTZ, dst, src);
		instr->getSrc().push_back(findVariable(s.getValue()));
		instr->setJumpLabel(lab.getValue());
		emit(instr);
		break;
	}

	case T_BEQ:			// E -> beq rid, rid, id
	{
		eat(T_BEQ);
		Token s1 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token s2 = currentToken; eat(T_R_ID);
		eat(T_COMMA);
		Token lab = currentToken; eat(T_ID);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_BEQ, dst, src);
		instr->getSrc().push_back(findVariable(s1.getValue()));
		instr->getSrc().push_back(findVariable(s2.getValue()));
		instr->setJumpLabel(lab.getValue());
		emit(instr);
		break;
	}

	case T_NOP:			// E -> nop
	{
		eat(T_NOP);

		Variables dst, src;
		Instruction* instr = new Instruction(instructionCounter++, I_NOP, dst, src);
		emit(instr);
		break;
	}

	default:			// nijedna instrukcija se ne poklapa - greska
		printSyntaxError(currentToken);
		errorFound = true;
		break;
	}
}