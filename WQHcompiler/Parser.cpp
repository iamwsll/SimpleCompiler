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
    // 为parser分配内存
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
        "open read close write printf scanf malloc free memset memcmp memcpy exit void main";

    // 将关键字提前添加到符号表，在词法分析时关键字走标识符的识别流程，由于已经在符号表中，所以直接返回符号表的结果
    tmp = Break;
    while (tmp <= While)
    {
        TokenOp.getToken();
        current_id->token = tmp++; // only need token
    }

    // 将库函数添加到符号表中，和关键字含义类似
    tmp = OPEN;
    while (tmp <= EXIT)
    {
        TokenOp.getToken();
        current_id->IdClass = Sys;   // 标识符类型是系统调用
        current_id->type = INT;    // 返回值类型
        current_id->value = tmp++; // 指令
    }
    // void将被视为char处理，main被作为标识符添加到符号表，并使用idmain记录main函数的符号表项
    TokenOp.getToken();
    current_id->token = Char; // void type, regard void as char
    TokenOp.getToken();
    idmain = current_id; // keep track on main

}
void Parser::read_src(char** argv)
{
    FILE* fp;
    char* open_filename;
	if (TokenOp._testVM.file_flag == 1)//表示用户没有输入打开文件的参数
    {
		open_filename = (char*)"test.txt";
	}
    else //依靠debug标记位来找到用户想要编译的文件
    {
        open_filename = *(argv + (TokenOp._testVM.debug == 1 ? 2 : 1));
	}
    
    if ((fp = fopen(open_filename, "r")) == nullptr)
    {
        std::cout<<"Could not open(%s)"<<*argv<<std::endl;
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
	LOG(DEBUG, "open file %s\n", open_filename);
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
parser入口
（EBNF文法）(Extended Backus-Naur Form):
终结符 (Terminal): 具体的符号或字符串，通常用引号括起来表示。

例如："a" 表示字符 a。
非终结符 (Non-terminal): 语法规则的名称，通常用尖括号括起来表示。

例如：<identifier> 表示一个标识符。
选择 (Alternation): 用竖线 | 表示多个选项中的一个。

例如："a" | "b" 表示可以是 a 或 b。
串接 (Concatenation): 用空格或省略表示依次出现的符号或规则。

例如："a" "b" 表示先出现 a，再出现 b。
重复 (Repetition): 用大括号 {} 表示可以出现零次或多次。

例如：{"a"} 表示 a 可以出现零次或多次。
选项 (Optional): 用方括号 [] 表示可以出现零次或一次。

例如：["a"] 表示 a 可以出现一次或不出现。
分组 (Grouping): 用小括号 () 表示作为一个整体处理的部分。

例如：("a" "b") 表示 a 和 b 作为一个整体。
重复次数 (Repetition with specific number): 用两个数字和 , 表示重复的次数范围。

例如："a"{2,4} 表示 a 必须出现 2 到 4 次。


支持的C语言子集（EBNF文法）:
program = {global_decl};
global_decl = enum_decl | func_decl | var_decl | struct_decl | union_decl | forward_type_decl（自定义类型前置声明） | forward_func_decl(函数前置声明);
enum_decl = enum, [id(标识符名称）], "{", id, ["=", number], {",", id, ["=", number]} ,"}";
func_decl = ret_type, id, "(", [param_decl(参数列表)], ")", "{", func_body, "}";
struct_decl = struct, [id], "{", var_decl, {var_decl}, "}", ";";
union_decl = union, [id], "{", var_decl, {var_decl}, "}", ";";
forward_type_decl = (union | struct), id;
forward_func_decl = ret_type, id, "(", [param_decl], ")", ";";
param_decl = type, {"*"}, id, {",", type {"*"}, id};
ret_type = void | type, {"*"};
type = int | char | user_defined_type;
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

    cur_struct_type = STRUCT;
    cur_union_type = UNION;

    TokenOp.getToken();
	while (TokenOp.token > 0)//token是0，就说明next()的while语句里token = *src++中第一次执行src就是0，也就是说读到了源码的末尾
    {
        global_declaration();
    }

    // 全局定义解析完成后填充未定义的函数调用
    for (struct Func_call_item* func_list_pos = func_list; func_list_pos&&func_list_pos->hash; func_list_pos++)
    {
        long long find_func = 0;
        for (current_id = symbols; current_id&&current_id->token; current_id++)
        {
            if (current_id->IdClass == Fun && current_id->hash == func_list_pos->hash && current_id->value)//value条件避免了是扫描到自己
            {
				*func_list_pos->call_addresss = current_id->value;//注意这里这个解引用，call_addresss是一个地址，这个地址是在code区里的
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
        func_list_pos->call_addresss = 0;//置空
    }
}
/*
处理全局的变量定义、类型定义、以及函数定义、类型和函数的前向声明：
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
//这些文法其实就是ifelse以及递归
void Parser::global_declaration()
{
    long long type;
    struct Symbol_item* id;

    basetype = INT;

    // 解析enum: enum定义或变量声明或者作为返回值定义函数
    if (TokenOp.token == Enum)
    {
        TokenOp.match(Enum);
        // 可能是变量声明或者命名enum类型定义
        if (TokenOp.token == Id)
        {
            id = current_id;
            TokenOp.match(Id);
            // 不识别的新id，那只能是类型定义
            if (id->IdClass == 0)
            {
                id->IdClass = EnumType;
                id->type = Enum;        // 这个域意义不大，这里不区分枚举类型，都解释为int
                TokenOp.match('{');
                enum_body();
                TokenOp.match('}');
            }
            // 已定义的enum类型
            else if (id->IdClass == EnumType)//跳下去，直接被当做error处理
            {
                basetype = INT;
                goto define_glo_func;
            }
            // 已知标识符，但不是enum类型
            else
            {
                LOG(ERROR, "%d: known identifier can not be new enum name in definition\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
        }
        // 匿名enum定义
        else
        {
            TokenOp.match('{');
            enum_body();
            TokenOp.match('}');
        }
        TokenOp.match(';');
        return;//这句解析完了就返回了
    }
    // 解析struct定义、前向声明、struct变量定义、struct相关类型作为函数返回类型
    else if (TokenOp.token == Struct)
    {
        TokenOp.match(Struct);
        if (TokenOp.token == Id)
        {
            id = current_id;
            TokenOp.match(Id);

            // 不识别的新id，那只能是前向声明或者定义，确定类型值，如果是定义就解析定义
            if (id->IdClass == 0)
            {
                id->IdClass = StructType;
                id->type = cur_struct_type;
                cur_struct_type++;

                // 填充结构体类型信息
                struct_symbols_list[id->type - STRUCT].hash = id->hash;
                struct_symbols_list[id->type - STRUCT].type = id->type;
                struct_symbols_list[id->type - STRUCT].size = 0;    // 解析后确定
                struct_symbols_list[id->type - STRUCT].offset = 0;
                struct_symbols_list[id->type - STRUCT].next = 0;    // 解析后确定

                // 新的struct定义
                if (TokenOp.token == '{')
                {
                    TokenOp.match('{');
					struct_union_body(id->type, 1);//1是struct  2是union
                    TokenOp.match('}');
                }
                // 不是前向声明
                else if (TokenOp.token != ';')
                {
                    LOG(ERROR, "%d: invalid struct definition or forward declaration\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
                // else ;前向声明
            }
            // 已声明的struct类型
            else if (id->IdClass == StructType)
            {
                // struct定义
                if (TokenOp.token == '{')
                {
                    // 已经被定义
                    if (struct_symbols_list[id->type - STRUCT].next)
                    {
                        LOG(ERROR, "%d: duplicate struct definition\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // 没有定义过那就解析定义
                    else
                    {
                        TokenOp.match('{');
                        struct_union_body(id->type, 1);
                        TokenOp.match('}'); // ;最后会匹配
                    }
                }
                // 不是定义也不是前向声明，那应该就是全局变量或者函数定义
                else if (TokenOp.token != ';')
                {
                    basetype = id->type;
                    goto define_glo_func;
                }
                // else ;就是前向声明，而且已经声明过了，什么都不用做，不管有没有定义
            }
            // 已知标识符，但不是struct类型
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
    // 解析union定义、前向声明、union变量定义、union相关类型作为函数返回类型，和struct基本一模一样
    else if (TokenOp.token == Union)
    {
        TokenOp.match(Union);
        if (TokenOp.token == Id)
        {
            id = current_id;
            TokenOp.match(Id);

            // 不识别的新id，那只能是前向声明或者定义，确定类型值，如果是定义就解析定义
            if (id->IdClass == 0)
            {
                id->IdClass = UnionType;
                id->type = cur_union_type;
                cur_union_type++;

                // 填充联合体类型信息
                union_symbols_list[id->type - UNION].hash = id->hash;
                union_symbols_list[id->type - UNION].type = id->type;
                union_symbols_list[id->type - UNION].size = 0;    // 解析后确定
                union_symbols_list[id->type - UNION].offset = 0;
                union_symbols_list[id->type - UNION].next = 0;    // 解析后确定

                // 新的union定义
                if (TokenOp.token == '{')
                {
                    TokenOp.match('{');
                    struct_union_body(id->type, 0);
                    TokenOp.match('}');
                }
                // 不是前向声明
                else if (TokenOp.token != ';')
                {
                    LOG(ERROR, "%d: invalid struct definition or forward declaration\n", TokenOp.line);
                    std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                }
                // else ;前向声明
            }
            // 已声明的union类型
            else if (id->IdClass == UnionType)
            {
                // union定义
                if (TokenOp.token == '{')
                {
                    // 已经被定义
                    if (union_symbols_list[id->type - UNION].next != 0)
                    {
                        LOG(ERROR, "%d: duplicate union definition\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // 没有定义过那就解析定义
                    else
                    {
                        TokenOp.match('{');
                        struct_union_body(id->type, 0);
                        TokenOp.match('}'); // ;最后会匹配
                    }
                }
                // 不是定义也不是前向声明，那应该就是全局变量或者函数定义
                else if (TokenOp.token != ';')
                {
                    basetype = id->type;
                    goto define_glo_func;
                }
                // else ;就是前向声明，而且已经声明过了，什么都不用做，不管有没有定义
            }
            // 已知标识符，但不是struct类型
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

    // 内置类型
    if (TokenOp.token == Int)
    {
        TokenOp.match(Int);
    }
    else if (TokenOp.token == Char)
    {
        TokenOp.match(Char);
        basetype = CHAR;
    }

define_glo_func://跳转到这里，一定是开始了函数定义或者变量定义
    // 变量定义、函数定义、函数声明，直到变量定义函数声明结束;，函数定义结束}
    struct Symbol_item cur_func; // 函数解析时缓存当前符号表项
    while (TokenOp.token != ';' && TokenOp.token != '}')
    {
        type = basetype;
       
        while (TokenOp.token == Mul)//处理指针的*
        {
            TokenOp.match(Mul);
            type = type + PTR;
        }

        // 无效的声明
        if (TokenOp.token != Id)
        {
            LOG(ERROR, "%d: invalid global declaration\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        id = current_id;
        TokenOp.match(Id);

        // 函数定义或者声明
        if (TokenOp.token == '(')
        {
            // 不支持union或者struct作为函数返回值
            if (type < PTR && type >= UNION)
            {
                LOG(ERROR, "%d: do not support struct/union to be type of function return value, please use pointer instead\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }

            // 缓存当前符号表项
            memcpy(&cur_func, id, sizeof(struct Symbol_item));

            // 先给一个定义，保证递归调用时函数已经有了定义，结束解析后再检查当前函数是否重复定义，返回值是否与声明匹配等
            id->IdClass = Fun;
            id->type = type;
            id->value = (long long)(TokenOp._testVM.code + 1); // 函数入口

            // 前向声明，不生成代码
            if (function_declaration())
            {
                // 符号已经定义
                if (cur_func.IdClass)
                {
                    // 符号已经声明为其他类型
                    if (cur_func.IdClass != Fun)
                    {
                        LOG(ERROR, "%d: duplicate global declaration, identifier has been used\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // 已经声明或者定义为函数，但返回值不同
                    else if (cur_func.type != type)
                    {
                        LOG(ERROR, "%d: invalid function declaration, different return type\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // 已经声明或定义为函数，还原为缓存项
                    else
                    {
                        id->value = cur_func.value;
                    }
                }
                // 新的函数名，初次声明，值为空
                else
                {
                    id->value = 0;
                }
            }
            // 函数定义
            else
            {
                // 符号已经定义
                if (cur_func.IdClass)
                {
                    // 符号已经声明为其他类型
                    if (cur_func.IdClass != Fun)
                    {
                        LOG(ERROR, "%d: duplicate global declaration, identifier has been used\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // 同名函数已经定义
                    else if (cur_func.value)
                    {
                        LOG(ERROR, "%d: redefinition of function\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // 未定义，返回值和声明不同
                    else if (cur_func.type != type)
                    {
                        LOG(ERROR, "%d: invalid function definition, different return type with declaration\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                    // else 和声明匹配并且第一次定义，已经定义
                }
                // else 新的函数名，已经定义
            }
        }
        // 全局变量定义，在data区分配内存
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

            // 结构与联合
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
            // 指针、整型,char不足int大小的也按照int大小对齐，保证地址一定是sizeof(long long)整数倍
            else
            {
                TokenOp._testVM.data = TokenOp._testVM.data + sizeof(long long);
            }

            // 一行定义多个变量
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
    TokenOp.getToken(); // ; }
}
/*
解析枚举定义：{}内的内容
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
        TokenOp.getToken();
        if (TokenOp.token == Assign)
        {
            TokenOp.match(Assign);
            if (TokenOp.token != Num)
            {
                LOG(ERROR, "%d: invalid enum initializer\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }
            enum_val = TokenOp.token_val;
            TokenOp.getToken();
        }

        // 给枚举赋值，视为常量
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
解析函数体，从参数列表开始
func_decl = ret_type, id, "(", param_decl, ")", "{", func_body, "}";
forward_func_decl = ret_type, id, "(", [param_decl], ")", ";";

前向声明返回1，函数定义返回0
*/
long long Parser::function_declaration()
{

    cur_loop = 0;
    long long forward_decl = 0;

    TokenOp.match('(');
    function_parameter();
    TokenOp.match(')');

    if (TokenOp.token == ';')
    {
        forward_decl = 1; // 不消耗;，留到外面
    }
    else
    {
        TokenOp.match('{');
        function_body();
        //match('}'); // 不消耗}，留到global_declaration中用于标识函数解析过程的结束
    }

    // 填充goto的标号地址
    for (struct Label_list_item* label_list_pos = label_list; label_list_pos->label_hash; label_list_pos++)
    {
        long long find_label = 0;
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
        // 填充完后依次清空goto列表
        label_list_pos->label_hash = 0;
        label_list_pos->goto_address = 0;
    }

    // 遍历符号表，恢复全局变量定义，如果没有同名全局变量，则删除后符号表中还有该项，但是class/type/value都会被置空，不会影响
    current_id = symbols;
    while (current_id->token)
    {
        if (current_id->IdClass == Loc || current_id->IdClass == Label)  // 同时清空标号定义
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
处理结构体和联合体的声明列表。
struct_decl = struct, [id], "{", var_decl, {var_decl}, "}", ";";
union_decl = union, [id], "{", var_decl, {var_decl}, "}", ";";
var_decl = type {"*"}, id, {",", {"*"}, id}, ";";
type = long long | char | user_defined_type;
user_defined_type = (enum | union | struct), id;

struct_or_union: 1 struct 0 union

union的大小取最大，struct大小累加。
*/
void Parser::struct_union_body(long long su_type, long long struct_or_union)//第一个参数是结构体/联合体的type，第二个参数是1表示结构体，0表示联合体
{
	long long domain_type;//每个成员（也就是域）的类型
    long long domain_size;//每个成员的大小
	long long cur_offset;//当前域的偏移
    long long max_size;
    struct us_domain* cur_node;
    // 当前类型的结构体/联合体信息表中的记录 
    struct us_domain* cur_us_symbol = struct_or_union ? &struct_symbols_list[su_type - STRUCT] : &union_symbols_list[su_type - UNION];
    struct us_domain** next_domain = &cur_us_symbol->next; // 保存上一个节点的next指针的地址，新建下一个节点时用来连接
    cur_offset = 0;
    max_size = 0;

    if (TokenOp.token == '}')
    {
        LOG(ERROR, "%d: struct/union definition can not be empty\n", TokenOp.line);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }

    while (TokenOp.token != '}')
    {
        // 类型
        domain_type = parse_type();
        if (domain_type == -1)
        {
            LOG(ERROR, "%d: invalid type in struct/union definition\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }

        // 指针
        while (TokenOp.token == Mul)
        {
            TokenOp.match(Mul);
            domain_type = domain_type + PTR;
        }

        // 确定域的大小
        // 整型
        if (domain_type >= CHAR && domain_type <= ENUM)
        {
            domain_size = sizeof(long long); // char也按8字节大小对齐，存在优化空间，这里只是为了方便
        }
        // 指针
        else if (domain_type >= PTR)
        {
            domain_size = sizeof(long long);
        }
        // 已定义的结构或者联合
        else
        {
            // struct
            if (domain_type >= STRUCT)
            {
                if (struct_symbols_list[domain_type - STRUCT].next == 0)//也就是，这里包含的这个struct应该是一个已经完成的东西，不能直接在里面定义一个东西
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
			if (TokenOp.token == Id)//有必要对下面的做一个解释：这里curnode代表的是一个成员cur_us_symbol代表的是一个结构体/联合体。
            {
                // 结构中成员不会和全局作用域中的符号冲突，只需要哈希值即可，不会修改符号表，但需要检测是否在struct或者union内部有命名冲突
                for (cur_node = cur_us_symbol->next; cur_node; cur_node = cur_node->next)
                {
                    // 已经在struct/union中定义了同名变量
                    if (cur_node->hash == current_id->hash)
                    {
                        LOG(ERROR, "%d: struct/union member redefinition\n", TokenOp.line);
                        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
                    }
                }

                // 在us_domains_list中新建链表节点，添加到当前节点上
                for (cur_node = us_domains_list; cur_node&&cur_node->hash; cur_node++);//注意这里只是往后走
				cur_node->hash = current_id->hash;
                cur_node->type = domain_type;
                cur_node->size = domain_size;
                cur_node->offset = struct_or_union ? cur_offset : 0;
                cur_node->next = 0;

                // 连接到上一个节点
                *next_domain = cur_node;
                next_domain = &cur_node->next;

                TokenOp.match(Id);
            }
            else
            {
                LOG(ERROR, "%d: invalid variable declaration in struct/union definition\n", TokenOp.line);
                std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
            }

            // struct累计偏移，union确定最大的成员
            cur_offset = cur_offset + domain_size;//因此这里我们对齐数是0
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

    // 最终struct/union大小
    if (struct_or_union)
    {
        struct_symbols_list[su_type - STRUCT].size = cur_offset;
    }
    else
    {
        union_symbols_list[su_type - UNION].size = max_size;
    }
    // } 留到外面
}
/*
解析函数体
func_body = {var_decl}, {statement};
var_decl = type {"*"}, id, {",", id}, ";";
type = long long | char | user_defiend_type;
user_defined_type = (enum | union | struct), id;
代码生成：
函数：
ENT count_of_locals
...
LEV

调用方：
参数按照调用顺序依次压栈
JSR addr_of_func
ADJ count_of_params
...

函数参数相对于新的bp的偏移: ..., +4, +3, +2
函数内局部变量相对于bp的偏移: -1, -2, -3, ...

struct/union局部变量的分配：
long long a;
struct node s;
long long b;

分配内存:
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
    long long local_pos; // 局部变量计数
    local_pos = 0;

    // 局部变量声明，内置类型或者自定义类型，类型不能省略
    while (TokenOp.token == Char || TokenOp.token == Int || TokenOp.token == Enum || TokenOp.token == Struct || TokenOp.token == Union)
    {
        // 一定是有效的类型，while已经做了判断
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

            // 局部变量允许和全局变量、函数、枚举值、自定义类型同名，应该覆盖其定义
            if (current_id->IdClass >= EnumVal && current_id->IdClass <= Glo || current_id->IdClass >= EnumType && current_id->IdClass <= StructType)
            {
                current_id->gclass = current_id->IdClass;
                current_id->gtype = current_id->type;
                current_id->gvalue = current_id->value;
            }
            current_id->IdClass = Loc;
            current_id->type = type;

            // 结构与联合
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
            // 整型与指针
            else
            {
                current_id->value = ++local_pos + index_of_bp; // 用index_of_bp减这个值得到相对bp偏移，为了统一局部变量和参数的处理
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

    // 生成函数体代码，第一条指令，切换堆栈，局部变量分配内存：ENT n，n是局部变量个数
    *++TokenOp._testVM.code = ENT;
    *++TokenOp._testVM.code = local_pos;

    // 解析语句直到函数结束
    while (TokenOp.token != '}')
    {
        statementobj->statement();
    }

    // 离开被调函数返回主调函数的指令LEV
    *++TokenOp._testVM.code = LEV;
}
/*
解析函数参数列表
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
        if (type == -1) // 没有类型，使用默认的int
        {
            type = INT;
        }

        while (TokenOp.token == Mul)
        {
            type = type + PTR;
            TokenOp.match(Mul);
        }

        // 不支持struct/union作为函数参数类型
        if (type < PTR && type >= UNION)
        {
            LOG(ERROR, "%d: do not support struct/union to be type of function paramter, please use pointer instead\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }

        // 参数名称
        if (TokenOp.token != Id)
        {
            LOG(ERROR, "%d: invalid parameter declaration\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        // 已经定义同名局部变量
        else if (current_id->IdClass == Loc)
        {
            LOG(ERROR, "%d: duplicate parameter declaration\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
        TokenOp.match(Id);

        // 函数参数同局部变量允许和全局变量、函数、枚举值、自定义类型同名，应该覆盖其定义
        if (current_id->IdClass >= EnumVal && current_id->IdClass <= Glo || current_id->IdClass >= EnumType && current_id->IdClass <= StructType)
        {
            current_id->gclass = current_id->IdClass;
            current_id->gtype = current_id->type;
            current_id->gvalue = current_id->value;
        }
        current_id->IdClass = Loc;
        current_id->type = type;
        current_id->value = params++; // 参数的下标，从0开始，最后用index_of_bp减去这个值得到相对bp偏移，为了统一局部变量和参数的处理

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
    index_of_bp = params + 1; // 记录第一个参数相对bp位置
}
/*
解析类型：用于一定是类型的场景：局部变量声明、函数参数、类型转换、sizeof()运算符。
1. 全局变量定义和函数定义可能是类型定义，有其他情况，单独解析。
2. 非法类型返回-1，如果类型可以省略，比如函数参数里面，应该在外层加以处理，使用默认的int。
3. 只处理基本类型，指针在外层处理。
*/
long long Parser::parse_type()
{
    long long type;
    type = INT; // 默认类型就是INT

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
    // 自定义 enum
    else if (TokenOp.token == Enum)
    {
        TokenOp.match(Enum);
        if (TokenOp.token == Id && current_id->IdClass == EnumType)
        {
            type = INT; // 视为int
            TokenOp.match(Id);
        }
        else
        {
            LOG(ERROR, "%d: unknown enum type\n", TokenOp.line);
            std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
        }
    }
    // 自定义 struct
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
    // 自定义 union
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