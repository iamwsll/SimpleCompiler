#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
/*
��ʶtoken�����ͣ��Ѿ��ܹ��õ���ASCII��ʾ��ĳЩtoken��ʵû�а��������棬��!~()����Ҳ�Ǵ�128��ʼ��ԭ��
*/
//�������ǲ���ö�ٵķ�ʽ������
#include"Log.h"
class Parser;
enum Token_type
{
    Num = 128,      // number
    Id,             // identifier
    //�ؼ��֣����ֵ�������
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
    // �����ȼ����е������
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
	long long token=0;          // ��ǰtoken
	long long token_val=0;      // ��ǰtoken�ǳ������ַ�������ֵʱ������¼ֵ
	char* src=0;          // Դ��
	long long line=0;           // �к�
	long long last_token=0;     // ֧�ּ�¼�ͻ���token���Ĺ���
	long long last_token_val=0; // ֧�ּ�¼�ͻ���token���Ĺ���
	char* last_src=0;     // ֧�ּ�¼�ͻ���token���Ĺ���
	long long last_line=0;      // ֧�ּ�¼�ͻ���token���Ĺ���
    VM _testVM;
    FILE* compiler_file;
public:
    TokensClass(VM& testVM,Parser* parserptr);
    void next();
    void match(long long tk);
    ~TokensClass();
};
