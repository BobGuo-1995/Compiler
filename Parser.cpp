#include "Parser.h"
#include "Coder.h"


void PrintStatement(TokenStream& ts);
void ParseCompoundStatement(TokenStream& ts);
void ParseStatement(TokenStream& ts);
void ParseIOStatement(TokenStream& ts);
void ParseLValue(TokenStream& ts);
void ParseValue(TokenStream& ts);
void ParseBoolExpr(TokenStream& ts);
void ParseBoolFactor(TokenStream& ts);
bool ParseBoolTerm(TokenStream& ts);
void ParseFactor(TokenStream& ts);
void ParseCondition(TokenStream& ts);
void ParseExpression(TokenStream& ts);
void ParseTerm(TokenStream& ts);
void ParseFactor(TokenStream& ts);

//Constant storage. 
//Example 
//const a=12; => constMap["a"]=12
unordered_map<string, int> constMap;

//map of variables. Key - variable name, Value - adress of memory
//Example 
//var a; => varMap["a"]=adress of memory (offset from fp)
unordered_map<string, int> varMap;

//map of array. Key - array name, Value - adress of memory
//Example 
//var a[10]; => arrMap["a"]=first element adress of memory  (offset 4*10 from fp)
unordered_map<string, int> arrMap;


//Syntax diagram
//****Figure Program****
//Parse declaration functions
void ParseConstant(TokenStream& ts)
{

	while (true)
	{
		Token* t = ts.Next();
		if (t->kind == Kind::name){
			if (constMap.find(t->varName) == constMap.end())
			{
				if (ts.Next()->kind == Kind::equal)
				{
					if (ts.Next()->kind == Kind::number){
						constMap[t->varName] = ts.Current()->val;
						ts.Next();
						if (ts.Current()->kind == Kind::semicolon){
							 return;
						}
						if (ts.Current()->kind == Kind::comma) continue;
					}
				}
			}
		}
		//Error
		throw invalid_argument("invalid constant");
	}
}
void ParseVariables(TokenStream& ts)
{
	while (true)
	{
		Token* t = ts.Next();
		if (t->kind == Kind::name){
			string varName = t->varName;
			if (varMap.find(t->varName) == varMap.end() && arrMap.find(t->varName) == arrMap.end() && constMap.find(t->varName) == constMap.end())
			{//var A[...]

				if (ts.Next()->kind == Kind::left_bracket){
					ts.Next();
					//A[number]
					if (ts.Current()->kind == Kind::number){
						DeclareVariable(varName, ts.Current()->val);
					}
					else if (ts.Current()->kind == Kind::name){
						//A[const]
						auto it = constMap.find(ts.Current()->varName);
						if (it != constMap.end()){
							DeclareVariable(varName, it->second);
						}
						else{ throw invalid_argument("Not found const name"); }

					}
					else  throw invalid_argument("Invalid array");
					if (ts.Next()->kind != Kind::right_bracket) throw invalid_argument("Invalid Array not found right bracket");
					ts.Next();
				}

				else DeclareVariable(varName);
				t = ts.Current();

				if (ts.Current()->kind == Kind::semicolon) return;
				if (ts.Current()->kind == Kind::comma) continue;
			}
		}
		//Error
		throw invalid_argument("invalid variables");
	}
}
void ParseDeclaration(TokenStream& ts)
{
	ts.Next();
	if (ts.Current()->kind == Kind::constants)
	{
		ParseConstant(ts);
		ts.Next();
	}
	if (ts.Current()->kind == Kind::variables)
	{
		ParseVariables(ts);
		ts.Next();
	}
}
//**********************************
//End parse declaration

