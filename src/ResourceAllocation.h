#pragma once

#include "IR.h"
#include <stack>

// Broj registara na ciljnoj platformi (t0, t1, t2, t3).
#define REG_NUMBER 4

// Oznake u matrici smetnji.
#define INTERFERENCE '1'
#define EMPTY        '0'


/**
 * Graf smetnji - kvadratna matrica gde red/kolona predstavljaju registarske
 * promenljive, a vrednost '1' znaci da dve promenljive smetaju jedna drugoj.
 */
struct InterferenceGraph
{
	Variables variables;	// sve registarske promenljive (cvorovi grafa)
	char** matrix;			// matrica smetnji (size x size)
	int size;				// dimenzija matrice
};


class ResourceAllocation
{
public:

	/**
	 * Konstruktor - prima liste promenljivih i instrukcija.
	 */
	ResourceAllocation(Variables& variables, Instructions& instructions);

	/**
	 * Pokrece kompletnu alokaciju:
	 *  1. build  - napravi graf smetnji
	 *  2. simplify - skidaj cvorove na stek
	 *  3. select - boji sa steka
	 * Vraca true ako je alokacija uspela, false ako je doslo do spill-a.
	 */
	bool Do();

	/**
	 * Ispisuje matricu smetnji.
	 */
	void printInterferenceGraph();

	/**
	 * Ispisuje dodeljene registre svakoj promenljivoj.
	 */
	void printAllocation();

private:

	Variables& variables;
	Instructions& instructions;

	InterferenceGraph ig;
	std::stack<Variable*> simplificationStack;

	/**
	 * Numerise registarske promenljive (postavlja pos) i izdvaja ih u graf.
	 */
	void initVariables();

	/**
	 * Build faza: pravi matricu smetnji iz out skupova instrukcija.
	 */
	void buildInterferenceGraph();

	/**
	 * Simplify faza: skida cvorove sa manje od REG_NUMBER suseda na stek.
	 * Vraca false ako se zaglavi (spill).
	 */
	bool doSimplification();

	/**
	 * Select faza: boji cvorove sa steka (dodeljuje registre).
	 * Vraca false ako neki cvor ne moze da se oboji (spill).
	 */
	bool doSelection();
};