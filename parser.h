#pragma once
#include <vector>
#include "parseObj.h"
#include "scope.h"
namespace lang
{
class parser
{
public:
    scope* runner;
	std::vector<parseObj*> parsers;
	std::string program;
void parser::function();

public:
    parser(std::string);
public:
    ~parser(void);
};
}
