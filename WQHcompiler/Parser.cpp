#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include"VM.h"
#include"Parser.h"
#include"Token.h"
Parser::Parser(VM& testVM, long long input_poolsize)
    :poolsize(input_poolsize)
    , TokenOp(testVM, this)
    ,statementobj(new StateMent(this))
{
    // Ϊparser�����ڴ�
    if (!(symbols = (struct Symbol_item*)malloc(poolsize)))
    {
        LOG(FATAL, "Could not malloc(%d) for symbol table\n", poolsize);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(break_list = (struct Bc_list_item*)malloc(sizeof(struct Bc_list_item) * 1024))) // 1024 unit
    {
        LOG(FATAL, "Could not malloc(%d) for break list of parser\n", 8 * 1024);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(continue_list = (struct Bc_list_item*)malloc(sizeof(struct Bc_list_item) * 1024))) // 1024 unit
    {
        LOG(FATAL, "Could not malloc(%d) for continue list of parser\n", sizeof(struct Bc_list_item) * 1024);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(label_list = (struct Label_list_item*)malloc(sizeof(struct Label_list_item) * 1024))) // 1024 unit
    {
        LOG(FATAL, "Could not malloc(%d) for label list for goto of parser\n", sizeof(struct Label_list_item) * 1024);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(func_list = (struct Func_call_item*)malloc(sizeof(struct Func_call_item) * 1024))) // 1024 unit
    {
        LOG(FATAL, "Could not malloc(%d) for function call list of parser\n", sizeof(struct Func_call_item) * 1024);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(struct_symbols_list = (struct us_domain*)malloc(sizeof(struct us_domain) * 1024))) // 1024 unit
    {
        LOG(FATAL, "Could not malloc(%d) for struct symbol list of parser\n", sizeof(struct us_domain) * 1024);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(union_symbols_list = (struct us_domain*)malloc(sizeof(struct us_domain) * 1024))) // 1024 unit
    {
        LOG(FATAL, "Could not malloc(%d) for union symbol list of parser\n", sizeof(struct us_domain) * 1024);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(us_domains_list = (struct us_domain*)malloc(sizeof(struct us_domain) * 4096))) // 4096 unit
    {
        LOG(FATAL, "Could not malloc(%d) for union and struct domains symbol info list of parser\n", sizeof(struct us_domain) * 4096);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    memset(symbols, 0, poolsize);
    memset(break_list, 0, sizeof(struct Bc_list_item) * 1024);
    memset(continue_list, 0, sizeof(struct Bc_list_item) * 1024);
    memset(label_list, 0, sizeof(struct Label_list_item) * 1024);
    memset(func_list, 0, sizeof(struct Func_call_item) * 1024);
    memset(struct_symbols_list, 0, sizeof(struct us_domain) * 1024);
    memset(union_symbols_list, 0, sizeof(struct us_domain) * 1024);
    memset(us_domains_list, 0, sizeof(struct us_domain) * 4096);

    init_symbols();
}
Parser::~Parser()
{
    free(symbols);
	free(break_list);
	free(continue_list);
	free(label_list);
	free(func_list);
	free(struct_symbols_list);
	free(union_symbols_list);
	free(us_domains_list);
	delete statementobj;
}
void Parser::init_symbols()
{
    long long tmp;
    TokenOp.src = (char*)"break char continue do else enum for goto if int return sizeof struct union while "
        "open read close write printf malloc free memset memcmp memcpy exit void main";

    // ���ؼ�����ǰ��ӵ����ű��ڴʷ�����ʱ�ؼ����߱�ʶ����ʶ�����̣������Ѿ��ڷ��ű��У�����ֱ�ӷ��ط��ű�Ľ��
    tmp = Break;
    while (tmp <= While)
    {
        TokenOp.next();
        current_id->token = tmp++; // only need token
    }

    // ���⺯����ӵ����ű��У��͹ؼ��ֺ�������
    tmp = OPEN;
    while (tmp <= EXIT)
    {
        TokenOp.next();
        current_id->IdClass = Sys;   // ��ʶ��������ϵͳ����
        current_id->type = INT;    // ����ֵ����
        current_id->value = tmp++; // ָ��
    }
    // void������Ϊchar����main����Ϊ��ʶ����ӵ����ű���ʹ��idmain��¼main�����ķ��ű���
    TokenOp.next();
    current_id->token = Char; // void type, regard void as char
    TokenOp.next();
    idmain = current_id; // keep track on main

}
void Parser::read_src(char** argv)
{
    FILE* fp;
    char* open_filename;
	if (TokenOp._testVM.file_flag == 1)
    {
		open_filename = (char*)"test.txt";
	}
    else
    {
        open_filename = *(argv + (TokenOp._testVM.debug == 1 ? 2 : 1));
	}
    
    if ((fp = fopen(open_filename, "r")) == nullptr)
    {
        std::cout<<"Could not open(%s)"<<*argv<<std::endl;
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
	LOG(DEBUG, "open file %s\n", *argv);
    if (!(TokenOp.src = (char*)malloc(poolsize)))
    {
        LOG(FATAL, "Could not malloc(%d) for source code area\n", poolsize);
        fclose(fp);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }

    memset(TokenOp.src, 0, poolsize);
    long long tmp;
    if ((tmp = fread(TokenOp.src, 1, poolsize, fp)) <= 0)
    {
        LOG(FATAL, "fread() returned %zu\n", tmp);
        free(TokenOp.src);
        fclose(fp);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }

    fclose(fp);
    TokenOp.src[tmp] = 0; // add \0 to identify end of source code

}
/*
parser���
֧�ֵ�C�����Ӽ���EBNF�ķ���:

program = {global_decl};
global_decl = enum_decl | func_decl | var_decl | struct_decl | union_decl | forward_type_decl | forward_func_decl;
enum_decl = enum, [id], "{", id, ["=", number], {",", id, ["=", number]} ,"}";
func_decl = ret_type, id, "(", [param_decl], ")", "{", func_body, "}";
struct_decl = struct, [id], "{", var_decl, {var_decl}, "}", ";";
union_decl = union, [id], "{", var_decl, {var_decl}, "}", ";";
forward_type_decl = (union | struct), id;
forward_func_decl = ret_type, id, "(", [param_decl], ")", ";";
param_decl = type, {"*"}, id, {",", type {"*"}, id};
ret_type = void | type, {"*"};
type = long long | char | user_defined_type;
user_defined_type = (enum | union | struct), id;
func_body = {var_decl}, {statement};
var_decl = type {"*"}, id, {",", id}, ";";
statement = if_statement
        | while_statement
        | for_statement
        | do_while_statement
        | break_statement
        | continue_statement
        | "{", {statement}, "}"
        | return, [expression], ";"
        | [expression], ";";
        | id, ":", statement;
        | goto, id, ";";
if_statement = if, "(", expression, ")", statement, [else, statement];
while_statement = while, "(", expression, ")", statement;
for_statement = for, "(", [expression], ";", [expression], ";", [expression], ")", statement;
do_while_statement = do, statement, while, "(", [expression], ")", ";";
break_statement = break, ";";
continue_statement = continue, ";";
*/
void Parser::parse()
{
    struct Func_call_item* func_list_pos;
    long long find_func;

    cur_struct_type = STRUCT;
    cur_union_type = UNION;

    TokenOp.next();
    while (TokenOp.token > 0)
    {
        global_declaration();
    }

    // ȫ�ֶ��������ɺ����δ����ĺ�������
    for (func_list_pos = func_list; func_list_pos->hash; func_list_pos++)
    {
        find_func = 0;
        for (current_id = symbols; current_id->token; current_id++)
        {
            if (current_id->IdClass == Fun && current_id->hash == func_list_pos->hash && current_id->value)
            {
                *func_list_pos->call_addresss = current_id->value;
                find_func = 1;
                break;
            }
        }
        if (!find_func)
        {
            LOG(ERROR, "%d: call declared but undefined function\n", func_list_pos->line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        func_list_pos->hash = 0;
        func_list_pos->line = 0;
        func_list_pos->call_addresss = 0;
    }
}
/*
����ȫ�ֵı������塢���Ͷ��塢�Լ��������塢���ͺͺ�����ǰ��������
global_decl = enum_decl | func_decl | var_decl | struct_decl | union_decl | forward_type_decl | forward_func_decl;
enum_decl = enum, [id], "{", enum_body ,"}";
func_decl = ret_type, id, "(", [param_decl], ")", "{", func_body, "}";
var_decl = type {"*"}, id, {",", {"*"}, id}, ";";
ret_type = void | type, {"*"};
type = long long | char | user_defined_type;
user_defined_type = (enum | union | struct), id;
struct_decl = struct, [id], "{", var_decl, {var_decl}, "}", ";";
union_decl = union, [id], "{", var_decl, {var_decl}, "}", ";";
forward_type_decl = (union | struct), id;
forward_func_decl = ret_type, id, "(", [param_decl], ")", ";";
*/
void Parser::global_declaration()
{
    long long type;
    struct Symbol_item cur_func; // ��������ʱ���浱ǰ���ű���
    struct Symbol_item* id;

    basetype = INT;

    // ����enum: enum������������������Ϊ����ֵ���庯��
    if (TokenOp.token == Enum)
    {
        TokenOp.match(Enum);
        // �����Ǳ���������������enum���Ͷ���
        if (TokenOp.token == Id)
        {
            id = current_id;
            TokenOp.match(Id);
            // ��ʶ�����id����ֻ�������Ͷ���
            if (id->IdClass == 0)
            {
                id->IdClass = EnumType;
                id->type = Enum;        // ��������岻�����ﲻ����ö�����ͣ�������Ϊint
                TokenOp.match('{');
                enum_body();
                TokenOp.match('}');
            }
            // �Ѷ����enum����
            else if (id->IdClass == EnumType)
            {
                basetype = INT;
                goto define_glo_func;
            }
            // ��֪��ʶ����������enum����
            else
            {
                LOG(ERROR, "%d: known identifier can not be new enum name in definition\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
        }
        // ����enum����
        else
        {
            TokenOp.match('{');
            enum_body();
            TokenOp.match('}');
        }
        TokenOp.match(';');
        return;
    }
    // ����struct���塢ǰ��������struct�������塢struct���������Ϊ������������
    else if (TokenOp.token == Struct)
    {
        TokenOp.match(Struct);
        if (TokenOp.token == Id)
        {
            id = current_id;
            TokenOp.match(Id);

            // ��ʶ�����id����ֻ����ǰ���������߶��壬ȷ������ֵ������Ƕ���ͽ�������
            if (id->IdClass == 0)
            {
                id->IdClass = StructType;
                id->type = cur_struct_type;
                cur_struct_type++;

                // ���ṹ��������Ϣ
                struct_symbols_list[id->type - STRUCT].hash = id->hash;
                struct_symbols_list[id->type - STRUCT].type = id->type;
                struct_symbols_list[id->type - STRUCT].size = 0;    // ������ȷ��
                struct_symbols_list[id->type - STRUCT].offset = 0;
                struct_symbols_list[id->type - STRUCT].next = 0;    // ������ȷ��

                // �µ�struct����
                if (TokenOp.token == '{')
                {
                    TokenOp.match('{');
                    struct_union_body(id->type, 1);
                    TokenOp.match('}');
                }
                // ����ǰ������
                else if (TokenOp.token != ';')
                {
                    LOG(ERROR, "%d: invalid struct definition or forward declaration\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
                // else ;ǰ������
            }
            // ��������struct����
            else if (id->IdClass == StructType)
            {
                // struct����
                if (TokenOp.token == '{')
                {
                    // �Ѿ�������
                    if (struct_symbols_list[id->type - STRUCT].next != 0)
                    {
                        LOG(ERROR, "%d: duplicate struct definition\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // û�ж�����Ǿͽ�������
                    else
                    {
                        TokenOp.match('{');
                        struct_union_body(id->type, 1);
                        TokenOp.match('}'); // ;����ƥ��
                    }
                }
                // ���Ƕ���Ҳ����ǰ����������Ӧ�þ���ȫ�ֱ������ߺ�������
                else if (TokenOp.token != ';')
                {
                    basetype = id->type;
                    goto define_glo_func;
                }
                // else ;����ǰ�������������Ѿ��������ˣ�ʲô����������������û�ж���
            }
            // ��֪��ʶ����������struct����
            else
            {
                LOG(ERROR, "%d: known identifier can not be new struct name in definition\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
        }
        else
        {
            LOG(ERROR, "%d: invalid use of keyword struct, must be with an identifier\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        TokenOp.match(';');
        return;
    }
    // ����union���塢ǰ��������union�������塢union���������Ϊ�����������ͣ���struct����һģһ��
    else if (TokenOp.token == Union)
    {
        TokenOp.match(Union);
        if (TokenOp.token == Id)
        {
            id = current_id;
            TokenOp.match(Id);

            // ��ʶ�����id����ֻ����ǰ���������߶��壬ȷ������ֵ������Ƕ���ͽ�������
            if (id->IdClass == 0)
            {
                id->IdClass = UnionType;
                id->type = cur_union_type;
                cur_union_type++;

                // ���������������Ϣ
                union_symbols_list[id->type - UNION].hash = id->hash;
                union_symbols_list[id->type - UNION].type = id->type;
                union_symbols_list[id->type - UNION].size = 0;    // ������ȷ��
                union_symbols_list[id->type - UNION].offset = 0;
                union_symbols_list[id->type - UNION].next = 0;    // ������ȷ��

                // �µ�union����
                if (TokenOp.token == '{')
                {
                    TokenOp.match('{');
                    struct_union_body(id->type, 0);
                    TokenOp.match('}');
                }
                // ����ǰ������
                else if (TokenOp.token != ';')
                {
                    LOG(ERROR, "%d: invalid struct definition or forward declaration\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
                // else ;ǰ������
            }
            // ��������union����
            else if (id->IdClass == UnionType)
            {
                // union����
                if (TokenOp.token == '{')
                {
                    // �Ѿ�������
                    if (union_symbols_list[id->type - UNION].next != 0)
                    {
                        LOG(ERROR, "%d: duplicate union definition\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // û�ж�����Ǿͽ�������
                    else
                    {
                        TokenOp.match('{');
                        struct_union_body(id->type, 0);
                        TokenOp.match('}'); // ;����ƥ��
                    }
                }
                // ���Ƕ���Ҳ����ǰ����������Ӧ�þ���ȫ�ֱ������ߺ�������
                else if (TokenOp.token != ';')
                {
                    basetype = id->type;
                    goto define_glo_func;
                }
                // else ;����ǰ�������������Ѿ��������ˣ�ʲô����������������û�ж���
            }
            // ��֪��ʶ����������struct����
            else
            {
                LOG(ERROR, "%d: known identifier can not be new union name in definition\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
        }
        else
        {
            LOG(ERROR, "%d: invalid use of struct, must with an identifier\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        TokenOp.match(';');
        return;
    }

    // ��������
    if (TokenOp.token == Int)
    {
        TokenOp.match(Int);
    }
    else if (TokenOp.token == Char)
    {
        TokenOp.match(Char);
        basetype = CHAR;
    }

define_glo_func:
    // �������塢�������塢����������ֱ���������庯����������;�������������}
    while (TokenOp.token != ';' && TokenOp.token != '}')
    {
        type = basetype;
        while (TokenOp.token == Mul)
        {
            TokenOp.match(Mul);
            type = type + PTR;
        }

        // ��Ч������
        if (TokenOp.token != Id)
        {
            LOG(ERROR, "%d: invalid global declaration\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        id = current_id;
        TokenOp.match(Id);

        // ���������������
        if (TokenOp.token == '(')
        {
            // ��֧��union����struct��Ϊ��������ֵ
            if (type < PTR && type >= UNION)
            {
                LOG(ERROR, "%d: do not support struct/union to be type of function return value, please use pointer instead\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }

            // ���浱ǰ���ű���
            memcpy(&cur_func, id, sizeof(struct Symbol_item));

            // �ȸ�һ�����壬��֤�ݹ����ʱ�����Ѿ����˶��壬�����������ټ�鵱ǰ�����Ƿ��ظ����壬����ֵ�Ƿ�������ƥ���
            id->IdClass = Fun;
            id->type = type;
            id->value = (long long)(TokenOp._testVM.code + 1); // �������

            // ǰ�������������ɴ���
            if (function_declaration())
            {
                // �����Ѿ�����
                if (cur_func.IdClass)
                {
                    // �����Ѿ�����Ϊ��������
                    if (cur_func.IdClass != Fun)
                    {
                        LOG(ERROR, "%d: duplicate global declaration, identifier has been used\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // �Ѿ��������߶���Ϊ������������ֵ��ͬ
                    else if (cur_func.type != type)
                    {
                        LOG(ERROR, "%d: invalid function declaration, different return type\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // �Ѿ���������Ϊ��������ԭΪ������
                    else
                    {
                        id->value = cur_func.value;
                    }
                }
                // �µĺ�����������������ֵΪ��
                else
                {
                    id->value = 0;
                }
            }
            // ��������
            else
            {
                // �����Ѿ�����
                if (cur_func.IdClass)
                {
                    // �����Ѿ�����Ϊ��������
                    if (cur_func.IdClass != Fun)
                    {
                        LOG(ERROR, "%d: duplicate global declaration, identifier has been used\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // ͬ�������Ѿ�����
                    else if (cur_func.value)
                    {
                        LOG(ERROR, "%d: redefinition of function\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // δ���壬����ֵ��������ͬ
                    else if (cur_func.type != type)
                    {
                        LOG(ERROR, "%d: invalid function definition, different return type with declaration\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // else ������ƥ�䲢�ҵ�һ�ζ��壬�Ѿ�����
                }
                // else �µĺ��������Ѿ�����
            }
        }
        // ȫ�ֱ������壬��data�������ڴ�
        else
        {
            if (id->IdClass)
            {
                LOG(ERROR, "%d: duplicate global declaration, identifier has been used\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
            id->IdClass = Glo;
            id->type = type;
            id->value = (long long)TokenOp._testVM.data;

            // �ṹ������
            if (id->type < PTR && id->type >= UNION)
            {
                if (id->type >= STRUCT) // struct
                {
                    TokenOp._testVM.data = TokenOp._testVM.data + struct_symbols_list[id->type - STRUCT].size;
                }
                else // union
                {
                    TokenOp._testVM.data = TokenOp._testVM.data + union_symbols_list[id->type - UNION].size;
                }
            }
            // ָ�롢����,char����int��С��Ҳ����int��С���룬��֤��ַһ����sizeof(long long)������
            else
            {
                TokenOp._testVM.data = TokenOp._testVM.data + sizeof(long long);
            }

            // һ�ж���������
            if (TokenOp.token != ';')
            {
                TokenOp.match(Comma);
                if (TokenOp.token == ';')
                {
                    LOG(ERROR, "%d: expected identifier after ','\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
            }
        }
    }
    TokenOp.next(); // ; }
}
/*
����ö�ٶ��壺{}�ڵ�����
enum_body = id, ["=", number], {",", id, ["=", number]};
*/
void Parser::enum_body()
{
    long long enum_val;
    enum_val = 0;
    while (TokenOp.token != '}')
    {
        if (TokenOp.token != Id)
        {
            LOG(ERROR, "%d: invalid enum identifier %d\n", TokenOp.line, TokenOp.token);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        TokenOp.next();
        if (TokenOp.token == Assign)
        {
            TokenOp.match(Assign);
            if (TokenOp.token != Num)
            {
                LOG(ERROR, "%d: invalid enum initializer\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
            enum_val = TokenOp.token_val;
            TokenOp.next();
        }

        // ��ö�ٸ�ֵ����Ϊ����
        current_id->IdClass = EnumVal;
        current_id->type = INT;
        current_id->value = enum_val++;

        if (TokenOp.token != '}')
        {
            TokenOp.match(Comma);
        }
    }
}
/*
���������壬�Ӳ����б�ʼ
func_decl = ret_type, id, "(", param_decl, ")", "{", func_body, "}";
forward_func_decl = ret_type, id, "(", [param_decl], ")", ";";

ǰ����������1���������巵��0
*/
long long Parser::function_declaration()
{
    long long forward_decl;
    struct Label_list_item* label_list_pos;
    long long find_label;

    cur_loop = 0;
    forward_decl = 0;

    TokenOp.match('(');
    function_parameter();
    TokenOp.match(')');

    if (TokenOp.token == ';')
    {
        forward_decl = 1; // ������;����������
    }
    else
    {
        TokenOp.match('{');
        function_body();
        //match('}'); // ������}������global_declaration�����ڱ�ʶ�����������̵Ľ���
    }

    // ���goto�ı�ŵ�ַ
    for (label_list_pos = label_list; label_list_pos->label_hash; label_list_pos++)
    {
        find_label = 0;
        for (current_id = symbols; current_id->token; current_id++)
        {
            if (current_id->token == Id && current_id->IdClass == Label && current_id->hash == label_list_pos->label_hash && current_id->value)
            {
                *(long long*)label_list_pos->goto_address = current_id->value;
                find_label = 1;
                break;
            }
        }
        if (!find_label)
        {
            LOG(ERROR, "%d: undefined label for goto in function, hash: %d\n", label_list_pos->line, label_list_pos->label_hash);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        // �������������goto�б�
        label_list_pos->label_hash = 0;
        label_list_pos->goto_address = 0;
    }

    // �������ű��ָ�ȫ�ֱ������壬���û��ͬ��ȫ�ֱ�������ɾ������ű��л��и������class/type/value���ᱻ�ÿգ�����Ӱ��
    current_id = symbols;
    while (current_id->token)
    {
        if (current_id->IdClass == Loc || current_id->IdClass == Label)  // ͬʱ��ձ�Ŷ���
        {
            current_id->IdClass = current_id->gclass;
            current_id->type = current_id->gtype;
            current_id->value = current_id->gvalue;
        }
        current_id++;
    }
    return forward_decl;
}
/*
����ṹ���������������б�
struct_decl = struct, [id], "{", var_decl, {var_decl}, "}", ";";
union_decl = union, [id], "{", var_decl, {var_decl}, "}", ";";
var_decl = type {"*"}, id, {",", {"*"}, id}, ";";
type = long long | char | user_defined_type;
user_defined_type = (enum | union | struct), id;

struct_or_union: 1 struct 0 union

union�Ĵ�Сȡ���struct��С�ۼӡ�
*/
void Parser::struct_union_body(long long su_type, long long struct_or_union)
{
    long long domain_type;
    long long domain_size;
    long long cur_offset;
    long long max_size;

    struct us_domain* cur_us_symbol;    // ��ǰ���͵Ľṹ��/��������Ϣ���еļ�¼ 
    struct us_domain** next_domain;     // ������һ���ڵ��nextָ��ĵ�ַ���½���һ���ڵ�ʱ��������
    struct us_domain* cur_node;

    cur_us_symbol = struct_or_union ? &struct_symbols_list[su_type - STRUCT] : &union_symbols_list[su_type - UNION];
    next_domain = &cur_us_symbol->next;
    cur_offset = 0;
    max_size = 0;

    if (TokenOp.token == '}')
    {
        LOG(ERROR, "%d: struct/union definition can not be empty\n", TokenOp.line);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }

    while (TokenOp.token != '}')
    {
        // ����
        domain_type = parse_type();
        if (domain_type == -1)
        {
            LOG(ERROR, "%d: invalid type in struct/union definition\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }

        // ָ��
        while (TokenOp.token == Mul)
        {
            TokenOp.match(Mul);
            domain_type = domain_type + PTR;
        }

        // ȷ����Ĵ�С
        // ����
        if (domain_type >= CHAR && domain_type <= ENUM)
        {
            domain_size = sizeof(long long); // charҲ��int��С���룬�����Ż��ռ䣬����ֻ��Ϊ�˷���
        }
        // ָ��
        else if (domain_type >= PTR)
        {
            domain_size = sizeof(long long);
        }
        // �Ѷ���Ľṹ��������
        else
        {
            // struct
            if (domain_type >= STRUCT)
            {
                if (struct_symbols_list[domain_type - STRUCT].next == 0)
                {
                    LOG(ERROR, "%d: incomplete struct type can not be member of struct/union\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
                else
                {
                    domain_size = struct_symbols_list[domain_type - STRUCT].size;
                }
            }
            // union
            else if (domain_type >= UNION)
            {
                if (union_symbols_list[domain_type - UNION].next == 0)
                {
                    LOG(ERROR, "%d: incomplete union type can not be member of struct/union\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
                else
                {
                    domain_size = union_symbols_list[domain_type - UNION].size;
                }
            }
            else
            {
                LOG(ERROR, "%d: invalid type in struct/union definition\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
        }

        while (TokenOp.token != ';')
        {
            if (TokenOp.token == Id)
            {
                // �ṹ�г�Ա�����ȫ���������еķ��ų�ͻ��ֻ��Ҫ��ϣֵ���ɣ������޸ķ��ű�����Ҫ����Ƿ���struct����union�ڲ���������ͻ
                for (cur_node = cur_us_symbol->next; cur_node; cur_node = cur_node->next)
                {
                    // �Ѿ���struct/union�ж�����ͬ������
                    if (cur_node->hash == current_id->hash)
                    {
                        LOG(ERROR, "%d: struct/union member redefinition\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                }

                // ��us_domains_list���½�����ڵ㣬��ӵ���ǰ�ڵ���
                for (cur_node = us_domains_list; cur_node->hash; cur_node++);
                cur_node->hash = current_id->hash;
                cur_node->type = domain_type;
                cur_node->size = domain_size;
                cur_node->offset = struct_or_union ? cur_offset : 0;
                cur_node->next = 0;

                // ���ӵ���һ���ڵ�
                *next_domain = cur_node;
                next_domain = &cur_node->next;

                TokenOp.match(Id);
            }
            else
            {
                LOG(ERROR, "%d: invalid variable declaration in struct/union definition\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }

            // struct�ۼ�ƫ�ƣ�unionȷ�����ĳ�Ա
            cur_offset = cur_offset + domain_size;
            max_size = max_size > domain_size ? max_size : domain_size;

            if (TokenOp.token != ';')
            {
                TokenOp.match(Comma);
                if (TokenOp.token == ';')
                {
                    LOG(ERROR, "%d: invalid ',' before ';' in struct/union definition\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
            }
        }
        TokenOp.match(';');
    }

    // ����struct/union��С
    if (struct_or_union)
    {
        struct_symbols_list[su_type - STRUCT].size = cur_offset;
    }
    else
    {
        union_symbols_list[su_type - UNION].size = max_size;
    }
    // } ��������
}
/*
����������
func_body = {var_decl}, {statement};
var_decl = type {"*"}, id, {",", id}, ";";
type = long long | char | user_defiend_type;
user_defined_type = (enum | union | struct), id;

|    ....       | high address
+---------------+
| arg: param_a  |    new_bp + 3
+---------------+
| arg: param_b  |    new_bp + 2
+---------------+
|return address |    new_bp + 1
+---------------+
| old BP        | <- new BP
+---------------+
| local_1       |    new_bp - 1
+---------------+
| local_2       |    new_bp - 2
+---------------+
|    ....       |  low address

�������ɣ�
������
ENT count_of_locals
...
LEV

���÷���
�������յ���˳������ѹջ
JSR addr_of_func
ADJ count_of_params
...

��������������µ�bp��ƫ��: ..., +4, +3, +2
�����ھֲ����������bp��ƫ��: -1, -2, -3, ...

struct/union�ֲ������ķ��䣺
long long a;
struct node s;
long long b;

�����ڴ�:
...             High address        reletive address to bp
variable a                          -1
end of struct s                     -2
...
begin of struct s                   -(1+sizeof(struct node)/sizeof(long long)) <---- address of s
variable b
...             Low addresss
*/
void Parser::function_body()
{
    long long type;
    long long local_pos; // �ֲ���������
    local_pos = 0;

    // �ֲ������������������ͻ����Զ������ͣ����Ͳ���ʡ��
    while (TokenOp.token == Char || TokenOp.token == Int || TokenOp.token == Enum || TokenOp.token == Struct || TokenOp.token == Union)
    {
        // һ������Ч�����ͣ�while�Ѿ������ж�
        basetype = parse_type();

        while (TokenOp.token != ';')
        {
            type = basetype;
            while (TokenOp.token == Mul)
            {
                TokenOp.match(Mul);
                type = type + PTR;
            }

            if (TokenOp.token != Id)
            {
                LOG(ERROR, "%d: invalid local declaration\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
            if (current_id->IdClass == Loc)
            {
                LOG(ERROR, "%d: duplicate local declaration\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
            TokenOp.match(Id);

            // �ֲ����������ȫ�ֱ�����������ö��ֵ���Զ�������ͬ����Ӧ�ø����䶨��
            if (current_id->IdClass >= EnumVal && current_id->IdClass <= Glo || current_id->IdClass >= EnumType && current_id->IdClass <= StructType)
            {
                current_id->gclass = current_id->IdClass;
                current_id->gtype = current_id->type;
                current_id->gvalue = current_id->value;
            }
            current_id->IdClass = Loc;
            current_id->type = type;

            // �ṹ������
            if (type < PTR && type >= UNION)
            {
                if (type >= STRUCT) // struct
                {
                    local_pos = local_pos + struct_symbols_list[type - STRUCT].size / sizeof(long long);
                    current_id->value = local_pos + index_of_bp;
                }
                else // union
                {
                    local_pos = local_pos + union_symbols_list[type - UNION].size / sizeof(long long);
                    current_id->value = local_pos + index_of_bp;
                }
            }
            // ������ָ��
            else
            {
                current_id->value = ++local_pos + index_of_bp; // ��index_of_bp�����ֵ�õ����bpƫ�ƣ�Ϊ��ͳһ�ֲ������Ͳ����Ĵ���
            }

            if (TokenOp.token != ';')
            {
                TokenOp.match(Comma);
                if (TokenOp.token == ';')
                {
                    LOG(ERROR, "%d: expected identifier after ','\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
            }
        }
        TokenOp.match(';');
    }

    // ���ɺ�������룬��һ��ָ��л���ջ���ֲ����������ڴ棺ENT n��n�Ǿֲ���������
    *++TokenOp._testVM.code = ENT;
    *++TokenOp._testVM.code = local_pos;

    // �������ֱ����������
    while (TokenOp.token != '}')
    {
        statementobj->statement();
    }

    // �뿪����������������������ָ��LEV
    *++TokenOp._testVM.code = LEV;
}
/*
�������������б�
param_decl = type, {"*"}, id, {",", type {"*"}, id};
type = long long | char | user_defined_type;
user_defined_type = (enum | union | struct), id;
*/
void Parser::function_parameter()
{
    long long type;
    long long params;
    params = 0;
    while (TokenOp.token != ')')
    {
        type = parse_type();
        if (type == -1) // û�����ͣ�ʹ��Ĭ�ϵ�int
        {
            type = INT;
        }

        while (TokenOp.token == Mul)
        {
            type = type + PTR;
            TokenOp.match(Mul);
        }

        // ��֧��struct/union��Ϊ������������
        if (type < PTR && type >= UNION)
        {
            LOG(ERROR, "%d: do not support struct/union to be type of function paramter, please use pointer instead\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }

        // ��������
        if (TokenOp.token != Id)
        {
            LOG(ERROR, "%d: invalid parameter declaration\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        // �Ѿ�����ͬ���ֲ�����
        else if (current_id->IdClass == Loc)
        {
            LOG(ERROR, "%d: duplicate parameter declaration\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        TokenOp.match(Id);

        // ��������ͬ�ֲ����������ȫ�ֱ�����������ö��ֵ���Զ�������ͬ����Ӧ�ø����䶨��
        if (current_id->IdClass >= EnumVal && current_id->IdClass <= Glo || current_id->IdClass >= EnumType && current_id->IdClass <= StructType)
        {
            current_id->gclass = current_id->IdClass;
            current_id->gtype = current_id->type;
            current_id->gvalue = current_id->value;
        }
        current_id->IdClass = Loc;
        current_id->type = type;
        current_id->value = params++; // �������±꣬��0��ʼ�������index_of_bp��ȥ���ֵ�õ����bpƫ�ƣ�Ϊ��ͳһ�ֲ������Ͳ����Ĵ���

        if (TokenOp.token != ')')
        {
            TokenOp.match(Comma);
            if (TokenOp.token == ')')
            {
                LOG(ERROR, "%d: expected identifier after ','\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
        }
    }
    index_of_bp = params + 1; // ��¼��һ���������bpλ��
}
/*
�������ͣ�����һ�������͵ĳ������ֲ�������������������������ת����sizeof()�������
1. ȫ�ֱ�������ͺ���������������Ͷ��壬���������������������
2. �Ƿ����ͷ���-1��������Ϳ���ʡ�ԣ����纯���������棬Ӧ���������Դ���ʹ��Ĭ�ϵ�int��
3. ֻ����������ͣ�ָ������㴦��
*/
long long Parser::parse_type()
{
    long long type;
    type = INT; // Ĭ�����;���INT

    if (TokenOp.token != Int && TokenOp.token != Char && TokenOp.token != Enum && TokenOp.token != Struct && TokenOp.token != Union)
    {
        return -1;
    }

    if (TokenOp.token == Int)
    {
        TokenOp.match(Int);
    }
    else if (TokenOp.token == Char)
    {
        type = CHAR;
        TokenOp.match(Char);
    }
    // �Զ��� enum
    else if (TokenOp.token == Enum)
    {
        TokenOp.match(Enum);
        if (TokenOp.token == Id && current_id->IdClass == EnumType)
        {
            type = INT; // ��Ϊint
            TokenOp.match(Id);
        }
        else
        {
            LOG(ERROR, "%d: unknown enum type\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
    }
    // �Զ��� struct
    else if (TokenOp.token == Struct)
    {
        TokenOp.match(Struct);
        if (TokenOp.token == Id && current_id->IdClass == StructType)
        {
            type = current_id->type;
            TokenOp.match(Id);
        }
        else
        {
            LOG(ERROR, "%d: unknown struct type\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
    }
    // �Զ��� union
    else if (TokenOp.token == Union)
    {
        TokenOp.match(Union);
        if (TokenOp.token == Id && current_id->IdClass == UnionType)
        {
            type = current_id->type;
            TokenOp.match(Id);
        }
        else
        {
            LOG(ERROR, "%d: unknown union type\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
    }
    return type;
}