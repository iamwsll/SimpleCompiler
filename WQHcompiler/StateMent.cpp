#define _CRT_SECURE_NO_WARNINGS 1
#include"StateMent.h"
StateMent::StateMent(Parser* parserptr)
	:_parserptr(parserptr) {}
/*
解析语句：
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

代码生成：
=======================if-else语句=========================
if (condition)
{
    true_statements;
}

if (condition)
{
    true_statements;
}
else
{
    false_statements;
}

[condition]
JZ [end]
[true_statements]
...                  <-----[end]

[condition]
JZ [a]
[true_statements]
JMP [end]
[false_statements]   <-----[a]
...                  <-----[end]

=======================while 语句============================
while (condition)
{
    while_statements;
}

[condition]         <-----[a]
JZ [end]
[while_statements]
JMP [a]
...                 <-----[end]

=======================for 语句=============================
for (init; condition; iter)
{
    for_statements;
}

[init]
[condition]/IMM 1       <------ [a] // has condition/no condition
JNZ [c]
JMP [end]
[iter]                  <------ [b]
JMP [a]
[for_statements]        <------ [c]
JMP [b]
...                     <------ [end]

=========================do while语句=======================
[do_while_statements]   <------ [a]
[condition]
JNZ [a]
...                     <------ [end]

=========================break 语句=========================
JMP [end]

=========================cotinue语句========================
JMP [entry]

*/
void StateMent::statement()
{
    long long* a, * b, * c, * end; // 记录保存跳转地址的code段地址，后续确定后填充
    struct Parser::Symbol_item* id;
    struct Parser::Bc_list_item* bclist_pos;
    struct Parser::Label_list_item* label_list_pos;
    long long* tmp_loop;  // 考虑循环嵌套，暂存当前循环，以便结束内层循环后恢复cur_loop，为了实现break和continue


    a = b = c = end = 0;
    tmp_loop = 0;
    bclist_pos = 0;

    // 暂存当前循环，进入内层循环时会直接覆盖cur_loop
    tmp_loop = _parserptr->cur_loop;

    // if, "(", expression, ")", statement, [else, statement]
    if (_parserptr->TokenOp.token == If)
    {
        _parserptr->TokenOp.match(If);
        _parserptr->TokenOp.match('(');
        expression(Comma);
        _parserptr->TokenOp.match(')');

        *++_parserptr->TokenOp._testVM.code = JZ;
        a = end = ++_parserptr->TokenOp._testVM.code;

        statement();
        if (_parserptr->TokenOp.token == Else)
        {
            _parserptr->TokenOp.match(Else);
            *a = (long long)(_parserptr->TokenOp._testVM.code + 3);
            *++_parserptr->TokenOp._testVM.code = JMP;
            end = ++_parserptr->TokenOp._testVM.code;
            statement();
        }
        *end = (long long)(_parserptr->TokenOp._testVM.code + 1);
    }
    // while, "(", expression, ")", statement
    else if (_parserptr->TokenOp.token == While)
    {
        _parserptr->TokenOp.match(While);
        a = _parserptr->TokenOp._testVM.code + 1;
        _parserptr->cur_loop = a;   // 保存当前循环，for break & continue

        _parserptr->TokenOp.match('(');
        expression(Comma);
        _parserptr->TokenOp.match(')');

        *++_parserptr->TokenOp._testVM.code = JZ;
        end = ++_parserptr->TokenOp._testVM.code;

        statement();

        *++_parserptr->TokenOp._testVM.code = JMP;
        *++_parserptr->TokenOp._testVM.code = (long long)a;
        *end = (long long)(_parserptr->TokenOp._testVM.code + 1);

        // 处理break和continue列表中的跳转地址
        for (bclist_pos = _parserptr->break_list; bclist_pos->loop; bclist_pos++)
        {
            if (bclist_pos->loop == _parserptr->cur_loop)
            {
                *(long long*)bclist_pos->bc_address = (long long)(_parserptr->TokenOp._testVM.code + 1);
                bclist_pos->loop = 0;
                bclist_pos->bc_address = 0;
            }
        }
        for (bclist_pos = _parserptr->continue_list; bclist_pos->loop; bclist_pos++)
        {
            if (bclist_pos->loop == _parserptr->cur_loop)
            {
                *(long long*)bclist_pos->bc_address = (long long)a;
                bclist_pos->loop = 0;
                bclist_pos->bc_address = 0;
            }
        }
    }
    // for, "(", [expression], ";", [expression], ";", [expression], ")", statement
    else if (_parserptr->TokenOp.token == For)
    {
        _parserptr->TokenOp.match(For);
        _parserptr->TokenOp.match('(');
        // 初始化表达式可能为空
        if (_parserptr->TokenOp.token != ';')
        {
            expression(Comma);
        }
        _parserptr->TokenOp.match(';');

        a = _parserptr->TokenOp._testVM.code + 1;
        _parserptr->cur_loop = a;   // 保存当前循环，for break & continue
        // 条件表达式为空
        if (_parserptr->TokenOp.token == ';')
        {
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = 1;
        }
        else
        {
            expression(Comma);
        }
        _parserptr->TokenOp.match(';');

        *++_parserptr->TokenOp._testVM.code = JNZ;
        c = ++_parserptr->TokenOp._testVM.code;
        *++_parserptr->TokenOp._testVM.code = JMP;
        end = ++_parserptr->TokenOp._testVM.code;

        b = _parserptr->TokenOp._testVM.code + 1;
        // 迭代表达式
        if (_parserptr->TokenOp.token != ')')
        {
            expression(Comma);
        }
        _parserptr->TokenOp.match(')');
        *++_parserptr->TokenOp._testVM.code = JMP;
        *++_parserptr->TokenOp._testVM.code = (long long)a;
        *c = (long long)(_parserptr->TokenOp._testVM.code + 1);

        statement();

        *++_parserptr->TokenOp._testVM.code = JMP;
        *++_parserptr->TokenOp._testVM.code = (long long)b;
        *end = (long long)(_parserptr->TokenOp._testVM.code + 1);

        // 处理break和continue列表中的跳转地址
        for (bclist_pos = _parserptr->break_list; bclist_pos->loop; bclist_pos++)
        {
            if (bclist_pos->loop == _parserptr->cur_loop)
            {
                *(long long*)bclist_pos->bc_address = (long long)(_parserptr->TokenOp._testVM.code + 1);
                bclist_pos->loop = 0;
                bclist_pos->bc_address = 0;
            }
        }
        for (bclist_pos = _parserptr->continue_list; bclist_pos->loop; bclist_pos++)
        {
            if (bclist_pos->loop == _parserptr->cur_loop)
            {
                *(long long*)bclist_pos->bc_address = (long long)b; // continue will goto iter statement
                bclist_pos->loop = 0;
                bclist_pos->bc_address = 0;
            }
        }
    }
    // do, statement, while, "(", [expression], ")", ";"
    else if (_parserptr->TokenOp.token == Do)
    {
        _parserptr->TokenOp.match(Do);
        a = _parserptr->TokenOp._testVM.code + 1;
        _parserptr->cur_loop = a;   // 保存当前循环，for break & continue
        statement();
        _parserptr->TokenOp.match(While);
        _parserptr->TokenOp.match('(');
        b = _parserptr->TokenOp._testVM.code + 1;   // for continue
        expression(Comma);
        _parserptr->TokenOp.match(')');
        _parserptr->TokenOp.match(';');
        *++_parserptr->TokenOp._testVM.code = JNZ;
        *++_parserptr->TokenOp._testVM.code = (long long)a;

        // 处理break和continue列表中的跳转地址
        for (bclist_pos = _parserptr->break_list; bclist_pos->loop; bclist_pos++)
        {
            if (bclist_pos->loop == _parserptr->cur_loop)
            {
                *(long long*)bclist_pos->bc_address = (long long)(_parserptr->TokenOp._testVM.code + 1);
                bclist_pos->loop = 0;
                bclist_pos->bc_address = 0;
            }
        }
        for (bclist_pos = _parserptr->continue_list; bclist_pos->loop; bclist_pos++)
        {
            if (bclist_pos->loop == _parserptr->cur_loop)
            {
                *(long long*)bclist_pos->bc_address = (long long)b; // continue will goto condition
                bclist_pos->loop = 0;
                bclist_pos->bc_address = 0;
            }
        }
    }
    // break, ";"
    else if (_parserptr->TokenOp.token == Break)
    {
        // 当前不在循环中，报错
        if (!_parserptr->cur_loop)
        {
            LOG(ERROR, "%d: invalid break statement, not in a loop\n", _parserptr->TokenOp.line);
            exit(-1);
        }
        _parserptr->TokenOp.match(Break);
        _parserptr->TokenOp.match(';');

        *++_parserptr->TokenOp._testVM.code = JMP;
        // 添加当前需要填充的地址到break列表末尾
        for (bclist_pos = _parserptr->break_list; bclist_pos->loop; bclist_pos++);
        bclist_pos->loop = _parserptr->cur_loop;
        bclist_pos->bc_address = ++_parserptr->TokenOp._testVM.code;
    }
    // continue, ";"
    else if (_parserptr->TokenOp.token == Continue)
    {
        // 当前不在循环中，报错
        if (!_parserptr->cur_loop)
        {
            LOG(ERROR, "%d: invalid continue statement, not in a loop\n", _parserptr->TokenOp.line);
            exit(-1);
        }
        _parserptr->TokenOp.match(Continue);
        _parserptr->TokenOp.match(';');

        *++_parserptr->TokenOp._testVM.code = JMP;
        // 添加当前需要填充的地址到continue列表末尾
        for (bclist_pos = _parserptr->continue_list; bclist_pos->loop; bclist_pos++);
        bclist_pos->loop = _parserptr->cur_loop;
        bclist_pos->bc_address = ++_parserptr->TokenOp._testVM.code;
    }
    // "{", {statement}, "}"
    else if (_parserptr->TokenOp.token == '{')
    {
        _parserptr->TokenOp.match('{');
        while (_parserptr->TokenOp.token != '}')
        {
            statement();
        }
        _parserptr->TokenOp.match('}');
    }
    // "return", [expression], ";"
    else if (_parserptr->TokenOp.token == Return)
    {
        _parserptr->TokenOp.match(Return);
        if (_parserptr->TokenOp.token != ';')
        {
            expression(Comma);
        }
        _parserptr->TokenOp.match(';');
        *++_parserptr->TokenOp._testVM.code = LEV; // leave subroutine
    }
    // ";"
    else if (_parserptr->TokenOp.token == ';')
    {
        _parserptr->TokenOp.match(';');
    }
    // goto, id, ";";
    else if (_parserptr->TokenOp.token == Goto)
    {
        _parserptr->TokenOp.match(Goto);
        _parserptr->TokenOp.match(Id);

        *++_parserptr->TokenOp._testVM.code = JMP;
        // 添加当前需要填充的地址到label列表末尾
        for (label_list_pos = _parserptr->label_list; label_list_pos->label_hash; label_list_pos++);
        label_list_pos->label_hash = _parserptr->current_id->hash;
        label_list_pos->line = _parserptr->TokenOp.line;
        label_list_pos->goto_address = ++_parserptr->TokenOp._testVM.code;

        _parserptr->TokenOp.match(';');
    }
    // expression, ";" | id, ":", statement
    else
    {
        // 记录状态
        record();

        // 尝试解析为标号
        if (_parserptr->TokenOp.token == Id)
        {
            _parserptr->TokenOp.match(Id);
            id = _parserptr->current_id;
            // 是标号
            if (_parserptr->TokenOp.token == ':')
            {
                _parserptr->TokenOp.match(':');

                // 标准C语言标号和变量、函数名是互不冲突的，但这里需要位置来保存，
                // 出于实现方便和代码清晰考虑，直接限制标号不能和类型、函数、变量、系统调用、枚举值同名。
                if (id->IdClass >= EnumVal && id->IdClass < Label)
                {
                    LOG(ERROR, "%d: label can not have a same name with types, global vars, local vars, functions, system calls, and enum values\n", _parserptr->TokenOp.line);
                    exit(-1);
                }
                else if (id->IdClass == Label)
                {
                    LOG(ERROR, "%d: labels can not have same name\n", _parserptr->TokenOp.line);
                    exit(-1);
                }

                // 定义一个新的标号
                id->IdClass = Label;
                id->value = (long long)(_parserptr->TokenOp._testVM.code + 1);

                // 标号后必须有语句，位于块末尾的必须加一个空语句;
                if (_parserptr->TokenOp.token == '}')
                {
                    LOG(ERROR, "%d: there must a statement after a label, please add a ';'\n", _parserptr->TokenOp.line);
                    exit(-1);
                }
                statement();
                _parserptr->cur_loop = tmp_loop;
                return;
            }
            // 不是标号，回溯状态到匹配标识符前
            else
            {
                backtrack();
            }
        }
        expression(Comma);
        _parserptr->TokenOp.match(';');
    }

    // 恢复当前循环
    _parserptr->cur_loop = tmp_loop;
}
/*
记录与回溯token流状态，目前只用于标号的解析，只应该用于测试能否解析，不应该在中间生成任何代码。
*/
void StateMent::record()
{
    _parserptr->TokenOp.last_src = _parserptr->TokenOp.src;
    _parserptr->TokenOp.last_token = _parserptr->TokenOp.token;
    _parserptr->TokenOp.last_token_val = _parserptr->TokenOp.token_val;
    _parserptr->TokenOp.last_line = _parserptr->TokenOp.line;
}

