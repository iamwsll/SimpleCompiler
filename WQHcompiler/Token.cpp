#define _CRT_SECURE_NO_WARNINGS 1
#include"VM.h"
#include<iostream>
#include"Parser.h"
#include"Token.h"
/*
做词法分析（tokenize/lex）：从源码获取到下一个token，得到这个token的类型和值。
*/
void TokensClass::getToken()
{
    char* last_pos;
    while (token = *src++)//注意这里src在token前面一位
    {
        if (token == '\n')
        {
            // 调试模式下，如果生成了代码的话，按行打印出中间生成的代码
            if (_testVM.debug && (_testVM.last_code <= _testVM.code))
            {
				fprintf(compiler_file, "line %lld:\n", line);
                while (_testVM.last_code <= _testVM.code)
                {
                    long long op = *(_testVM.last_code);
					fprintf(compiler_file, "     0x%.10llX: %.4s", (long long)(_testVM.last_code)++, &"LEA ,IMM ,JMP ,JSR ,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
						"OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"//这个*5是注意到每个单词（包括逗号）刚好5个字符
						"OPEN,READ,CLOS,WRIT,PRTF,SCAN,MALC,FREE,MSET,MCMP,MCPY,EXIT,"[(op - LEA) * 5]);
                    if (op >= JMP && op <= JNZ)
					fprintf(compiler_file, "          0x%.10llX\n", *_testVM.last_code++);//这里的数据是地址
                    else if (op <= ADJ)
                    fprintf(compiler_file, "          %lld\n", *_testVM.last_code++);//这里的数据就是一个数
                    else
                    fprintf(compiler_file, "\n");
                }
                _testVM.last_code = _testVM.code + 1;
            }
            line++;
        }
        // 不支持宏和预编译指令，遇到#直接视为行注释
        else if (token == '#')
        {
            while (*src != '\0' && *src != '\n')
            {
                src++;
            }
        }
		// 标识符：关键字也算
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || token == '_')
        {

            last_pos = src - 1;
            long long hash = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || *src == '_' || (*src >= '0' && *src <= '9'))
            {
                hash = hash * 147 + *src; // 计算字符串的哈希值，用来唯一标识一个标识符，假定不会发生哈希冲突
                src++;
            }
            // 查找已有标识符
            _parserptr->current_id = _parserptr->symbols;
            while (_parserptr->current_id&&_parserptr->current_id->token)
            {
                if (_parserptr->current_id->hash == hash && !memcmp(_parserptr->current_id->name, last_pos, src - last_pos))
                {
                    // 找到了为已有的标识符
                    token = _parserptr->current_id->token;
                    return;
                }
                _parserptr->current_id++;//正是在这里，current_id始终保持了最新的状态。因此每调用一次next，都会更新它
            }
            // 在符号表中保存新的标识符
            token = _parserptr->current_id->token = Id;
            _parserptr->current_id->hash = hash;
            _parserptr->current_id->name = last_pos;
            return;
        }
        // 整数字面值，三种类型：0123八进制，123十进制，0x123十六进制
        else if (token >= '0' && token <= '9')
        {
            token_val = token - '0';
            // 十进制
            if (token_val > 0)
            {
                while (*src >= '0' && *src <= '9')
                {
                    token_val = token_val * 10 + (*src - '0');
                    src++;
                }
            }
            else
            {
                // 十六进制
                if (*src == 'x' || *src == 'X')
                {
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || token >= 'A' && token <= 'F')
                    {
                        // token & 15 计算字符对应的数值（0-9对应自身, A-F对应10-15）（奇妙的思路，from chatgpt）
                        // 如果是大写字母或小写字母，再加上偏移值9
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0); // '0'~48,'A'~65,'a'~97
                        token = *++src;
                    }
                }
                // 八进制
                else
                {
                    while (*src >= '0' && *src <= '7')
                    {
                        token_val = token_val * 8 + *src - '0';
                        src++;
                    }
                }
            }
            token = Num;
            return;
        }
        // 字符串和字符字面量，仅支持\n转义，其他转义暂时不进行支持(如果加了其他的，那么就相当于没加\)，字面量将存储到data区
        else if (token == '"' || token == '\'')//两种引号分别表示字符和字符串
        {
            last_pos = _testVM.data;
            while (*src != 0 && *src != token)
            {
				token_val = *src++;//token_val是字符的ascii码
                // 处理转义字符\n，不匹配\n的反斜杠忽略，直接表示其下一个字符
                if (token_val == '\\')
                {
                    token_val = *src++;
                    if (token_val == 'n')
                    {
                        token_val = '\n';
                    }
                }

                if (token == '"')
                {//这里会不断的往data里加字符
                    *(long long*)_testVM.data++ = token_val; // 字符串末尾补\0不在这里做 
                    //为了方便，我们直接存8字节的，这样方便处理
                }
            }

            src++;
            if (token == '"')
            {
				token_val = (long long)last_pos;//也就是字符串的起始地址，之前while循环里token_val只是一个辅助
            }
            else
            {
				token = Num; // 字符常量视为整数  这时候while循环里的token才有意义，也就是字符的ascii码
            }
            return;
        }
        // 注释和除号, // /**/ /
        else if (token == '/')
        {
            //
            if (*src == '/')
            {
                while (*src != 0 && *src != '\n')
                {
                    src++;
                }
            }
            /* */
            else if (*src == '*')
            {
                src++;
                while (*src != 0 && (*src != '*' || *(src + 1) != '/'))
                {
                    if (*src == '\n') // 注释中的换行
                        line++;
                    src++;
                }
                if (*src == '*' && *(src + 1) == '/')
                {
                    src = src + 2;
                }
            }
            else
            {
                token = Div;
                return;
            }
        }
        // 操作符
        else if (token == ',') { token = Comma; return; }
        else if (token == '=') { if (*src == '=') { src++; token = Eq; } else token = Assign; return; } // = ==
        else if (token == '+') { if (*src == '+') { src++; token = Inc; } else token = Add; return; } // + ++
        else if (token == '-') { if (*src == '-') { src++; token = Dec; } else if (*src == '>') { src++, token = Gmbp; } else token = Sub; return; } // - -- ->
        else if (token == '!') { if (*src == '=') { src++; token = Ne; } return; } // != !
        else if (token == '<') { if (*src == '=') { src++; token = Le; } else if (*src == '<') { src++; token = Shl; } else token = Lt; return; } // < <= <<
        else if (token == '>') { if (*src == '=') { src++; token = Ge; } else if (*src == '>') { src++; token = Shr; } else token = Gt; return; } // > >= >>
        else if (token == '&') { if (*src == '&') { src++; token = Land; } else token = And; return; } // & &&
        else if (token == '|') { if (*src == '|') { src++; token = Lor; } else token = Or; return; } // | ||
        else if (token == '^') { token = Xor; return; } // ^
        else if (token == '%') { token = Mod; return; } // %
        else if (token == '*') { token = Mul; return; } // *
        else if (token == '?') { token = Cond; return; } // ?
        else if (token == '[') { token = Brak; return; } // [
        else if (token == '.') { token = Dot; return; } // .
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ':') return; // tokens are their ASCII
    }
}
TokensClass::TokensClass(VM& testVM, Parser* parserptr)
    :_testVM(testVM)
    , _parserptr(parserptr)
    ,compiler_file(fopen("compile_record.txt", "w"))
{}
/*
检查下一个token是否是某种特定类型，不匹配报错退出。
*/
void TokensClass::match(long long tk)
{
    char* tokens;
    if (token == tk)
    {
        getToken();
    }
    else
    {
        tokens =(char*)
            "Num     "
            "Id      "
            "Break   "
            "Char    "
            "Continue"
            "Do      "
            "Else    "
            "Enum    "
            "For     "
            "Goto    "
            "If      "
            "Int     "
            "Return  "
            "Sizeof  "
            "Struct  "
            "Union   "
            "While   "
            "Comma   "
            "Assign  "
            "Cond    "
            "Lor     "
            "Land    "
            "Or      "
            "Xor     "
            "And     "
            "Eq      "
            "Ne      "
            "Lt      "
            "Gt      "
            "Le      "
            "Ge      "
            "Shl     "
            "Shr     "
            "Add     "
            "Sub     "
            "Mul     "
            "Div     "
            "Mod     "
            "Inc     "
            "Dec     "
            "Brak    "
            "Dot     "
            "Gmbp    ";
        if (tk >= Num)
        {
            LOG(ERROR, "%d: expected token : %.8s\n", line, &tokens[8 * (tk - Num)]);//这个8跟之前的5一样的，都是巧妙地实现了对齐
        }
        else
        {
            LOG(ERROR, "%d: expected token : '%c'\n", line, tk);
        }
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
}
TokensClass::~TokensClass()
{
    fprintf(compiler_file, "\n");
    fclose(compiler_file);
}