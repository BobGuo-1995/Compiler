#include "Scanner.h"
//message for display 
unordered_map<Kind, string> mLexMessage = { { Kind::nothing, "nothing" }, { Kind::period, "period" }, { Kind::comma, "comma" }, { Kind::semicolon, "semicolon" },
{ Kind::add, "add" }, { Kind::minus, "minus" }, { Kind::multiply, "multiply" }, { Kind::divide, "divide" },
{ Kind::remainder_k, "remainder" }, { Kind::left_parenthesis, "left parenthesis" }, { Kind::right_parenthesis, "right parenthesis" },
{ Kind::left_bracket, "left bracket" }, { Kind::right_bracket, "right bracket" }, { Kind::assignment, "assignment" },
{ Kind::less, "less" }, { Kind::less_or_equal, "less or equal" }, { Kind::greater, "greater" }, { Kind::greater_or_equal, "greater or equal" }, { Kind::equal, "equal" }, { Kind::not_equal, "not equal" }, { Kind::odd, "odd" }, { Kind::constants, "constants" }, { Kind::variables, "variables" }, { Kind::begin_k, "begin" }, { Kind::end_k, "end" },
{ Kind::if_k, "if" }, { Kind::then_k, "then" }, { Kind::else_k, "else" }, { Kind::while_k, "while" },
{ Kind::do_k, "do" }, { Kind::for_k, "for" }, { Kind::to_k, "to" }, { Kind::and_k, "and" },
{ Kind::or_k, "or" }, { Kind::not_k, "not" }, { Kind::read_k, "read" }, { Kind::write_k, "write" },
{ Kind::readchar_k, "readchar" }, { Kind::writechar_k, "writechar" } };

//map keywords to Kind
unordered_map<string, Kind > mLexemKind = { { "const", Kind::constants, }, { "var", Kind::variables }, { "begin", Kind::begin_k },
{ "end", Kind::end_k }, { "if", Kind::if_k }, { "then", Kind::then_k }, { "odd", Kind::odd }, { "else", Kind::else_k },
{ "while", Kind::while_k }, { "do", Kind::do_k }, { "for", Kind::for_k }, { "to", Kind::to_k }, { "and", Kind::and_k },
{ "or", Kind::or_k }, { "not", Kind::not_k }, { "read", Kind::read_k }, { "write", Kind::write_k }, { "readchar", Kind::readchar_k },
{ "writechar", Kind::writechar_k } };

string GetLexemName(Kind kind)
{
	auto it = mLexMessage.find(kind);
	if (it != mLexMessage.end())return it->second;
	return "";
}