void ParseExpression(TokenStream& ts);
void ParseCompoundStatement(TokenStream& ts);
void ParseIOStatement(TokenStream& ts);
void ParseValue(TokenStream& ts);
void ParseBoolExpr(TokenStream& ts);
//***************
//Helper functions
//*************
void TestVariable(Token* t)
{
	auto varName = t->varName;
	auto it = varMap.find(varName);
	if (it == varMap.end())throw invalid_argument("undeclared variable " + varName);
}
void TestConst(Token* t){
	auto constName = t->varName;
	auto it = constMap.find(constName);
	if (it == constMap.end())throw invalid_argument("undeclared const " + constName);
}
void TestArray(Token* t){
	auto arrName = t->varName;
	auto it = arrMap.find(arrName);
	if (it == arrMap.end())throw invalid_argument("undeclared array " + arrName);
}
void TestConstVariable(Token* t){
	try{
		TestConst(t);
	}
	catch (exception e){
		TestVariable(t);
	}
}
void TestIdentifier(Token* t){
	try{
		TestConst(t);
	}
	catch (exception e){
		try {
			TestArray(t);
		}
		catch (exception e){
			TestVariable(t);
		}
	}
}
//End Helper functions
//****************

//Figure Compound Statement
void ParseCompoundStatement(TokenStream& ts)
{
	if (ts.Current()->kind == Kind::begin_k){
		ts.Next();
		while (ts.Current()->kind != Kind::end_k)
		{
			ParseStatement(ts);
			switch (ts.Current()->kind)
			{
			case Kind::semicolon:
				PrintStatement(ts);
				ts.Next();
				continue;
				break;
			case Kind::end_k:
				PrintStatement(ts);
				continue;
				break;
			default:
				throw invalid_argument("bad compound statement");
				break;
			}
		}
		ts.Next();
	}
}

//Figure Statement
void ParseStatement(TokenStream& ts){
	switch (ts.Current()->kind)
	{
	case Kind::name:
		ParseLValue(ts);
		if (ts.Current()->kind != Kind::assignment) throw invalid_argument("missing assignment");
		ts.Next();
		ParseExpression(ts);
		AssignVariable();
		break;
	case Kind::begin_k:
		ParseCompoundStatement(ts);
		break;
	case Kind::if_k:
		ts.Next();
		ParseBoolExpr(ts);
		if (ts.Current()->kind != Kind::then_k) throw invalid_argument("missing then in if ... then construction");
		//Then_K();//Coder
		ts.Next();
		ParseStatement(ts);
		if (ts.Current()->kind != Kind::else_k){
			SetLabelFalse(); //Coder
			break;
		}
		Else_k();//Coder
		ts.Next();
		ParseStatement(ts);
		SetLabelFalse();//Coder
		break;
	case Kind::while_k:
		ts.Next();
		While_k();//Coder
		ParseBoolExpr(ts);
		if (ts.Current()->kind != Kind::do_k)throw invalid_argument("missing do in while loop construction");
		ts.Next();
		ParseStatement(ts);
		While_k_end();//Coder
		break;
	case Kind::for_k:
	{
		if (ts.Next()->kind != Kind::name)throw invalid_argument("missing variable in for loop construction");
		Token* t = ts.Current();
		TestVariable(t);
		if (ts.Next()->kind != Kind::assignment)throw invalid_argument("missing assignment in for loop construction");
		ts.Next();
		ParseExpression(ts);
		if (ts.Current()->kind != Kind::to_k)throw invalid_argument("missing to in for loop construction");
		ts.Next();
		SetVariableToLValueStack(t);//Coder
		AssignVariable();//Coder
		While_k();
		ParseExpression(ts);
		For_k(t);//Coder
		if (ts.Current()->kind != Kind::do_k)throw invalid_argument("missing do in for loop construction");
		ts.Next();
		ParseStatement(ts);
		For_k_end();//Coder
	}
	break;
	case Kind::readchar_k:
	case Kind::read_k:
	case Kind::writechar_k:
	case Kind::write_k:
		ParseIOStatement(ts);
		break;
	default:
		throw invalid_argument("bad statement");
		break;
	}


}

//Figure IOStatement
void ParseIOStatement(TokenStream& ts)
{
	switch (ts.Current()->kind)
	{
	case Kind::readchar_k:
		ts.Next();
		ParseLValue(ts);
		ReadChar_k();//Coder
		break;
	case Kind::writechar_k:
		ts.Next();
		ParseValue(ts);
		WriteChar_K();//Coder
		break;
	case Kind::read_k:
		ts.Next();
		ParseLValue(ts);
		if (ts.Current()->kind != Kind::comma)throw invalid_argument("missing comma in read");
		ts.Next();
		ParseLValue(ts);
		Read_k();//Coder
		break;
	case Kind::write_k:
		ts.Next();
		ParseValue(ts);
		Write_K();//Coder
		break;
	default:
		throw invalid_argument("bad IO Statement");
		break;
	}
}

