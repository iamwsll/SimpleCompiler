#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include"Parser.h"
class Parser;
class StateMent
{
public:
	Parser* _parserptr;
	StateMent(Parser* parserptr);
	void statement();
	void record();
	void backtrack();
	void expression(long long level);
	long long get_unit_size(long long type);
};

	