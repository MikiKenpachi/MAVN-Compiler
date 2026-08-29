#include "LexicalAnalysis.h"
#include "SyntaxAnalysis.h"
#include "LivenessAnalysis.h"
#include "ResourceAllocation.h"
#include "CodeGeneration.h"

#include <iostream>
#include <exception>

using namespace std;

void main()
{
	try
	{
		std::string fileName = ".\\..\\examples\\simple.mavn";
		bool retVal = false;

		LexicalAnalysis lex;
		
		if (!lex.readInputFile(fileName))
			throw runtime_error("\nException! Failed to open input file!\n");
		
		lex.initialize();

		retVal = lex.Do();

		if (retVal)
		{
			cout << "Lexical analysis finished successfully!" << endl;
			lex.printTokens();

			// --- Sintaksna analiza ---
			SyntaxAnalysis syntax(lex);
			if (syntax.Do())
			{
				cout << endl << "Syntax analysis finished successfully!" << endl;
				syntax.printInstructions();

				// --- Analiza zivotnog veka ---
				LivenessAnalysis liveness(syntax.getInstructions());
				liveness.Do();
				liveness.printLivenessInfo();

				// --- Alokacija registara ---
				ResourceAllocation allocation(syntax.getVariables(), syntax.getInstructions());
				if (allocation.Do())
				{
					cout << endl << "Resource allocation finished successfully!" << endl;
					allocation.printAllocation();

					// --- Generisanje MIPS koda ---
					CodeGeneration codeGen(syntax.getVariables(), syntax.getInstructions(), syntax.getFunctionName());
					codeGen.generate(".\\..\\examples\\output.s");
				}
				else
				{
					cout << endl << "SPILL detected! Cannot allocate registers." << endl;
				}
			}
			else
			{
				throw runtime_error("\nException! Syntax analysis failed!\n");
			}
		}
		else
		{
			lex.printLexError();
			throw runtime_error("\nException! Lexical analysis failed!\n");
		}

	}
	catch (runtime_error e)
	{
		cout << e.what() << endl;
	}
}