//Figure LValue
void ParseLValue(TokenStream& ts){
	if (ts.Current()->kind == Kind::name){
		auto itVar = varMap.find(ts.Current()->varName);
		if (itVar != varMap.end()){
			//********
			//Its variable
			SetVariableToLValueStack(ts.Current());
		}
		else{
			//It's array
			auto itArr = arrMap.find(ts.Current()->varName);
			if (itArr != arrMap.end()){
				Token *t = ts.Current();
				if (ts.Next()->kind != Kind::left_bracket) throw invalid_argument("invalid Lvalue Array missing left bracket");
				ts.Next();
				ParseExpression(ts);
				if (ts.Current()->kind != Kind::right_bracket)throw invalid_argument("invalid Lvalue Array missing right bracket");
				//Do something
				SetArrayToLvalueStack(t);//Coder
			}
			else throw invalid_argument("Lvalue: undeclared variable " + ts.Current()->varName);
		}
		ts.Next();
	}
	else {
		throw invalid_argument("invalid Lvalue");
	}
}

//Figure Value
void ParseValue(TokenStream& ts)
{
	switch (ts.Current()->kind)
	{
	case Kind::name:
	{Token* t = ts.Current();
	TestIdentifier(ts.Current());
	if (ts.Next()->kind != Kind::left_bracket){ SetValueToStack(t); return; }
	ts.Next();
	ParseExpression(ts);
	if (ts.Current()->kind != Kind::right_bracket) throw invalid_argument("missing right bracket in array");
	//Do something
	SetArrayToStack(t);//Coder
	}
	break;
	case Kind::number:
		SetValueToStack(ts.Current());
		//Do something
		break;
	default:
		throw invalid_argument("bad value");
		break;
	}
	ts.Next();
}

// Figure Boolxxpr
//The function generates code where the next command is executed when the expression is true. 
// And add label to labelsFalse stack for false result 
//
//Boolexpression->IsTrue->next command. 
void ParseBoolExpr(TokenStream& ts){
	GenerateLabelTrue();
	bool isAnd = false;
	while (true){

		isAnd = ParseBoolTerm(ts);

		if (ts.Current()->kind != Kind::or_k){
			break;
		}
		Or_k();//Coder
		if (isAnd)SetLabelFalse();
		ts.Next();
	}
	Then_K(isAnd);//Coder
}

//Figure Boolterm
bool ParseBoolTerm(TokenStream& ts){

	bool isAnd = false;
	while (true){
		ParseBoolFactor(ts);
		if (ts.Current()->kind != Kind::and_k){
			break;
		}
		if (!isAnd) {
			GenerateLabelFalse(); isAnd = true;
		}
		And_k();//Coder

		ts.Next();
	}
	return isAnd;
}

//Figure Boolfactor
void ParseBoolFactor(TokenStream& ts){

	switch (ts.Current()->kind)
	{
	case Kind::left_parenthesis:
		ts.Next();
		ParseBoolExpr(ts);
		if (ts.Current()->kind != Kind::right_parenthesis)throw invalid_argument("missing right parenthesis");
		{
			Token tExpr;
			tExpr.kind = Kind::boolexpr_b;
			SetValueToStack(&tExpr);
		}
		ts.Next();
		break;
	case Kind::not_k:
		ts.Next();
		{
			ParseBoolFactor(ts);
			Not_k();//Coder
		}
		break;
	default:
		//Condition
		ParseCondition(ts);
		break;
	}
}

//Figure Condition
void ParseCondition(TokenStream& ts){
	Token *t;
	if (ts.Current()->kind == Kind::odd){
		ts.Next();
		ParseValue(ts);
		Odd_k();//Coder
		return;
		//****
	}
	ParseValue(ts);
	switch (ts.Current()->kind)
	{
	case Kind::less:
	case Kind::less_or_equal:
	case Kind::greater:
	case Kind::greater_or_equal:
	case Kind::equal:
	case Kind::not_equal:
		t = ts.Current();
		break;
	default:
		throw invalid_argument("bad condition");
		break;
	}
	ts.Next();
	ParseValue(ts);
	Condition(t);//Coder
}

