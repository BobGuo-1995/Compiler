#include <ctype.h>
#include <functional>
#include <exception>
#include <iterator>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Token.h"
using namespace std;

void CreateTokensFromFile(const string& fileName, TokenStream& sToken);