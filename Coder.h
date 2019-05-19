#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <unordered_map>
#include "Token.h"

using namespace std;


void InitCoder(ofstream* );
string GetCode();
void SetValueToStack(Token* t);
void SetArrayToStack(Token* t);
void PlusMinus(Token* t);
void SetVariableToLValueStack(Token *t);
void SetArrayToLvalueStack(Token *t);
void AssignVariable();
void DeclareVariable(string varName, int sz = 0/*size of array*/);

void WriteChar_K();
void Write_K();
void ReadChar_k();
void Read_k();

void Condition(Token* t);
void GenerateLabelTrue();
void GenerateLabelFalse();
void SetLabelTrue();
void SetLabelFalse();
void DeleteLabelTrue();
void DeleteLabelFalse();
void And_k();
void Or_k();
void Not_k();
void Odd_k();
void Then_K(bool=false);
void Else_k();
void While_k();
void While_k_end();
void For_k(Token* t);
void For_k_end();