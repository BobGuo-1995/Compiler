#include "Coder.h"


unordered_map<Kind, string>sams = { { Kind::add, "add" }, { Kind::minus, "sub" }, { Kind::multiply, "mul" }, { Kind::divide, "div" }, { Kind::remainder_k, "mod" }, { Kind::equal, "be" }, { Kind::greater, "bg" }, { Kind::greater_or_equal, "bge" }, { Kind::less, "bl" }, { Kind::less_or_equal, "ble" } };
int fp = 0;
extern unordered_map<string, int> constMap;
extern unordered_map<string, int> varMap;
extern unordered_map<string, int> arrMap;

ofstream* streamOutC = nullptr;
string scode;
stack<Token> tStack;//stack for values and registers
stack<Token> lStack;//stack for LValues
unordered_map<string, int> freeRegisters;//state of registers (free-1 or busy-0)

string GetAdrrArrayElement(Token* t);
//**************************************************************
void InitCoder(ofstream* stream)
{
	streamOutC = stream;
	for (int i = 0; i < 8; ++i){
		freeRegisters["r" + to_string(i)] = 1;
	}
}
void WriteToCode(const string& op)
{
	cout << op << endl;
	*streamOutC << op << endl;
	//scode += op+"\n";
}
void WriteToCode(const string& op, const string& arg0)
{
	WriteToCode(op + "(" + arg0 + ")");
}
void WriteToCode(const string& op, const string& arg0, const string& arg1)
{
	WriteToCode(op + "(" + arg0 + "," + arg1 + ")");
}
void WriteToCode(const string& op, const string& arg0, const string& arg1, const string& arg2)
{
	WriteToCode(op + "(" + arg0 + "," + arg1 + "," + arg2 + ")");
}
string AllocRegister()
{
	for (auto &item : freeRegisters){
		if (item.second == 1){
			item.second = 0;
			return item.first;
		}
	}
	throw invalid_argument("out of registers");
}
void FreeRegister(string registerName)
{
	if (freeRegisters.find(registerName) != freeRegisters.end())
		freeRegisters[registerName] = 1;
}
string GetFunc(Token* t/*function*/, string x, string y, string z){
	auto it = sams.find(t->kind);
	string sfunc;
	if (it != sams.end()){
		sfunc = it->second + "(" + x + "," + y + "," + z + ")";
		return sfunc;
	}
	throw invalid_argument("Can't find asm function");
}

string GetCode(){ return scode; }
void ChangeConstTokenToNumber(Token& t){
	auto it = constMap.find(t.varName);
	if (it != constMap.end()){
		t.kind = Kind::number;
		t.val = it->second;
	}
}
string GetValueFromToken(Token& t){
	string value;
	ChangeConstTokenToNumber(t);
	switch (t.kind){
	case Kind::number:
		value = to_string(t.val);
		break;
	case Kind::name:
	{//t - const

		auto it = varMap.find(t.varName);
		if (it != varMap.end()){
			string regName = AllocRegister();
			WriteToCode("load", "fp-" + to_string(it->second), regName);
			t.kind = Kind::reg;
			t.varName = regName;
			value = regName;
		}
	}
	break;
	case Kind::reg: //Register
		value = t.varName;
		break;
	default:
		throw invalid_argument("unknown argument");
	}
	return value;
}
Token GetValueFromStack()
{
	Token t = tStack.top();
	tStack.pop();
	return t;
}
void SetArrayToStack(Token* t)
{
	string registerName1 = GetAdrrArrayElement(t);
	string registerName2 = AllocRegister();
	WriteToCode("load", "fp-" + registerName1, registerName2);
	FreeRegister(registerName1);
	Token tVal;
	tVal.kind = Kind::reg;
	tVal.varName = registerName2;
	tStack.push(tVal);
}
void SetValueToStack(Token *t)
{
	if (!t)return;
	tStack.push(*t);
}
void SetValueToStack(string registerName)
{
	Token t;
	t.kind = Kind::reg;
	t.varName = registerName;
	SetValueToStack(&t);
}
string NumberToRegister(Token& t)
{
	string registerName = AllocRegister();
	t.kind = Kind::reg;
	t.varName = registerName;
	WriteToCode("add", "g0", to_string(t.val), registerName);
	return registerName;
}

