#include "LivenessAnalysis.h"

#include <iostream>

using namespace std;


LivenessAnalysis::LivenessAnalysis(Instructions& instr)
	: instructions(instr)
{
}


void LivenessAnalysis::Do()
{
	fillUseDef();			// korak A: use = src, def = dst
	buildControlFlow();		// korak B: succ / pred
	calculateInOut();		// korak C: in / out (iterativno)
}


void LivenessAnalysis::fillUseDef()
{
	// Za svaku instrukciju: use je ono sto cita (src), def je ono sto pise (dst).
	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		Instruction* instr = *it;

		// use = src
		for (Variables::iterator s = instr->getSrc().begin(); s != instr->getSrc().end(); ++s)
			instr->getUse().push_back(*s);

		// def = dst
		for (Variables::iterator d = instr->getDst().begin(); d != instr->getDst().end(); ++d)
			instr->getDef().push_back(*d);
	}
}


// Pomocna: pronalazi instrukciju koja nosi datu labelu (linearna pretraga).
static Instruction* findInstructionByLabel(Instructions& instructions, std::string label)
{
	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		if ((*it)->getLabel() == label)
			return *it;
	}
	return nullptr;
}


void LivenessAnalysis::buildControlFlow()
{
	// 1) Za svaku instrukciju odredi sledbenike (succ).
	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		Instruction* instr = *it;

		// Iterator na sledecu instrukciju (ako postoji).
		Instructions::iterator next = it;
		++next;

		InstructionType t = instr->getType();

		if (t == I_B)
		{
			// Bezuslovni skok: sledbenik je samo ciljna instrukcija.
			Instruction* target = findInstructionByLabel(instructions, instr->getJumpLabel());
			if (target != nullptr)
				instr->getSucc().push_back(target);
		}
		else if (t == I_BLTZ || t == I_BEQ)
		{
			// Uslovni skok: ciljna instrukcija + sledeca instrukcija.
			Instruction* target = findInstructionByLabel(instructions, instr->getJumpLabel());
			if (target != nullptr)
				instr->getSucc().push_back(target);

			if (next != instructions.end() && *next != target)
				instr->getSucc().push_back(*next);
		}
		else
		{
			// Obicna instrukcija: sledbenik je sledeca instrukcija.
			if (next != instructions.end())
				instr->getSucc().push_back(*next);
		}
	}

	// 2) Izvedi prethodnike (pred) iz sledbenika: ako je B u succ[A], onda je A u pred[B].
	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		Instruction* instr = *it;
		for (std::list<Instruction*>::iterator s = instr->getSucc().begin(); s != instr->getSucc().end(); ++s)
			(*s)->getPred().push_back(instr);
	}
}


// Pomocna: unija - dodaje sve iz 'source' u 'dest' bez duplikata.
static void unionVariables(Variables& dest, Variables& source)
{
	for (Variables::iterator it = source.begin(); it != source.end(); ++it)
		if (!variableExists(*it, dest))
			dest.push_back(*it);
}

// Pomocna: da li su dve liste promenljivih jednake (isti elementi, bez obzira na redosled).
static bool sameVariables(Variables& a, Variables& b)
{
	if (a.size() != b.size())
		return false;
	for (Variables::iterator it = a.begin(); it != a.end(); ++it)
		if (!variableExists(*it, b))
			return false;
	return true;
}


void LivenessAnalysis::calculateInOut()
{
	bool changed = true;

	// Ponavljaj dok se skupovi ne ustale (dok ima promena).
	while (changed)
	{
		changed = false;

		// Prolaz unazad - od poslednje ka prvoj instrukciji.
		for (Instructions::reverse_iterator rit = instructions.rbegin(); rit != instructions.rend(); ++rit)
		{
			Instruction* instr = *rit;

			// Zapamti stare in i out da bismo proverili da li se promenilo.
			Variables oldIn = instr->getIn();
			Variables oldOut = instr->getOut();

			// out[n] = unija in[s] za sve sledbenike s
			Variables newOut;
			for (std::list<Instruction*>::iterator s = instr->getSucc().begin(); s != instr->getSucc().end(); ++s)
				unionVariables(newOut, (*s)->getIn());

			// in[n] = use[n] U (out[n] - def[n])
			Variables newIn;
			unionVariables(newIn, instr->getUse());			// dodaj use
			for (Variables::iterator o = newOut.begin(); o != newOut.end(); ++o)
				if (!variableExists(*o, instr->getDef()))	// out - def
					if (!variableExists(*o, newIn))
						newIn.push_back(*o);

			// Upisi nove skupove.
			instr->getIn() = newIn;
			instr->getOut() = newOut;

			// Da li se nesto promenilo?
			if (!sameVariables(oldIn, newIn) || !sameVariables(oldOut, newOut))
				changed = true;
		}
	}
}


void LivenessAnalysis::printLivenessInfo()
{
	cout << endl << "=== Analiza zivotnog veka ===" << endl;

	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		Instruction* instr = *it;

		cout << "[" << instr->getPos() << "]  use: { ";
		for (Variables::iterator v = instr->getUse().begin(); v != instr->getUse().end(); ++v)
			cout << (*v)->getName() << " ";

		cout << "}  def: { ";
		for (Variables::iterator v = instr->getDef().begin(); v != instr->getDef().end(); ++v)
			cout << (*v)->getName() << " ";

		cout << "}  succ: { ";
		for (std::list<Instruction*>::iterator s = instr->getSucc().begin(); s != instr->getSucc().end(); ++s)
			cout << (*s)->getPos() << " ";

		cout << "}  pred: { ";
		for (std::list<Instruction*>::iterator p = instr->getPred().begin(); p != instr->getPred().end(); ++p)
			cout << (*p)->getPos() << " ";

		cout << "}  in: { ";
		for (Variables::iterator v = instr->getIn().begin(); v != instr->getIn().end(); ++v)
			cout << (*v)->getName() << " ";

		cout << "}  out: { ";
		for (Variables::iterator v = instr->getOut().begin(); v != instr->getOut().end(); ++v)
			cout << (*v)->getName() << " ";

		cout << "}" << endl;
	}
}