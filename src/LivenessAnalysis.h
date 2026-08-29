#pragma once

#include "IR.h"

class LivenessAnalysis
{
public:

	/**
	 * Konstruktor - prima referencu na listu instrukcija (iz parsera).
	 */
	LivenessAnalysis(Instructions& instructions);

	/**
	 * Pokrece kompletnu analizu zivotnog veka:
	 *  1. popuni use/def skupove
	 *  2. izgradi succ/pred (graf toka)
	 *  3. iterativno izracuna in/out skupove
	 */
	void Do();

	/**
	 * Ispisuje rezultate analize (use, def, in, out za svaku instrukciju).
	 */
	void printLivenessInfo();

private:

	/**
	 * Referenca na listu instrukcija nad kojom se radi analiza.
	 */
	Instructions& instructions;

	/**
	 * Korak A: popunjava use i def skupove svake instrukcije
	 * (kopira iz src i dst koje je parser vec napunio).
	 */
	void fillUseDef();

	/**
	 * Korak B: gradi succ (sledbenike) i pred (prethodnike) svake instrukcije.
	 */
	void buildControlFlow();

	/**
	 * Korak C: iterativni algoritam koji racuna in i out skupove
	 * dok se dve uzastopne iteracije ne izjednace.
	 */
	void calculateInOut();
};