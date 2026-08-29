#pragma once

#include "LexicalAnalysis.h"
#include "IR.h"

class SyntaxAnalysis
{
public:

	/**
	 * Konstruktor - prima referencu na leksicku analizu (izvor tokena).
	 */
	SyntaxAnalysis(LexicalAnalysis& lex);

	/**
	 * Pokrece sintaksnu analizu. Vraca true ako je program sintaksno ispravan.
	 */
	bool Do();

	/**
	 * Vraca listu svih promenljivih prikupljenih tokom parsiranja.
	 */
	Variables& getVariables();

	/**
	 * Vraca listu svih instrukcija prikupljenih tokom parsiranja.
	 */
	Instructions& getInstructions();

	std::string getFunctionName();

	/**
	 * Ispisuje sve prikupljene instrukcije sa njihovim dst i src listama.
	 */
	void printInstructions();

private:
	
	/**
	 * Referenca na objekat leksicke analize.
	 */
	LexicalAnalysis& lexicalAnalysis;

	/**
	 * Iterator kroz listu tokena - pokazuje na trenutni token.
	 */
	TokenList::iterator tokenIterator;

	/**
	 * Trenutni token koji se analizira.
	 */
	Token currentToken;

	/**
	 * Zastavica greske - postaje true ako se naidje na sintaksnu gresku.
	 */
	bool errorFound;

	/**
	 * Lista svih promenljivih (_mem i _reg deklaracije).
	 */
	Variables variables;

	/**
	 * Lista svih instrukcija programa.
	 */
	Instructions instructions;

	/**
	 * Brojac pozicije instrukcije (redni broj u programu).
	 */
	int instructionCounter;

	std::string functionName;

	/**
	 * Pravi instrukciju i dodaje je u listu instrukcija.
	 */
	void emit(Instruction* instr);

	/**
	 * Pomocna funkcija - pronalazi promenljivu po imenu u listi variables.
	 * Vraca pokazivac na nju, ili nullptr ako ne postoji.
	 */
	Variable* findVariable(std::string name);

	/**
	 * Dobavlja sledeci token iz liste i smesta ga u currentToken.
	 */
	Token getNextToken();

	/**
	 * Proverava da li je trenutni token ocekivanog tipa t.
	 * Ako jeste, prelazi na sledeci token. Ako nije, prijavljuje sintaksnu gresku.
	 */
	void eat(TokenType t);

	/**
	 * Ispisuje informacije o sintaksnoj gresci (token koji je izazvao gresku).
	 */
	void printSyntaxError(Token token);

	/**
	 * Ispisuje informacije o uspesno prepoznatom tokenu.
	 */
	void printTokenInfo(Token token);

	/**
	 * Funkcije za neterminalne simbole gramatike (rekurzivni descent).
	 * Q -> S ; L
	 * S -> deklaracija ili instrukcija
	 * E -> jedna instrukcija
	 */
	void Q();
	void S();
	void E();
	void L();
};