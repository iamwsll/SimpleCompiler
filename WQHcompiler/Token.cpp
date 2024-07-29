#define _CRT_SECURE_NO_WARNINGS 1
#include"VM.h"
#include<iostream>
#include"Parser.h"
#include"Token.h"
/*
���ʷ�������tokenize/lex������Դ���ȡ����һ��token���õ����token�����ͺ�ֵ��
*/
void TokensClass::next()
{
    long long op;
    char* last_pos;
    long long hash;
    while (token = *src++)
    {
        if (token == '\n')
        {
            // ����ģʽ�£���������˴���Ļ������д�ӡ���м����ɵĴ���
            if (_testVM.debug && (_testVM.last_code <= _testVM.code))
            {
				fprintf(compiler_file, "line %lld:\n", line);
                while (_testVM.last_code <= _testVM.code)
                {
                    op = *(_testVM.last_code);
					fprintf(compiler_file, "0x%.10llX: %.4s", (long long)(_testVM.last_code)++, &"LEA ,IMM ,JMP ,JSR ,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
						"OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
						"OPEN,READ,CLOS,WRIT,PRTF,MALC,FREE,MSET,MCMP,MCPY,EXIT,"[(op - LEA) * 5]);
                    if (op >= JMP && op <= JNZ)
					fprintf(compiler_file, "0x%.10llX\n", *_testVM.last_code++);
                    else if (op <= ADJ)
                    fprintf(compiler_file, "%lld\n", *_testVM.last_code++);
                    else
                    fprintf(compiler_file, "\n");
                }
                _testVM.last_code = _testVM.code + 1;
            }
            line++;
        }
        // ��֧�ֺ��Ԥ����ָ�����#ֱ����Ϊ��ע��
        else if (token == '#')
        {
            while (*src != '\0' && *src != '\n')
            {
                src++;
            }
        }
        // ��ʶ��
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || token == '_')
        {
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || *src == '_' || (*src >= '0' && *src <= '9'))
            {
                hash = hash * 147 + *src; // �����ַ����Ĺ�ϣֵ������Ψһ��ʶһ����ʶ�����ٶ����ᷢ����ϣ��ͻ
                src++;
            }
            // �������б�ʶ��
            _parserptr->current_id = _parserptr->symbols;
            while (_parserptr->current_id->token)
            {
                if (_parserptr->current_id->hash == hash && !memcmp(_parserptr->current_id->name, last_pos, src - last_pos))
                {
                    // �ҵ���Ϊ���еı�ʶ��
                    token = _parserptr->current_id->token;
                    return;
                }
                _parserptr->current_id++;
            }
            // �ڷ��ű��б����µı�ʶ��
            token = _parserptr->current_id->token = Id;
            _parserptr->current_id->hash = hash;
            _parserptr->current_id->name = last_pos;
            return;
        }
        // ��������ֵ���������ͣ�0123�˽��ƣ�123ʮ���ƣ�0x123ʮ������
        else if (token >= '0' && token <= '9')
        {
            token_val = token - '0';
            // ʮ����
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
                // ʮ������
                if (*src == 'x' || *src == 'X')
                {
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || token >= 'A' && token <= 'F')
                    {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0); // '0'~48,'A'~65,'a'~97
                        token = *++src;
                    }
                }
                // �˽���
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
        // �ַ������ַ�����������֧��\nת�壬����ת����ʱ������֧�֣����������洢��data��
        else if (token == '"' || token == '\'')
        {
            last_pos = _testVM.data;
            while (*src != 0 && *src != token)
            {
                token_val = *src++;
                // ����ת���ַ�\n����ƥ��\n�ķ�б�ܺ��ԣ�ֱ�ӱ�ʾ����һ���ַ�
                if (token_val == '\\')
                {
                    token_val = *src++;
                    if (token_val == 'n')
                    {
                        token_val = '\n';
                    }
                }

                if (token == '"')
                {
                    *_testVM.data++ = token_val; // �ַ���ĩβ��\0����������
                }
            }

            src++;
            if (token == '"')
            {
                token_val = (long long)last_pos;
            }
            else
            {
                token = Num; // �ַ�������Ϊ����
            }
            return;
        }
        // ע�ͺͳ���, // /**/ /
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
                    if (*src == '\n') // ע���еĻ���
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
        // ������
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
    ,compiler_file(fopen("compile_record.txt", "a"))
{}
/*
�����һ��token�Ƿ���ĳ���ض����ͣ���ƥ�䱨���˳���
*/
void TokensClass::match(long long tk)
{
    char* tokens;
    if (token == tk)
    {
        next();
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
            LOG(ERROR, "%d: expected token : %.8s\n", line, &tokens[8 * (tk - Num)]);
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