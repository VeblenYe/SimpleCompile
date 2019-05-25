
#include "Lexer.h"
#include "Grammar.h"


using namespace std;


int main() {
	
	ifstream input("expression.txt");
	Lexer l;
	while(!input.eof())
		l.scan(input);
	// l.show();

	ifstream grammar("grammar.txt");
	Grammar g;
	g.scan(grammar);
	/*
	g.convert_to_LL1();
	g.print();
	g.analysis(l);
	*/
	g.convert_to_OPG();
	g.print();
	g.OPG_analysis(l);

	return 0;
}