void SetVariableToLValueStack(Token *t)
{
	lStack.push(*t);
}
//Return register name, reg=address of array element. Array name in t, array index in stack
string GetAdrrArrayElement(Token* t)
{
	auto arrName = t->varName;
	auto offset = arrMap[arrName];
	Token tIndex = GetValueFromStack();
	string registerName1, registerName2;
	ChangeConstTokenToNumber(tIndex);
	switch (tIndex.kind)
	{
	case Kind::number:
		offset -= tIndex.val * 4;
		registerName1 = AllocRegister();
		WriteToCode("sub", to_string(offset), "g0", registerName1);
		break;
	case Kind::name:
		registerName2 = AllocRegister();
		registerName1 = GetValueFromToken(tIndex);
		WriteToCode("lshift", registerName1, "2", registerName2);
		WriteToCode("sub", to_string(offset), registerName2, registerName1);
		FreeRegister(registerName2);
		break;
	case Kind::reg:
		registerName2 = AllocRegister();
		registerName1 = tIndex.varName;
		WriteToCode("lshift", registerName1, "2", registerName2);
		WriteToCode("sub", to_string(offset), registerName2, registerName1);
		FreeRegister(registerName2);
		break;
	default:
		throw invalid_argument("Unknown kind in GetAdrrArrayElement");
		break;
	}

	return registerName1;
}
void SetArrayToLvalueStack(Token *t)
{
	Token tArr;
	tArr.kind = Kind::reg;
	tArr.varName = GetAdrrArrayElement(t);
	lStack.push(tArr);
}
//lStack variables  = tStack values
void AssignVariable()
{
	while (!lStack.empty()){
		Token tVar = lStack.top(); lStack.pop();
		string sOffset;
		switch (tVar.kind)
		{
		case Kind::name:
		{auto it = varMap.find(tVar.varName);
		if (it != varMap.end()){
			sOffset = to_string(it->second);
		}}
		break;
		case Kind::reg:
			sOffset = tVar.varName;
			break;
		default:
			break;
		}
		Token tValue = GetValueFromStack();
		switch (tValue.kind)
		{
		case Kind::number:
			NumberToRegister(tValue); WriteToCode("store", tValue.varName, "fp-" + sOffset);
			FreeRegister(tValue.varName); break;
		case Kind::reg:
			WriteToCode("store", tValue.varName, "fp-" + sOffset);
			FreeRegister(tValue.varName); break;
		case Kind::name:
		{
			string r1 = GetValueFromToken(tValue);
			WriteToCode("store", r1, "fp-" + sOffset);
			FreeRegister(tValue.varName);
			FreeRegister(r1);
		}
		break;
		default:
			throw invalid_argument("Unknown kind in AssignVariable");
			break;


		}
	}
}
void DeclareVariable(string varName, int sz)
{
	if (sz == 0){
		fp += 4;
		varMap[varName] = fp;
	}
	else
	{
		fp += 4 * sz;
		arrMap[varName] = fp;
	}
}
void PlusMinus(Token* t)
{
	if (!t)return;
	Token ty = GetValueFromStack();
	Token tx = GetValueFromStack();

	string x = GetValueFromToken(tx);;
	if (tx.kind == Kind::number && ty.kind == Kind::number){
		x = NumberToRegister(tx);
	}
	string y = GetValueFromToken(ty);
	string z = AllocRegister();
	WriteToCode(GetFunc(t, x, y, z));
	SetValueToStack(z);
	if (tx.kind == Kind::reg)FreeRegister(tx.varName);
	if (ty.kind == Kind::reg)FreeRegister(ty.varName);

}
void WriteChar_K()
{
	Token t = GetValueFromStack();
	string sval;
	ChangeConstTokenToNumber(t);
	if (t.kind == Kind::number)sval = NumberToRegister(t);
	sval = GetValueFromToken(t);
	WriteToCode("outc", sval);
	FreeRegister(sval);
}
void Write_K()
{
	Token t = GetValueFromStack();
	string sval;
	ChangeConstTokenToNumber(t);
	if (t.kind == Kind::number)sval = NumberToRegister(t);
	sval = GetValueFromToken(t);
	WriteToCode("out", sval);
	FreeRegister(sval);
}
void ReadChar_k()
{
	string reg1 = AllocRegister();
	WriteToCode("inc", reg1);
	SetValueToStack(reg1);
	AssignVariable();
}
void Read_k()
{
	string reg1 = AllocRegister();
	WriteToCode("in", reg1);
	SetValueToStack(reg1);
	Token t;
	t.kind = Kind::number;
	t.val = 1;
	SetValueToStack(&t);
	AssignVariable();
}
//*************************
//Bool Expressions
int labelCount = 1;
stack<string> labelsTrue;
stack<string> labelsFalse;
string GenerateLabel()
{
	return "L" + to_string(labelCount++);
}
void GenerateLabelTrue()
{
	labelsTrue.push(GenerateLabel());
}
void DeleteLabelTrue(){
	labelsTrue.pop();
}
void DeleteLabelFalse()
{
	labelsFalse.pop();
}
void GenerateLabelFalse()
{
	labelsFalse.push(GenerateLabel());
}
void SetLabelTrue()
{
	WriteToCode("ba", labelsFalse.top());
	WriteToCode(labelsTrue.top() + ":");
	labelsTrue.pop();
}
void SetLabelFalse()
{
	WriteToCode(labelsFalse.top() + ":");
	labelsFalse.pop();
}
void GetBoolFunc(Token* t, const string& label)
{
	auto it = sams.find(t->kind);
	string sfunc;
	if (it != sams.end()){
		WriteToCode(it->second, label);
	}else
	if (t->kind == Kind::not_equal){
		string lbl=GenerateLabel();
		WriteToCode("be", lbl);
		WriteToCode("ba", label);
		WriteToCode(lbl + ":");
	}else
	throw invalid_argument("Can't find asm function");
}
void Condition(Token* t)
{
	if (!t)return;
	Token ty = GetValueFromStack();
	Token tx = GetValueFromStack();

	string x = GetValueFromToken(tx);
	if (tx.kind == Kind::number && ty.kind == Kind::number){
		x = NumberToRegister(tx);
	}
	string y = GetValueFromToken(ty);
	WriteToCode("cmp", x, y);
	tStack.push(*t);

	if (tx.kind == Kind::reg)FreeRegister(tx.varName);
	if (ty.kind == Kind::reg)FreeRegister(ty.varName);
}
void And_k()
{
	Token boolT = GetValueFromStack();
	if (boolT.kind == Kind::boolexpr_b){
		string lbl= GenerateLabel();
		WriteToCode("ba", lbl );
		SetLabelFalse();
		WriteToCode("ba", labelsFalse.top());
		WriteToCode(lbl + ":");
	}
	else{
		string labelTrue = GenerateLabel();
		string labelFalse = labelsFalse.top();
		GetBoolFunc(&boolT, labelTrue);
		WriteToCode("ba", labelFalse);
		WriteToCode(labelTrue + ":");
	}

}
void Or_k(){
	Token boolT = GetValueFromStack();
	if (boolT.kind == Kind::boolexpr_b){
		WriteToCode("ba", labelsTrue.top());
		SetLabelFalse();
	}
	else{
		string labelTrue;
		labelTrue = labelsTrue.top();
		GetBoolFunc(&boolT, labelTrue);
	}
}
void Not_k(){
	Token boolT = GetValueFromStack();
	Token tExpr;
	tExpr.varName = "not "+boolT.varName;
	tExpr.kind = Kind::boolexpr_b;
	if (boolT.kind == Kind::boolexpr_b){
		string lbl = GenerateLabel();
		WriteToCode("ba", lbl);
		SetLabelFalse();
		labelsFalse.push(lbl);
		SetValueToStack(&tExpr);
	}
	else{
		string  lbl= GenerateLabel();
		GetBoolFunc(&boolT, lbl);
		labelsFalse.push(lbl);
		SetValueToStack(&tExpr);
	}
}
void Odd_k(){
	Token t2;
	t2.kind = Kind::number;
	t2.val = 2;
	SetValueToStack(&t2);
	Token tReminder;
	tReminder.kind = Kind::remainder_k;
	PlusMinus(&tReminder);
	SetValueToStack("g0");
	Token tEq;
	tEq.kind = Kind::equal;
	Condition(&tEq);
}
void Then_K(bool isAnd){
	Token boolT = GetValueFromStack();
	if (boolT.kind == Kind::boolexpr_b)
	{ if (isAnd){
		string lbl = GenerateLabel();
		WriteToCode("ba", lbl);
		SetLabelFalse();
		WriteToCode("ba", labelsFalse.top());
		WriteToCode(lbl + ":");
	}
		labelsTrue.pop();
		return;
	}
	if (!isAnd)GenerateLabelFalse();
	string labelTrue = labelsTrue.top();
	string labelFalse = labelsFalse.top();
	GetBoolFunc(&boolT, labelTrue);
	SetLabelTrue();
}

