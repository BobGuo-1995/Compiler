#pragma once
#include <vector>
#include <string>
#include <memory>
using namespace std;
enum Kind{
	nothing,period, comma, semicolon, add, minus, multiply, divide, remainder_k, left_parenthesis, right_parenthesis,
	left_bracket, right_bracket, assignment, less, less_or_equal, greater, greater_or_equal, equal, not_equal,odd, constants, variables, begin_k, end_k, if_k, then_k, else_k, while_k, do_k, for_k, to_k, and_k, or_k, not_k, read_k, write_k, readchar_k, writechar_k,name, number,Start_K,EOF_k,
	/*For Coder*/
	reg,arr,
	/*For BoolExpr*/
	boolexpr_b
};
struct Token{
	Kind kind= Kind::nothing;
	int val=0;
	string varName;
	//pos in text
	int line=0;
	int column = 1;
};
class TokenStream{
	size_t pos = 0;
	vector<Token> vToken;
public:
	TokenStream(){
		Token t;
		t.kind = Kind::Start_K;
		vToken.push_back(t);
	}
	void Add(const Token& t)
	{
		vToken.push_back(t);
		
	}
	void Reset(){
		pos = 0;
	}
	inline Token* Current()
	{
		return &vToken[pos];
	}
	inline Token* Next(){
		return &vToken.at(++pos);
	}
	//Is current 
	bool IsUnaryMinusLastToken()
	{
		//if (vToken.back().kind != Kind::minus)return false;
		auto sz = vToken.size();
		auto k = vToken[sz - 1].kind;
		if (k == Kind::left_parenthesis || k == Kind::assignment || k == Kind::left_bracket || k == Kind::equal || k == Kind::not_equal || k == Kind::greater || k == Kind::greater_or_equal || k == Kind::less || k == Kind::less_or_equal)
			return true;
		return false;
	}
	void RemoveLast()
	{
		vToken.pop_back();
	}
};

