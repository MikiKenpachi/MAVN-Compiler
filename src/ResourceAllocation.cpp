#include "ResourceAllocation.h"

#include <iostream>

using namespace std;


ResourceAllocation::ResourceAllocation(Variables& vars, Instructions& instr)
	: variables(vars), instructions(instr)
{
	ig.matrix = nullptr;
	ig.size = 0;
}


void ResourceAllocation::initVariables()
{
	// Izdvoji samo registarske promenljive i numerisi ih (pos = indeks u matrici).
	int index = 0;
	for (Variables::iterator it = variables.begin(); it != variables.end(); ++it)
	{
		if ((*it)->getType() == Variable::REG_VAR)
		{
			(*it)->setPos(index);
			ig.variables.push_back(*it);
			index++;
		}
	}

	ig.size = index;
}


void ResourceAllocation::buildInterferenceGraph()
{
	// Alociraj kvadratnu matricu size x size i inicijalizuj na EMPTY.
	ig.matrix = new char* [ig.size];
	for (int i = 0; i < ig.size; i++)
	{
		ig.matrix[i] = new char[ig.size];
		for (int j = 0; j < ig.size; j++)
			ig.matrix[i][j] = EMPTY;
	}

	// Za svaku instrukciju gradimo smetnje po dva pravila:
	//  1) sve promenljive u 'out' skupu medjusobno smetaju
	//  2) svaka def promenljiva smeta svakoj promenljivoj u 'out'
	//     (def koja se odmah prepise nikad ne udje u out, pa je hvata samo ovo pravilo)
	for (Instructions::iterator it = instructions.begin(); it != instructions.end(); ++it)
	{
		Instruction* instr = *it;
		Variables& out = instr->getOut();
		Variables& def = instr->getDef();

		// Pravilo 1: svaki par (a, b) iz out skupa.
		for (Variables::iterator a = out.begin(); a != out.end(); ++a)
		{
			for (Variables::iterator b = out.begin(); b != out.end(); ++b)
			{
				if (*a == *b)
					continue;	// promenljiva ne smeta sama sebi

				int posA = (*a)->getPos();
				int posB = (*b)->getPos();

				if (posA >= 0 && posB >= 0)
				{
					ig.matrix[posA][posB] = INTERFERENCE;
					ig.matrix[posB][posA] = INTERFERENCE;
				}
			}
		}

		// Pravilo 2: svaki def smeta svakoj promenljivoj u out.
		for (Variables::iterator d = def.begin(); d != def.end(); ++d)
		{
			for (Variables::iterator o = out.begin(); o != out.end(); ++o)
			{
				if (*d == *o)
					continue;

				int posD = (*d)->getPos();
				int posO = (*o)->getPos();

				if (posD >= 0 && posO >= 0)
				{
					ig.matrix[posD][posO] = INTERFERENCE;
					ig.matrix[posO][posD] = INTERFERENCE;
				}
			}
		}
	}
}


void ResourceAllocation::printInterferenceGraph()
{
	cout << endl << "=== Graf smetnji (matrica) ===" << endl;

	// Zaglavlje sa imenima kolona.
	cout << "      ";
	for (Variables::iterator it = ig.variables.begin(); it != ig.variables.end(); ++it)
		cout << (*it)->getName() << " ";
	cout << endl;

	// Redovi.
	int i = 0;
	for (Variables::iterator it = ig.variables.begin(); it != ig.variables.end(); ++it)
	{
		cout << "  " << (*it)->getName() << ":  ";
		for (int j = 0; j < ig.size; j++)
			cout << ig.matrix[i][j] << "  ";
		cout << endl;
		i++;
	}
}


bool ResourceAllocation::Do()
{
	initVariables();
	buildInterferenceGraph();

	// Simplify: skidanje cvorova na stek. Ako se zaglavi -> spill.
	if (!doSimplification())
		return false;

	// Select: bojenje sa steka. Ako neki cvor ne moze da se oboji -> spill.
	if (!doSelection())
		return false;

	return true;
}


bool ResourceAllocation::doSimplification()
{
	// Lista da li je cvor vec skinut sa grafa (po indeksu pos).
	bool* removed = new bool[ig.size];
	for (int i = 0; i < ig.size; i++)
		removed[i] = false;

	int remaining = ig.size;	// koliko cvorova je jos u grafu

	// Ponavljaj dok ima cvorova u grafu.
	while (remaining > 0)
	{
		bool foundNode = false;

		// Trazi cvor sa stepenom < REG_NUMBER koji jos nije skinut.
		for (Variables::iterator it = ig.variables.begin(); it != ig.variables.end(); ++it)
		{
			Variable* var = *it;
			int pos = var->getPos();

			if (removed[pos])
				continue;	// vec skinut

			// Izracunaj stepen: broj suseda koji su jos u grafu.
			int degree = 0;
			for (int j = 0; j < ig.size; j++)
				if (!removed[j] && ig.matrix[pos][j] == INTERFERENCE)
					degree++;

			// Ako ima manje od REG_NUMBER suseda, skini ga na stek.
			if (degree < REG_NUMBER)
			{
				simplificationStack.push(var);
				removed[pos] = true;
				remaining--;
				foundNode = true;
				break;	// kreni iz pocetka (stepeni su se promenili)
			}
		}

		// Ako nijedan cvor nema < REG_NUMBER suseda, a jos ih ima -> spill.
		if (!foundNode)
		{
			delete[] removed;
			return false;	// SPILL
		}
	}

	delete[] removed;
	return true;
}


bool ResourceAllocation::doSelection()
{
	// Vadi cvorove sa steka (obrnutim redom) i boji svaki.
	while (!simplificationStack.empty())
	{
		Variable* var = simplificationStack.top();
		simplificationStack.pop();

		int pos = var->getPos();

		// Koje su boje (registri) zauzete kod suseda?
		bool usedColor[REG_NUMBER];
		for (int i = 0; i < REG_NUMBER; i++)
			usedColor[i] = false;

		// Prodji kroz susede i oznaci njihove boje kao zauzete.
		for (Variables::iterator it = ig.variables.begin(); it != ig.variables.end(); ++it)
		{
			Variable* other = *it;
			if (other == var)
				continue;

			if (ig.matrix[pos][other->getPos()] == INTERFERENCE)
			{
				Regs c = other->getAssignment();
				if (c != no_assign && (int)c - 1 >= 0 && (int)c - 1 < REG_NUMBER)
					usedColor[(int)c - 1] = true;
			}
		}

		// Nadji prvu slobodnu boju.
		int chosen = -1;
		for (int i = 0; i < REG_NUMBER; i++)
		{
			if (!usedColor[i])
			{
				chosen = i;
				break;
			}
		}

		// Nema slobodne boje -> spill.
		if (chosen == -1)
			return false;

		// Dodeli registar (t0 = 1, t1 = 2, ...). no_assign = 0, pa +1.
		var->setAssignment((Regs)(chosen + 1));
	}

	return true;
}


void ResourceAllocation::printAllocation()
{
	cout << endl << "=== Dodela registara ===" << endl;

	const char* regNames[] = { "no_assign", "t0", "t1", "t2", "t3" };

	for (Variables::iterator it = ig.variables.begin(); it != ig.variables.end(); ++it)
	{
		Variable* var = *it;
		int a = (int)var->getAssignment();
		cout << "  " << var->getName() << "  ->  " << regNames[a] << endl;
	}
}