#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
/*
标识token的类型，已经能够用单个ASCII表示的某些token其实没有包含在里面，如!~()，这也是从128开始的原因。
*/
//这里我们采用枚举的方式来进行
#include"Log.h"
class Parser;
enum Token_type
{
    Num = 128,      // number
    Id,             // identifier
    //关键字，按字典序排列
    Break,          // break
    Char,           // char
    Continue,       // Continue
    Do,             // do
    Else,           // else
    Enum,           // enum
    For,            // For
    Goto,           // goto
    If,             // if
     Int,            // int
     Return,         // return
    Sizeof,         // sizeof
    Struct,         // struct
    Union,          // union
    While,          // while
    // 按优先级排列的运算符
    Comma,          // ,
    Assign,         // =
    Cond,           // ?
    Lor,            // ||
    Land,           // &&
    Or,             // |
    Xor,            // ^
    And,            // &
    Eq, Ne,         // == !=
    Lt, Le, Gt, Ge, // < <= > >=
    Shl, Shr,       // << >>
    Add, Sub,       // + -
    Mul, Div, Mod,  // * / %
    Inc, Dec,       // ++ --
    Brak, Dot, Gmbp // [] . -> 
};
class TokensClass
{
public:
    Parser* _parserptr;
	// tokenizer
	long long token=0;          // 当前token
	long long token_val=0;      // 当前token是常量或字符串字面值时用来记录值
	char* src=0;          // 源码
	long long line=0;           // 行号
	long long last_token=0;     // 支持记录和回溯token流的功能
	long long last_token_val=0; // 支持记录和回溯token流的功能
	char* last_src=0;     // 支持记录和回溯token流的功能
	long long last_line=0;      // 支持记录和回溯token流的功能
    VM _testVM;
    FILE* compiler_file;
public:
    TokensClass(VM& testVM,Parser* parserptr);
    void next();
    void match(long long tk);
    ~TokensClass();
};