void StateMent::backtrack()
{
    _parserptr->TokenOp.src = _parserptr->TokenOp.last_src;
    _parserptr->TokenOp.token = _parserptr->TokenOp.last_token;
    _parserptr->TokenOp.token_val = _parserptr->TokenOp.last_token_val;
    _parserptr->TokenOp.line = _parserptr->TokenOp.last_line;
}
/*
解析表达式：

对于每个运算符，递归地向后处理高于当前运算符优先级(左结合的话是高于，右结合的话是高于和等于)的运算符后再回来处理当前的运算。
一元运算符优先级总是高于二元运算符，所以总是先处理一元运算符。

代码生成的逻辑：
1.一元运算符操作ax，二元运算符操作栈顶和ax，然后将结果保存到ax。
2.每次计算完一个子表达式后运算结果都将保存在ax中，然后计算外层表达式。
3.遇到二元运算符则会将保存在ax的左侧子表达式结果压栈，然后计算右侧子表达式结果。
*/
void StateMent::expression(long long level)
{
    struct Parser::Symbol_item* id;
    long long tmp;
    long long* addr;
    struct Parser::us_domain* cur_node;
    struct Parser::Func_call_item* func_list_pos;

    // 整数字面值
    if (_parserptr->TokenOp.token == Num)
    {
        _parserptr->TokenOp.match(Num);
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = _parserptr->TokenOp.token_val;
        _parserptr->expr_type = INT;
    }
    // 字符串字面值
    else if (_parserptr->TokenOp.token == '"')
    {
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = _parserptr->TokenOp.token_val;

        _parserptr->TokenOp.match('"');
        while (_parserptr->TokenOp.token == '"') // 处理多个字符串字面连接的情况"hello""world"
        {
            _parserptr->TokenOp.match('"');
        }
        _parserptr->TokenOp._testVM.data = (char*)(((long long)_parserptr->TokenOp._testVM.data + sizeof(long long)) & (-(long long)sizeof(long long))); // data首地址取int整数倍，同时字符串末尾填充为空位置中的0
        _parserptr->expr_type = CHAR + PTR;
    }
    // sizeof运算符：一元运算符，结果是int类型的右值
    else if (_parserptr->TokenOp.token == Sizeof)
    {
        _parserptr->TokenOp.match(Sizeof);
        _parserptr->TokenOp.match('(');
        _parserptr->expr_type = _parserptr->parse_type();
        if (_parserptr->expr_type == -1)
        {
            LOG(ERROR, "%d: invalid type in sizeof()\n", _parserptr->TokenOp.line);
            exit(-1);
        }

        while (_parserptr->TokenOp.token == Mul)
        {
            _parserptr->TokenOp.match(Mul);
            _parserptr->expr_type = _parserptr->expr_type + PTR;
        }

        _parserptr->TokenOp.match(')');

        *++_parserptr->TokenOp._testVM.code = IMM;
        if (_parserptr->expr_type == CHAR)
        {
            *++_parserptr->TokenOp._testVM.code = sizeof(char);
        }
        else if (_parserptr->expr_type < PTR && _parserptr->expr_type >= UNION)
        {
            if (_parserptr->expr_type >= STRUCT) // struct
            {
                if (_parserptr->struct_symbols_list[_parserptr->expr_type - STRUCT].next == 0)
                {
                    LOG(ERROR, "%d: can not get size of undefined struct type\n", _parserptr->TokenOp.line);
                    exit(-1);
                }
                *++_parserptr->TokenOp._testVM.code = _parserptr->struct_symbols_list[_parserptr->expr_type - STRUCT].size;
            }
            else // union
            {
                if (_parserptr->union_symbols_list[_parserptr->expr_type - UNION].next == 0)
                {
                    LOG(ERROR, "%d: can not get size of undefined union type\n", _parserptr->TokenOp.line);
                    exit(-1);
                }
                *++_parserptr->TokenOp._testVM.code = _parserptr->union_symbols_list[_parserptr->expr_type - UNION].size;
            }
        }
        else // pointer/enum/long long
        {
            *++_parserptr->TokenOp._testVM.code = sizeof(long long);
        }
        _parserptr->expr_type = INT;
    }
    // 变量与函数调用：可能是函数调用、enum值、全局/局部变量
    else if (_parserptr->TokenOp.token == Id)
    {
        _parserptr->TokenOp.match(Id);
        // 记录函数或者变量的id
        id = _parserptr->current_id;

        // 函数调用，右值
        if (_parserptr->TokenOp.token == '(')
        {
            _parserptr->TokenOp.match('(');
            tmp = 0;
            // 参数按照顺序依次压栈，标准C语言是按照逆序压栈的
            while (_parserptr->TokenOp.token != ')')
            {
                expression(Assign);
                *++_parserptr->TokenOp._testVM.code = PUSH;
                tmp++;
                if (_parserptr->TokenOp.token != ')')
                {
                    _parserptr->TokenOp.match(Comma);
                    if (_parserptr->TokenOp.token == ')')
                    {
                        LOG(ERROR, "%d: expected expression after ','\n", _parserptr->TokenOp.line);
                        exit(-1);
                    }
                }
            }
            _parserptr->TokenOp.match(')');

            // 系统调用
            if (id->IdClass == Sys)
            {
                *++_parserptr->TokenOp._testVM.code = id->value;
            }
            // 自定义函数调用
            else if (id->IdClass == Fun)
            {
                *++_parserptr->TokenOp._testVM.code = JSR;
                // 函数已经定义
                if (id->value)
                {
                    *++_parserptr->TokenOp._testVM.code = id->value;
                }
                // 函数声明了但是未定义，记录调用地址，全局声明解析结束后填充
                else
                {
                    for (func_list_pos = _parserptr->func_list; func_list_pos->hash; func_list_pos++);
                    func_list_pos->hash = id->hash;
                    func_list_pos->line = _parserptr->TokenOp.line;
                    func_list_pos->call_addresss = ++_parserptr->TokenOp._testVM.code;
                }
            }
            // 未定义的符号
            else
            {
                LOG(ERROR, "%d: call undeclared function\n", _parserptr->TokenOp.line);
                exit(-1);
            }

            // 函数调用返回时，清理为参数分配的栈空间
            if (tmp > 0)
            {
                *++_parserptr->TokenOp._testVM.code = ADJ;
                *++_parserptr->TokenOp._testVM.code = tmp;
            }
            _parserptr->expr_type = id->type; // 返回值类型
        }
        // 枚举常量，右值
        else if (id->IdClass == EnumVal)
        {
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = id->value;
            _parserptr->expr_type = INT;
        }
        // 全局或局部变量，作为左值，需要先加载地址到ax再通过LC/LI加载值，以上一条指令是否是LC/LI作为判断是否是左值的依据，
        // 需要当左值来使用时，去掉LC/LI后就能够在ax中得到左值的地址，以此来完成取地址、赋值等操作。当做右值使用时和常量、中间结果等右值无异。
        // 左值经过某些运算后还是左值比如=*[].->，经过某些运算后变成了右值+-*/%^&|>><< etc，皆通过这个机制来实现。
        // 可以说是表达式代码生成的最精髓之处！
        else
        {
            // 函数内定义的局部变量或者函数参数，加载与bp的相对地址
            if (id->IdClass == Loc)
            {
                *++_parserptr->TokenOp._testVM.code = LEA;
                *++_parserptr->TokenOp._testVM.code = _parserptr->index_of_bp - id->value;
            }
            // 全局变量则加载绝对地址
            else if (id->IdClass == Glo)
            {
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = id->value;
            }
            else
            {
                LOG(ERROR, "%d: undefined variable\n", _parserptr->TokenOp.line);
                exit(-1);
            }

            // 加载变量值到ax，地址已经由上面LEA或者IMM加载到了ax中
            _parserptr->expr_type = id->type;
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? LC : LI;
        }
    }
    // 强制类型转换、括号运算符
    else if (_parserptr->TokenOp.token == '(')
    {
        _parserptr->TokenOp.match('(');
        // 强制类型转换，获取转换类型，并直接修改expr_type中保存的类型
        if (_parserptr->TokenOp.token == Int || _parserptr->TokenOp.token == Char || _parserptr->TokenOp.token == Enum || _parserptr->TokenOp.token == Struct || _parserptr->TokenOp.token == Union)
        {
            tmp = _parserptr->parse_type();
            if (tmp == -1)
            {
                LOG(ERROR, "%d: invalid cast target type\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            while (_parserptr->TokenOp.token == Mul)
            {
                _parserptr->TokenOp.match(Mul);
                tmp = tmp + PTR;
            }
            //不支持转换为struct/union类型
            if (tmp < PTR && tmp >= UNION)
            {
                LOG(ERROR, "%d: do not support struct/union be cast target type, please consider use pointer\n", _parserptr->TokenOp.line);
                exit(-1);
            }

            _parserptr->TokenOp.match(')');

            expression(Inc); // 强制类型转换优先级同前缀++
            _parserptr->expr_type = tmp;
        }
        // 普通的括号运算符，而不是强制类型转换
        else
        {
            expression(Comma);
            _parserptr->TokenOp.match(')');
        }
    }
    // 指针解引用，得到左值
    else if (_parserptr->TokenOp.token == Mul)
    {
        _parserptr->TokenOp.match(Mul);
        expression(Inc); // 指针解引用和前缀++一个优先级，右结合

        if (_parserptr->expr_type >= PTR)
        {
            _parserptr->expr_type = _parserptr->expr_type - PTR;
        }
        else
        {
            LOG(ERROR, "%d: invalid dereference\n", _parserptr->TokenOp.line);
            exit(-1);
        }
        *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? LC : LI; // 如果是多重指针就是LI，加载地址到ax
    }
    // 取地址操作，运用于左值，得到右值
    else if (_parserptr->TokenOp.token == And)
    {
        _parserptr->TokenOp.match(And);
        expression(Inc); // 和前缀++一个优先级，右结合
        // 前一个操作如果是取值到ax，那么经过一个取地址操作就将这个操作移除就可以了，相当于还原ax为地址
        // 如果不是，说明不能取地址，因为能取地址的左值操作时都是使用地址然后LC/LI加载到ax来操作。
        if (*_parserptr->TokenOp._testVM.code == LC || *_parserptr->TokenOp._testVM.code == LI)
        {
            _parserptr->TokenOp._testVM.code--;
        }
        else
        {
            LOG(ERROR, "%d: invalid operand for address operation\n", _parserptr->TokenOp.line);
            exit(-1);
        }

        _parserptr->expr_type = _parserptr->expr_type + PTR;
    }
    // 逻辑非，得到右值
    else if (_parserptr->TokenOp.token == '!')
    {
        _parserptr->TokenOp.match('!');
        expression(Inc);

        *++_parserptr->TokenOp._testVM.code = PUSH;
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = 0;
        *++_parserptr->TokenOp._testVM.code = EQ;

        _parserptr->expr_type = INT;
    }
    // 按位取反，得到右值
    else if (_parserptr->TokenOp.token == '~')
    {
        _parserptr->TokenOp.match('~');
        expression(Inc); // 和前缀++一个优先级

        *++_parserptr->TokenOp._testVM.code = PUSH;
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = -1;
        *++_parserptr->TokenOp._testVM.code = XOR; // 和-1(0xFFFFFFFF)做异或，相当于按位取反

        _parserptr->expr_type = INT;
    }
    // 正号，得到右值
    else if (_parserptr->TokenOp.token == Add)
    {
        _parserptr->TokenOp.match(Add);
        expression(Inc);
        _parserptr->expr_type = INT;
    }
    // 负号，一元前缀右结合，得到右值
    else if (_parserptr->TokenOp.token == Sub)
    {
        _parserptr->TokenOp.match(Sub);
        // 常量取相反数
        if (_parserptr->TokenOp.token == Num)
        {
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = -_parserptr->TokenOp.token_val;
            _parserptr->TokenOp.match(Num);
        }
        // 变量取相反数，乘以-1实现
        else
        {
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = -1;
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Inc); // 和前缀++一个优先级
            *++_parserptr->TokenOp._testVM.code = MUL;
        }

        _parserptr->expr_type = INT;
    }
    // 前缀++/--，右结合，得到左值
    // Inc枚举值表示前缀++/--的优先级，后缀++/--左结合优先级高于前缀
    else if (_parserptr->TokenOp.token == Inc || _parserptr->TokenOp.token == Dec)
    {
        tmp = _parserptr->TokenOp.token;
        _parserptr->TokenOp.match(_parserptr->TokenOp.token);
        expression(Inc);

        if (*_parserptr->TokenOp._testVM.code == LC)
        {
            *_parserptr->TokenOp._testVM.code = PUSH; // 将ax地址再push到栈中，栈顶两个连续栈帧都保存同一个变量地址
            *++_parserptr->TokenOp._testVM.code = LC;
        }
        else if (*_parserptr->TokenOp._testVM.code == LI)
        {
            *_parserptr->TokenOp._testVM.code = PUSH;
            *++_parserptr->TokenOp._testVM.code = LI;
        }
        else
        {
            LOG(ERROR, "%d: invalid lvalue for pre-increment", _parserptr->TokenOp.line);
            exit(-1);
        }

        *++_parserptr->TokenOp._testVM.code = PUSH;
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type); // 需要处理是指针的情况
        *++_parserptr->TokenOp._testVM.code = (tmp == Inc) ? ADD : SUB;
        *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? SC : SI;
    }
    else
    {
        LOG(ERROR, "%d: invalid expression\n", _parserptr->TokenOp.line);
        exit(-1);
    }

    // 处理二元运算符，不断向右扫描，直到遇到优先级小于当前优先级的运算符，参数level指定了当前的优先级
    // 注意结合性的影响，左结合则向右计算优先级更高的运算符，右结合则向右计算优先级相等或者更高的运算符
    // 因为要区分不同运算符，必须使用不同枚举值，用来代指更高优先级的运算符应该选用高一级的同类优先级运算符中枚举值最小的那一个
    while (_parserptr->TokenOp.token >= level)
    {
        tmp = _parserptr->expr_type;
        // struct/union不支持除.,之外的所有运算符，统一检查并报错（->其中会检查，不在这里做）
        if (_parserptr->expr_type >= UNION && _parserptr->expr_type < PTR && _parserptr->TokenOp.token >= Assign && _parserptr->TokenOp.token <= Brak)
        {
            LOG(ERROR, "%d: invalid operator for struct/union : %d\n", _parserptr->TokenOp.line, _parserptr->TokenOp.token);
            exit(-1);
        }

        // 逗号表达式，左结合，优先级最低
        if (_parserptr->TokenOp.token == Comma)
        {
            _parserptr->TokenOp.match(Comma);
            // 什么都不做，后面的操作将覆盖ax，不需要特地清理，如果清理了前面的代码，条件跳转的地址(if ?:)可能会出现问题
            expression(Assign);
        }
        // var = expr;
        // 解析=前，已经为var生成了汇编代码，变量地址会保存在ax中
        else if (_parserptr->TokenOp.token == Assign)
        {
            // 右结合，如果有会先计算右边的赋值表达式
            _parserptr->TokenOp.match(Assign);
            // LC/LI表明上一步是加载值到ax（地址存在ax中），也就是=左边是一个左值
            if (*_parserptr->TokenOp._testVM.code == LC || *_parserptr->TokenOp._testVM.code == LI)
            {
                *_parserptr->TokenOp._testVM.code = PUSH; // 取消上一步的变量加载，转而将地址压到栈顶
            }
            else
            {
                LOG(ERROR, "%d: invalid lvalue in assignment\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            expression(Assign);

            _parserptr->expr_type = tmp;
            // 最后来将表达式的值存到栈顶地址的位置，实现赋值操作
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? SC : SI;
        }
        // expr ? a : b 三目运算符，注意中间的a相当于加了括号，需要使用最低优先级，左边的expr和b则就是?:的优先级
        else if (_parserptr->TokenOp.token == Cond)
        {
            _parserptr->TokenOp.match(Cond);
            *++_parserptr->TokenOp._testVM.code = JZ;
            addr = ++_parserptr->TokenOp._testVM.code;
            expression(Comma);
            if (_parserptr->TokenOp.token == ':') 
            {
                _parserptr->TokenOp.match(':');
            }
            else {
                LOG(ERROR, "%d: missing colon in conditional\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            *addr = (long long)(_parserptr->TokenOp._testVM.code + 3);
            *++_parserptr->TokenOp._testVM.code = JMP;
            addr = ++_parserptr->TokenOp._testVM.code;
            expression(Cond);
            *addr = (long long)(_parserptr->TokenOp._testVM.code + 1);
        }
        // ||
        else if (_parserptr->TokenOp.token == Lor)
        {
            _parserptr->TokenOp.match(Lor);
            *++_parserptr->TokenOp._testVM.code = JNZ;  // 短路求值，如果已经为true，则直接跳到下一个，多个||表达式则会一直跳到末尾
            addr = ++_parserptr->TokenOp._testVM.code;
            expression(Land);
            *addr = (long long)(_parserptr->TokenOp._testVM.code + 1);
            _parserptr->expr_type = INT;
        }
        // &&
        else if (_parserptr->TokenOp.token == Land)
        {
            _parserptr->TokenOp.match(Land);
            *++_parserptr->TokenOp._testVM.code = JZ;   // 短路求值，如果已经为false，则直接跳到下一个，多个&&表达式则会一直跳到末尾
            addr = ++_parserptr->TokenOp._testVM.code;
            expression(Or);
            *addr = (long long)(_parserptr->TokenOp._testVM.code + 1);
            _parserptr->expr_type = INT;
        }
        // |
        else if (_parserptr->TokenOp.token == Or)
        {
            _parserptr->TokenOp.match(Or);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Xor);
            *++_parserptr->TokenOp._testVM.code = OR;
            _parserptr->expr_type = INT;
        }
        // ^
        else if (_parserptr->TokenOp.token == Xor)
        {
            _parserptr->TokenOp.match(Xor);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(And);
            *++_parserptr->TokenOp._testVM.code = XOR;
            _parserptr->expr_type = INT;
        }
        // &
        else if (_parserptr->TokenOp.token == And)
        {
            _parserptr->TokenOp.match(And);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Eq);
            *++_parserptr->TokenOp._testVM.code = AND;
            _parserptr->expr_type = INT;
        }
        // ==
        else if (_parserptr->TokenOp.token == Eq)
        {
            _parserptr->TokenOp.match(Eq);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Lt);
            *++_parserptr->TokenOp._testVM.code = EQ;
            _parserptr->expr_type = INT;
        }
        // !=
        else if (_parserptr->TokenOp.token == Ne)
        {
            _parserptr->TokenOp.match(Ne);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Lt);
            *++_parserptr->TokenOp._testVM.code = NE;
            _parserptr->expr_type = INT;
        }
        // <
        else if (_parserptr->TokenOp.token == Lt)
        {
            _parserptr->TokenOp.match(Lt);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Shl);
            *++_parserptr->TokenOp._testVM.code = LT;
            _parserptr->expr_type = INT;
        }
        // <=
        else if (_parserptr->TokenOp.token == Le)
        {
            _parserptr->TokenOp.match(Le);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Shl);
            *++_parserptr->TokenOp._testVM.code = LE;
            _parserptr->expr_type = INT;
        }
        // >
        else if (_parserptr->TokenOp.token == Gt)
        {
            _parserptr->TokenOp.match(Gt);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Shl);
            *++_parserptr->TokenOp._testVM.code = GT;
            _parserptr->expr_type = INT;
        }
        // >=
        else if (_parserptr->TokenOp.token == Ge)
        {
            _parserptr->TokenOp.match(Ge);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Shl);
            *++_parserptr->TokenOp._testVM.code = GE;
            _parserptr->expr_type = INT;
        }
        // <<
        else if (_parserptr->TokenOp.token == Shl)
        {
            _parserptr->TokenOp.match(Shl);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Add);
            *++_parserptr->TokenOp._testVM.code = SHL;
            _parserptr->expr_type = INT;
        }
        // >>
        else if (_parserptr->TokenOp.token == Shr)
        {
            _parserptr->TokenOp.match(Shr);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Add);
            *++_parserptr->TokenOp._testVM.code = SHR;
            _parserptr->expr_type = INT;
        }
        // +
        else if (_parserptr->TokenOp.token == Add)
        {
            _parserptr->TokenOp.match(Add);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Mul);
            _parserptr->expr_type = tmp;
            if (_parserptr->expr_type > PTR) // pointer but not char*
            {
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type);
                *++_parserptr->TokenOp._testVM.code = MUL;
            }
            *++_parserptr->TokenOp._testVM.code = ADD;
        }
        // -
        else if (_parserptr->TokenOp.token == Sub)
        {
            _parserptr->TokenOp.match(Sub);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Mul);
            if (_parserptr->expr_type > PTR && tmp == _parserptr->expr_type) // 两个指针相减，得到偏移量
            {
                *++_parserptr->TokenOp._testVM.code = SUB;
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type);
                *++_parserptr->TokenOp._testVM.code = DIV;
                _parserptr->expr_type = INT;
            }
            else if (tmp > PTR) // 指针偏移
            {
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = get_unit_size(tmp);
                *++_parserptr->TokenOp._testVM.code = MUL;
                *++_parserptr->TokenOp._testVM.code = SUB;
                _parserptr->expr_type = tmp;
            }
            else // 整数减法、char*指针
            {
                *++_parserptr->TokenOp._testVM.code = SUB;
                _parserptr->expr_type = tmp;
            }
        }
        // *
        else if (_parserptr->TokenOp.token == Mul)
        {
            _parserptr->TokenOp.match(Mul);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Inc);
            *++_parserptr->TokenOp._testVM.code = MUL;
            _parserptr->expr_type = tmp;
        }
        // /
        else if (_parserptr->TokenOp.token == Div)
        {
            _parserptr->TokenOp.match(Div);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Inc);
            *++_parserptr->TokenOp._testVM.code = DIV;
            _parserptr->expr_type = tmp;
        }
        // %
        else if (_parserptr->TokenOp.token == Mod)
        {
            _parserptr->TokenOp.match(Mod);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Inc);
            *++_parserptr->TokenOp._testVM.code = MOD;
            _parserptr->expr_type = tmp;
        }
        // 后缀 ++ -- 最高优先级且左结合一定最先计算
        // 将变量值++或者--后将原值取到ax中
        else if (_parserptr->TokenOp.token == Inc || _parserptr->TokenOp.token == Dec)
        {
            if (*_parserptr->TokenOp._testVM.code == LI)
            {
                *_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = LI;
            }
            else if (*_parserptr->TokenOp._testVM.code == LC)
            {
                *_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = LC;
            }
            else
            {
                LOG(ERROR, "%d: invlaid lvalue long long post ++/--\n", _parserptr->TokenOp.line);
                exit(-1);
            }

            *++_parserptr->TokenOp._testVM.code = PUSH;
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type);
            *++_parserptr->TokenOp._testVM.code = (_parserptr->TokenOp.token == Inc) ? ADD : SUB;
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? SC : SI; // 先保存了++/--的结果
            *++_parserptr->TokenOp._testVM.code = PUSH;
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type);
            *++_parserptr->TokenOp._testVM.code = (_parserptr->TokenOp.token == Inc) ? SUB : ADD; // 再重新恢复原值参与计算
            _parserptr->TokenOp.match(_parserptr->TokenOp.token);
        }
        // []
        else if (_parserptr->TokenOp.token == Brak)
        {
            _parserptr->TokenOp.match(Brak);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Comma);
            _parserptr->TokenOp.match(']');
            if (tmp > PTR) // 并非char*的指针
            {
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = get_unit_size(tmp);
                *++_parserptr->TokenOp._testVM.code = MUL;
            }
            else if (tmp < PTR) // 不是指针
            {
                LOG(ERROR, "%d: pointer type expected for []\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            _parserptr->expr_type = tmp - PTR;
            *++_parserptr->TokenOp._testVM.code = ADD;
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? LC : LI;
        }
        // . -> 最高优先级，左结合，.需要左值，->不要求，得到左值，一定最先计算，不需要再向后查找更高优先级的表达式
        else if (_parserptr->TokenOp.token == Dot || _parserptr->TokenOp.token == Gmbp)
        {
            // ->
            if (_parserptr->TokenOp.token == Gmbp)
            {
                // 一层struct/union指针，此时ax中就是->左边的表达式地址，地址是否有效由写程序的人负责，这里也无法检测
                if (_parserptr->expr_type >= PTR + UNION && _parserptr->expr_type < PTR + PTR)
                {
                    _parserptr->expr_type = _parserptr->expr_type - PTR;
                }
                else
                {
                    LOG(ERROR, "%d: invalid variable type for -> operator\n", _parserptr->TokenOp.line);
                    exit(-1);
                }
            }
            // . 此时需要确保左边一定是左值
            else
            {
                // 确保是左值，则取消加载，还原地址
                if (*_parserptr->TokenOp._testVM.code == LC || *_parserptr->TokenOp._testVM.code == LI)
                {
                    _parserptr->TokenOp._testVM.code--;
                }
                else
                {
                    LOG(ERROR, "%d: invalid lvalue for operator .\n", _parserptr->TokenOp.line);
                    exit(-1);
                }
            }
            _parserptr->TokenOp.match(_parserptr->TokenOp.token);

            // 非struct/union类型
            if (_parserptr->expr_type >= PTR || _parserptr->expr_type < UNION)
            {
                LOG(ERROR, "%d: invalid lvalue for operator .\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            // 结构体/联合体的域
            if (_parserptr->TokenOp.token != Id)
            {
                LOG(ERROR, "%d: excepted member name after operator ./->\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            id = _parserptr->current_id;
            _parserptr->TokenOp.match(Id);

            cur_node = (_parserptr->expr_type >= STRUCT) ? &_parserptr->struct_symbols_list[_parserptr->expr_type - STRUCT] : &_parserptr->union_symbols_list[_parserptr->expr_type - UNION];
            cur_node = cur_node->next; // 从第一个成员开始
            for (; cur_node; cur_node = cur_node->next)
            {
                if (cur_node->hash == _parserptr->current_id->hash)
                {
                    break;
                }
            }
            // 没有这个成员
            if (!cur_node)
            {
                LOG(ERROR, "%d: invalid member name for operator ./->, hash: %d\n", _parserptr->TokenOp.line, _parserptr->current_id->hash);
                exit(-1);
            }

            // 计算指针偏移，现在ax中保存结构体的地址，struct才需要偏移，union约定所有成员地址都和union本身一致，可以不做，struct第一个成员也可以不做
            if (_parserptr->expr_type >= STRUCT && cur_node->offset != 0)
            {
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = cur_node->offset;
                *++_parserptr->TokenOp._testVM.code = ADD;
            }
            // 结果也是左值，加载结果
            _parserptr->expr_type = cur_node->type;
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? LC : LI;
        }
        else
        {
            LOG(ERROR, "%d: compiler error, token = %d\n", _parserptr->TokenOp.line, _parserptr->TokenOp.token);
            exit(-1);
        }
    }
}
/*
获取对类型进行++/--/+/-/[]运算符计算的单元大小，对于指针比较特殊。
多个运算符都要用，提取出来复用。
*/
long long StateMent::get_unit_size(long long type)
{
    long long unit_size;
    if (type > PTR)
    {
        // 一层的struct和union指针++/--时，单元大小是结构体和联合体大小
        if (type < PTR + PTR && type >= UNION + PTR)
        {
            if (type >= STRUCT + PTR) // struct*
            {
                unit_size = _parserptr->struct_symbols_list[type - PTR - STRUCT].size;
            }
            else // union *
            {
                unit_size = _parserptr->union_symbols_list[type - PTR - UNION].size;
            }
        }
        // 多层指针，整型指针
        else
        {
            unit_size = sizeof(long long);
        }
    }
    else // 整型、char*指针
    {
        unit_size = sizeof(char);
    }
    return unit_size;
}