//Figure Expression
void ParseExpression(TokenStream& ts){
	Token* temp = nullptr;
	while (true){

		ParseTerm(ts);
		MathFunction(temp);//Coder
		switch (ts.Current()->kind)
		{
		case Kind::add:
		case Kind::minus:
			temp = ts.Current();
			break;
		default:

			return;
			break;
		}
		ts.Next();
	}
}

//Figure Term
void ParseTerm(TokenStream& ts){
	Token* temp = nullptr;
	while (true){
		ParseFactor(ts);
		MathFunction(temp);//Coder
		switch (ts.Current()->kind)
		{
		case Kind::multiply:
		case Kind::divide:
		case Kind::remainder_k:
			temp = ts.Current();
			break;
		default:

			return;
			break;

		}
		ts.Next();
	}
}

//Figure Factor
void ParseFactor(TokenStream& ts){
	switch (ts.Current()->kind)
	{
	case Kind::name:
	{
		Token* tName = ts.Current();
		if (ts.Next()->kind != Kind::left_bracket){
			//identifier
			TestConstVariable(tName);
			SetValueToStack(tName);//Coder
		}
		else{
			ts.Next();
			ParseExpression(ts);
			TestArray(tName);
			SetArrayToStack(tName);//Coder
			if (ts.Current()->kind != Kind::right_bracket)throw invalid_argument("missing right bracket");
			ts.Next();
		}

	}
	break;
	case Kind::number:
		SetValueToStack(ts.Current());//Coder
		//number
		ts.Next();
		break;
	case Kind::minus:
		//unary minus
	{Token t0;
	t0.kind = Kind::reg;
	t0.varName = "g0";
	SetValueToStack(&t0); }
	ts.Next();
	ParseFactor(ts);
	{Token tMinus;
	tMinus.kind = Kind::minus;
	MathFunction(&tMinus);
	}
	break;
	case Kind::left_parenthesis:
		ts.Next();
		ParseExpression(ts);
		if (ts.Current()->kind != Kind::right_parenthesis)throw invalid_argument("missing right parenthesis");
		ts.Next();
		break;
	default:
		throw invalid_argument("bad factor");
		break;
	}

}

//***End Syntax Diagram****


//Print statement from source code in comments
void PrintStatement(TokenStream& ts)
{
	string tmp;
	size_t posBegin = ts.GetPos()+1;
	size_t pos = posBegin;
	for (; pos < ts.vToken.size(); ++pos){
		if (ts.vToken[pos].kind == Kind::semicolon)break;
	}
	if (pos >= ts.vToken.size())
		tmp.assign(ts.txt.begin() + ts.vToken[posBegin].pos, ts.txt.end());
	else
		tmp.assign(ts.txt.begin() + ts.vToken[posBegin].pos, ts.txt.begin() + ts.vToken[pos].pos);
	for (size_t i = 0;; ++i){
		 i= tmp.find('\n', i);
		 if (i == string::npos)break;
		 tmp.replace(tmp.begin() + i, tmp.begin() + i+  1, "\n//");
	}
	WriteToCode("//" + tmp );
}

//Main function for parse
void Parse(TokenStream& ts, const string& fnameOutC/*output file name*/)
{
	cout << "**Parser**" << endl;
	ofstream streamOutC(fnameOutC);
	streamOutC << "#include \"csc633.h\"" << endl << "code" << endl;
	InitCoder(&streamOutC);
	try{
		ParseDeclaration(ts);
		PrintStatement(ts);
		ParseCompoundStatement(ts);
		if (ts.Current()->kind != Kind::period)throw invalid_argument("missing period");
		if (ts.Next()->kind != Kind::EOF_k)throw invalid_argument("unexpected end");
		streamOutC << ";"<<endl << "end" << endl;
		return;
	}
	catch (exception e){
		cout << e.what() << " in line: " << ts.Current()->line << ", column: " << ts.Current()->column << endl;
	}
}