class CStream{
	string fname;
	string txt;
	size_t pos = 0;
	int lineNumber = 1;
	int columnNumber = 1;
	int fixLineNumber = 1;
	int fixColumnNumber = 1;
public:
	CStream(string fileName){
		fname = fileName;
		ifstream ifs(fileName);
		istreambuf_iterator<char> itStart(ifs);
		istreambuf_iterator<char> itEnd;
		txt.assign(itStart, itEnd);
	}
	int GetLine(){ return lineNumber; }
	int GetColumn(){ return columnNumber; }
	int GetFixLine(){ return fixLineNumber; }
	int GetFixColumn(){ return fixColumnNumber; }
	char CurrentSymbol(){
		if (pos < txt.length())
			return txt[pos];
		return 0;
	}
	char PeekSymbol(){
		if (pos + 1 < txt.length())
		{
			return txt[pos + 1];
		}
		return 0;
	}
	char NextSymbol(){
		if (pos < txt.length())
		{
			char c = txt[++pos]; ++columnNumber;
			if (c == '\n') { ++lineNumber; columnNumber = 0; }
			return c;
		}
		return 0;
	}
	//Fix position for display correct line and column
	void FixPos(){
		fixLineNumber = lineNumber;
		fixColumnNumber = columnNumber;
	}

};
void PrintError(string msg,bool flagUseFixPosition=true);
CStream *pStream = nullptr;
string delimeters = " \n\r\t+=-><*\\/[]().:;,%";
bool IsDelimiter(char c)
{
	if (delimeters.find(c) != string::npos) return true;
	return false;
}
char SkipSpace(){
	char c;
	while (c = pStream->NextSymbol())
	{
		if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n'))
		{
			return c;
		}
	}
	return 0;
}
char SkipComment()
{
	pStream->NextSymbol();
	char c;
	while (c = pStream->NextSymbol())
	{
		if (c == '*' && pStream->PeekSymbol() == '/')
		{
			pStream->NextSymbol();
			c = pStream->NextSymbol();
			return c;
		}
	}
	return 0;
}
int GetNumber()
{
	string s;
	s += pStream->CurrentSymbol();
	char c;
	if (pStream->CurrentSymbol() == '0'&&pStream->PeekSymbol() == 'x'){
		s += pStream->NextSymbol();
		while (c = pStream->NextSymbol())
		{
			if (!IsDelimiter(c)){
				s += c;
			}
			else
			{
				try{
					size_t pos=0;
					int number = stoi(s, &pos, 16);
					if (pos != s.size())
						throw invalid_argument("Malformed number!");
					return number;
				}
				catch (exception e){
					PrintError("Bad hex number.");
					throw;
				}
			}
		}
	}
	else
		while (c = pStream->NextSymbol())
		{
			if (!IsDelimiter(c)){
				s += c;
			}
			else
			{
				try{
					size_t pos=0;
					int number = stoi(s, &pos);
					if (pos != s.size()) 
						throw invalid_argument("Malformed number!");
					return number; 
				}
				catch (exception e){
					PrintError("Bad number.");
					throw;
				}
				
			}
		}
	throw invalid_argument("Malformed number!");
}
string GetKeyName()
{
	string s;
	s += pStream->CurrentSymbol();
	char c;
	while (c = pStream->NextSymbol())
	{
		if (!IsDelimiter(c)){
			if (isalnum(c) || c == '_')
			{
				s += c;
			}
			else
			{
				PrintError("Illegal character ASC(" + to_string((unsigned char)c) + ") deleting!",false);
			}
		}
		else
			break;
	}
	return s;
}
void SetPosToken(Token& t)
{
	t.line = pStream->GetLine();
	t.column = pStream->GetColumn();
}
void PrintError(string msg, bool flagUseFixPosition)
{
	int line, column;
	line = flagUseFixPosition ?pStream->GetFixLine(): pStream->GetLine();
	column = flagUseFixPosition ? pStream->GetFixColumn() : pStream->GetColumn();
	msg += " line " + to_string(line) + ", column " + to_string(column);
	cout << msg << endl;
}
void CreateTokensFromFile(const string& fileName, TokenStream& streamToken)
{
	pStream = new CStream(fileName);
	char c;
	try{
		while (c = pStream->CurrentSymbol()){
			pStream->FixPos();
			Token token;SetPosToken(token);
			string lexem, lexemName;
			if (isalpha(c)){
				
				lexem = GetKeyName();
				auto it = mLexemKind.find(lexem);
				if (it == mLexemKind.end()){
					//*** variable ***
					/*The maximum number of characters for an identifier is 63. Identifiers longer than that are considered (and reported) as an error.*/
					if (lexem.length() > 63){
						lexemName = "Error. Identifiers ("+lexem+") longer than 63";
						token.kind = Kind::nothing;
					}
					else{
						token.kind = Kind::name;
						token.varName = lexem;
						lexemName = "identifier(" + lexem + ")";
					}
				}
				else{
					//*** keyword ***
					token.kind = it->second;
					lexemName = GetLexemName(token.kind);
				}
			}
			else
				if (isdigit(c)){
					try{
						int val = GetNumber();
						token.kind = Kind::number;
						token.val = val;
						lexemName = "number(" + to_string(token.val) + ")";
					}
					catch (exception e){
						token.kind = Kind::nothing;
						lexemName = GetLexemName(token.kind);
					}
				}
				else
				{
					switch (c)
					{
					case ':':
						if (pStream->NextSymbol() == '='){
							token.kind = Kind::assignment; pStream->NextSymbol();
						}
						else {
							PrintError("Lexical error:");
							token.kind = Kind::nothing;
						}
						break;
					case '<':{
						char nextSymbol = pStream->NextSymbol();
						if (nextSymbol == '='){
							token.kind = Kind::less_or_equal; pStream->NextSymbol();
						}
						else
							if (nextSymbol == '>'){ token.kind = Kind::not_equal; pStream->NextSymbol(); }
							else token.kind = Kind::less;
							break;
					}

					case '>':if (pStream->NextSymbol() == '='){
						token.kind = Kind::greater_or_equal;
						pStream->NextSymbol();
					}
							 else token.kind = Kind::greater;
							 break;
					case '.':token.kind = Kind::period; pStream->NextSymbol(); break;
					case ',':token.kind = Kind::comma; pStream->NextSymbol(); break;
					case '=':token.kind = Kind::equal; pStream->NextSymbol(); break;
					case ';':token.kind = Kind::semicolon; pStream->NextSymbol(); break;
					case '+':token.kind = Kind::add; pStream->NextSymbol(); break;
					case '-':token.kind = Kind::minus;
						//Is minus => negative number?
						if (isdigit(pStream->NextSymbol()) && streamToken.IsUnaryMinusLastToken())
						{

							try{
								int val = GetNumber();
								token.kind = Kind::number;
								token.val = -val;
								lexemName = "number(" + to_string(token.val) + ")";
							}
							catch (exception e){
								token.kind = Kind::nothing;
								lexemName = GetLexemName(token.kind);
							}
							
							goto endif;
						}
						break;
					case '%':token.kind = Kind::remainder_k; pStream->NextSymbol(); break;
					case '/':
						if (pStream->PeekSymbol() == '*'){ SkipComment(); continue; }
						else
						{
							token.kind = Kind::divide; pStream->NextSymbol(); break;
						}
					case '*':token.kind = Kind::multiply; pStream->NextSymbol(); break;
					case '(':token.kind = Kind::left_parenthesis; 
						pStream->NextSymbol(); break;
					case ')':token.kind = Kind::right_parenthesis; pStream->NextSymbol(); break;
					case '[':token.kind = Kind::left_bracket; pStream->NextSymbol(); break;
					case ']':token.kind = Kind::right_bracket; pStream->NextSymbol(); break;
					case '\r':
					case '\t':
					case '\n':
					case ' ': SkipSpace(); continue; break;
					default:
						PrintError("Illegal character ASC(" + to_string((unsigned char)c) + ")",false);
						pStream->NextSymbol();
						continue;
						break;
					}
					lexemName = GetLexemName(token.kind);
					if (lexemName == ""){ lexemName = "identifier(" + lexem + ")"; }
				}
				endif:
			cout << lexemName << endl;
			streamToken.Add(token);
		}
		Token token; token.kind = Kind::EOF_k;
		streamToken.Add(token);
		cout << "nomore" << endl;
	}
	catch (exception e){
		cout << "Runtime error: " << e.what() << endl;
	}
	if (pStream != nullptr)delete pStream;
}