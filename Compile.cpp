#include "Compile.h"
#include "Scanner.h"
#include "Parser.h"


int main(int nargs, char *args[])
{
	
	string fname="test.txt";
	string fnameOutC;
	//string fname(R"(E:\temp\DavidLane\info for lexical analyer 2\info for lexical analyer\error test.txt)");
	if (nargs == 2)fname = args[1];
	fnameOutC = fname + ".c";
	TokenStream sToken;
	CreateTokensFromFile(fname, sToken);
	Parse(sToken, fnameOutC);
	return 0;
}