void Else_k()
{
	
	string lbl = GenerateLabel();
	WriteToCode("ba", lbl);
	SetLabelFalse();
	labelsFalse.push(lbl);
}
//**
stack<string> labelsWhile;
void While_k()
{
	string lbl = GenerateLabel();
	WriteToCode(lbl + ":");
	labelsWhile.push(lbl);
}
void While_k_end()
{
	string lbl = labelsWhile.top();
	labelsWhile.pop();
	WriteToCode("ba", lbl);
	SetLabelFalse();
}
stack<Token> identifierFor;
void ForAssign_k(Token* t){

}
void For_k(Token* t)
{
	identifierFor.push(*t);
	Token tTo = GetValueFromStack();
	SetValueToStack(t);
	SetValueToStack(&tTo);
	Token tLess;
	tLess.kind = Kind::less_or_equal;
	Condition(&tLess);
	GenerateLabelTrue();
	Then_K();
}
void For_k_end()
{
	Token tPlus;
	Token t1;
	t1.kind = Kind::number;
	t1.val = 1;
	tPlus.kind = Kind::add;
	Token tI = identifierFor.top();
	identifierFor.pop();
	SetVariableToLValueStack(&tI);
	SetValueToStack(&tI);
	SetValueToStack(&t1);
	PlusMinus(&tPlus);
	AssignVariable();
	While_k_end();
}