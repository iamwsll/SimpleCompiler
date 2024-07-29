#define _CRT_SECURE_NO_WARNINGS 1
#include"StateMent.h"
StateMent::StateMent(Parser* parserptr)
	:_parserptr(parserptr) {}
/*
������䣺
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

�������ɣ�
=======================if-else���=========================
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

=======================while ���============================
while (condition)
{
    while_statements;
}

[condition]         <-----[a]
JZ [end]
[while_statements]
JMP [a]
...                 <-----[end]

=======================for ���=============================
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

=========================do while���=======================
[do_while_statements]   <------ [a]
[condition]
JNZ [a]
...                     <------ [end]

=========================break ���=========================
JMP [end]

=========================cotinue���========================
JMP [entry]

*/
void StateMent::statement()
{
    long long* a, * b, * c, * end; // ��¼������ת��ַ��code�ε�ַ������ȷ�������
    struct Parser::Symbol_item* id;
    struct Parser::Bc_list_item* bclist_pos;
    struct Parser::Label_list_item* label_list_pos;
    long long* tmp_loop;  // ����ѭ��Ƕ�ף��ݴ浱ǰѭ�����Ա�����ڲ�ѭ����ָ�cur_loop��Ϊ��ʵ��break��continue


    a = b = c = end = 0;
    tmp_loop = 0;
    bclist_pos = 0;

    // �ݴ浱ǰѭ���������ڲ�ѭ��ʱ��ֱ�Ӹ���cur_loop
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
        _parserptr->cur_loop = a;   // ���浱ǰѭ����for break & continue

        _parserptr->TokenOp.match('(');
        expression(Comma);
        _parserptr->TokenOp.match(')');

        *++_parserptr->TokenOp._testVM.code = JZ;
        end = ++_parserptr->TokenOp._testVM.code;

        statement();

        *++_parserptr->TokenOp._testVM.code = JMP;
        *++_parserptr->TokenOp._testVM.code = (long long)a;
        *end = (long long)(_parserptr->TokenOp._testVM.code + 1);

        // ����break��continue�б��е���ת��ַ
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
        // ��ʼ�����ʽ����Ϊ��
        if (_parserptr->TokenOp.token != ';')
        {
            expression(Comma);
        }
        _parserptr->TokenOp.match(';');

        a = _parserptr->TokenOp._testVM.code + 1;
        _parserptr->cur_loop = a;   // ���浱ǰѭ����for break & continue
        // �������ʽΪ��
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
        // �������ʽ
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

        // ����break��continue�б��е���ת��ַ
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
        _parserptr->cur_loop = a;   // ���浱ǰѭ����for break & continue
        statement();
        _parserptr->TokenOp.match(While);
        _parserptr->TokenOp.match('(');
        b = _parserptr->TokenOp._testVM.code + 1;   // for continue
        expression(Comma);
        _parserptr->TokenOp.match(')');
        _parserptr->TokenOp.match(';');
        *++_parserptr->TokenOp._testVM.code = JNZ;
        *++_parserptr->TokenOp._testVM.code = (long long)a;

        // ����break��continue�б��е���ת��ַ
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
        // ��ǰ����ѭ���У�����
        if (!_parserptr->cur_loop)
        {
            LOG(ERROR, "%d: invalid break statement, not in a loop\n", _parserptr->TokenOp.line);
            exit(-1);
        }
        _parserptr->TokenOp.match(Break);
        _parserptr->TokenOp.match(';');

        *++_parserptr->TokenOp._testVM.code = JMP;
        // ��ӵ�ǰ��Ҫ���ĵ�ַ��break�б�ĩβ
        for (bclist_pos = _parserptr->break_list; bclist_pos->loop; bclist_pos++);
        bclist_pos->loop = _parserptr->cur_loop;
        bclist_pos->bc_address = ++_parserptr->TokenOp._testVM.code;
    }
    // continue, ";"
    else if (_parserptr->TokenOp.token == Continue)
    {
        // ��ǰ����ѭ���У�����
        if (!_parserptr->cur_loop)
        {
            LOG(ERROR, "%d: invalid continue statement, not in a loop\n", _parserptr->TokenOp.line);
            exit(-1);
        }
        _parserptr->TokenOp.match(Continue);
        _parserptr->TokenOp.match(';');

        *++_parserptr->TokenOp._testVM.code = JMP;
        // ��ӵ�ǰ��Ҫ���ĵ�ַ��continue�б�ĩβ
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
        // ��ӵ�ǰ��Ҫ���ĵ�ַ��label�б�ĩβ
        for (label_list_pos = _parserptr->label_list; label_list_pos->label_hash; label_list_pos++);
        label_list_pos->label_hash = _parserptr->current_id->hash;
        label_list_pos->line = _parserptr->TokenOp.line;
        label_list_pos->goto_address = ++_parserptr->TokenOp._testVM.code;

        _parserptr->TokenOp.match(';');
    }
    // expression, ";" | id, ":", statement
    else
    {
        // ��¼״̬
        record();

        // ���Խ���Ϊ���
        if (_parserptr->TokenOp.token == Id)
        {
            _parserptr->TokenOp.match(Id);
            id = _parserptr->current_id;
            // �Ǳ��
            if (_parserptr->TokenOp.token == ':')
            {
                _parserptr->TokenOp.match(':');

                // ��׼C���Ա�źͱ������������ǻ�����ͻ�ģ���������Ҫλ�������棬
                // ����ʵ�ַ���ʹ����������ǣ�ֱ�����Ʊ�Ų��ܺ����͡�������������ϵͳ���á�ö��ֵͬ����
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

                // ����һ���µı��
                id->IdClass = Label;
                id->value = (long long)(_parserptr->TokenOp._testVM.code + 1);

                // ��ź��������䣬λ�ڿ�ĩβ�ı����һ�������;
                if (_parserptr->TokenOp.token == '}')
                {
                    LOG(ERROR, "%d: there must a statement after a label, please add a ';'\n", _parserptr->TokenOp.line);
                    exit(-1);
                }
                statement();
                _parserptr->cur_loop = tmp_loop;
                return;
            }
            // ���Ǳ�ţ�����״̬��ƥ���ʶ��ǰ
            else
            {
                backtrack();
            }
        }
        expression(Comma);
        _parserptr->TokenOp.match(';');
    }

    // �ָ���ǰѭ��
    _parserptr->cur_loop = tmp_loop;
}
/*
��¼�����token��״̬��Ŀǰֻ���ڱ�ŵĽ�����ֻӦ�����ڲ����ܷ��������Ӧ�����м������κδ��롣
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
�������ʽ��

����ÿ����������ݹ���������ڵ�ǰ��������ȼ�(���ϵĻ��Ǹ��ڣ��ҽ�ϵĻ��Ǹ��ں͵���)����������ٻ�������ǰ�����㡣
һԪ��������ȼ����Ǹ��ڶ�Ԫ����������������ȴ���һԪ�������

�������ɵ��߼���
1.һԪ���������ax����Ԫ���������ջ����ax��Ȼ�󽫽�����浽ax��
2.ÿ�μ�����һ���ӱ��ʽ������������������ax�У�Ȼ����������ʽ��
3.������Ԫ�������Ὣ������ax������ӱ��ʽ���ѹջ��Ȼ������Ҳ��ӱ��ʽ�����
*/
void StateMent::expression(long long level)
{
    struct Parser::Symbol_item* id;
    long long tmp;
    long long* addr;
    struct Parser::us_domain* cur_node;
    struct Parser::Func_call_item* func_list_pos;

    // ��������ֵ
    if (_parserptr->TokenOp.token == Num)
    {
        _parserptr->TokenOp.match(Num);
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = _parserptr->TokenOp.token_val;
        _parserptr->expr_type = INT;
    }
    // �ַ�������ֵ
    else if (_parserptr->TokenOp.token == '"')
    {
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = _parserptr->TokenOp.token_val;

        _parserptr->TokenOp.match('"');
        while (_parserptr->TokenOp.token == '"') // �������ַ����������ӵ����"hello""world"
        {
            _parserptr->TokenOp.match('"');
        }
        _parserptr->TokenOp._testVM.data = (char*)(((long long)_parserptr->TokenOp._testVM.data + sizeof(long long)) & (-(long long)sizeof(long long))); // data�׵�ַȡint��������ͬʱ�ַ���ĩβ���Ϊ��λ���е�0
        _parserptr->expr_type = CHAR + PTR;
    }
    // sizeof�������һԪ������������int���͵���ֵ
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
    // �����뺯�����ã������Ǻ������á�enumֵ��ȫ��/�ֲ�����
    else if (_parserptr->TokenOp.token == Id)
    {
        _parserptr->TokenOp.match(Id);
        // ��¼�������߱�����id
        id = _parserptr->current_id;

        // �������ã���ֵ
        if (_parserptr->TokenOp.token == '(')
        {
            _parserptr->TokenOp.match('(');
            tmp = 0;
            // ��������˳������ѹջ����׼C�����ǰ�������ѹջ��
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

            // ϵͳ����
            if (id->IdClass == Sys)
            {
                *++_parserptr->TokenOp._testVM.code = id->value;
            }
            // �Զ��庯������
            else if (id->IdClass == Fun)
            {
                *++_parserptr->TokenOp._testVM.code = JSR;
                // �����Ѿ�����
                if (id->value)
                {
                    *++_parserptr->TokenOp._testVM.code = id->value;
                }
                // ���������˵���δ���壬��¼���õ�ַ��ȫ�������������������
                else
                {
                    for (func_list_pos = _parserptr->func_list; func_list_pos->hash; func_list_pos++);
                    func_list_pos->hash = id->hash;
                    func_list_pos->line = _parserptr->TokenOp.line;
                    func_list_pos->call_addresss = ++_parserptr->TokenOp._testVM.code;
                }
            }
            // δ����ķ���
            else
            {
                LOG(ERROR, "%d: call undeclared function\n", _parserptr->TokenOp.line);
                exit(-1);
            }

            // �������÷���ʱ������Ϊ���������ջ�ռ�
            if (tmp > 0)
            {
                *++_parserptr->TokenOp._testVM.code = ADJ;
                *++_parserptr->TokenOp._testVM.code = tmp;
            }
            _parserptr->expr_type = id->type; // ����ֵ����
        }
        // ö�ٳ�������ֵ
        else if (id->IdClass == EnumVal)
        {
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = id->value;
            _parserptr->expr_type = INT;
        }
        // ȫ�ֻ�ֲ���������Ϊ��ֵ����Ҫ�ȼ��ص�ַ��ax��ͨ��LC/LI����ֵ������һ��ָ���Ƿ���LC/LI��Ϊ�ж��Ƿ�����ֵ�����ݣ�
        // ��Ҫ����ֵ��ʹ��ʱ��ȥ��LC/LI����ܹ���ax�еõ���ֵ�ĵ�ַ���Դ������ȡ��ַ����ֵ�Ȳ�����������ֵʹ��ʱ�ͳ������м�������ֵ���졣
        // ��ֵ����ĳЩ���������ֵ����=*[].->������ĳЩ�����������ֵ+-*/%^&|>><< etc����ͨ�����������ʵ�֡�
        // ����˵�Ǳ��ʽ�������ɵ����֮����
        else
        {
            // �����ڶ���ľֲ��������ߺ���������������bp����Ե�ַ
            if (id->IdClass == Loc)
            {
                *++_parserptr->TokenOp._testVM.code = LEA;
                *++_parserptr->TokenOp._testVM.code = _parserptr->index_of_bp - id->value;
            }
            // ȫ�ֱ�������ؾ��Ե�ַ
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

            // ���ر���ֵ��ax����ַ�Ѿ�������LEA����IMM���ص���ax��
            _parserptr->expr_type = id->type;
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? LC : LI;
        }
    }
    // ǿ������ת�������������
    else if (_parserptr->TokenOp.token == '(')
    {
        _parserptr->TokenOp.match('(');
        // ǿ������ת������ȡת�����ͣ���ֱ���޸�expr_type�б��������
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
            //��֧��ת��Ϊstruct/union����
            if (tmp < PTR && tmp >= UNION)
            {
                LOG(ERROR, "%d: do not support struct/union be cast target type, please consider use pointer\n", _parserptr->TokenOp.line);
                exit(-1);
            }

            _parserptr->TokenOp.match(')');

            expression(Inc); // ǿ������ת�����ȼ�ͬǰ׺++
            _parserptr->expr_type = tmp;
        }
        // ��ͨ�������������������ǿ������ת��
        else
        {
            expression(Comma);
            _parserptr->TokenOp.match(')');
        }
    }
    // ָ������ã��õ���ֵ
    else if (_parserptr->TokenOp.token == Mul)
    {
        _parserptr->TokenOp.match(Mul);
        expression(Inc); // ָ������ú�ǰ׺++һ�����ȼ����ҽ��

        if (_parserptr->expr_type >= PTR)
        {
            _parserptr->expr_type = _parserptr->expr_type - PTR;
        }
        else
        {
            LOG(ERROR, "%d: invalid dereference\n", _parserptr->TokenOp.line);
            exit(-1);
        }
        *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? LC : LI; // ����Ƕ���ָ�����LI�����ص�ַ��ax
    }
    // ȡ��ַ��������������ֵ���õ���ֵ
    else if (_parserptr->TokenOp.token == And)
    {
        _parserptr->TokenOp.match(And);
        expression(Inc); // ��ǰ׺++һ�����ȼ����ҽ��
        // ǰһ�����������ȡֵ��ax����ô����һ��ȡ��ַ�����ͽ���������Ƴ��Ϳ����ˣ��൱�ڻ�ԭaxΪ��ַ
        // ������ǣ�˵������ȡ��ַ����Ϊ��ȡ��ַ����ֵ����ʱ����ʹ�õ�ַȻ��LC/LI���ص�ax��������
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
    // �߼��ǣ��õ���ֵ
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
    // ��λȡ�����õ���ֵ
    else if (_parserptr->TokenOp.token == '~')
    {
        _parserptr->TokenOp.match('~');
        expression(Inc); // ��ǰ׺++һ�����ȼ�

        *++_parserptr->TokenOp._testVM.code = PUSH;
        *++_parserptr->TokenOp._testVM.code = IMM;
        *++_parserptr->TokenOp._testVM.code = -1;
        *++_parserptr->TokenOp._testVM.code = XOR; // ��-1(0xFFFFFFFF)������൱�ڰ�λȡ��

        _parserptr->expr_type = INT;
    }
    // ���ţ��õ���ֵ
    else if (_parserptr->TokenOp.token == Add)
    {
        _parserptr->TokenOp.match(Add);
        expression(Inc);
        _parserptr->expr_type = INT;
    }
    // ���ţ�һԪǰ׺�ҽ�ϣ��õ���ֵ
    else if (_parserptr->TokenOp.token == Sub)
    {
        _parserptr->TokenOp.match(Sub);
        // ����ȡ�෴��
        if (_parserptr->TokenOp.token == Num)
        {
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = -_parserptr->TokenOp.token_val;
            _parserptr->TokenOp.match(Num);
        }
        // ����ȡ�෴��������-1ʵ��
        else
        {
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = -1;
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Inc); // ��ǰ׺++һ�����ȼ�
            *++_parserptr->TokenOp._testVM.code = MUL;
        }

        _parserptr->expr_type = INT;
    }
    // ǰ׺++/--���ҽ�ϣ��õ���ֵ
    // Incö��ֵ��ʾǰ׺++/--�����ȼ�����׺++/--�������ȼ�����ǰ׺
    else if (_parserptr->TokenOp.token == Inc || _parserptr->TokenOp.token == Dec)
    {
        tmp = _parserptr->TokenOp.token;
        _parserptr->TokenOp.match(_parserptr->TokenOp.token);
        expression(Inc);

        if (*_parserptr->TokenOp._testVM.code == LC)
        {
            *_parserptr->TokenOp._testVM.code = PUSH; // ��ax��ַ��push��ջ�У�ջ����������ջ֡������ͬһ��������ַ
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
        *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type); // ��Ҫ������ָ������
        *++_parserptr->TokenOp._testVM.code = (tmp == Inc) ? ADD : SUB;
        *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? SC : SI;
    }
    else
    {
        LOG(ERROR, "%d: invalid expression\n", _parserptr->TokenOp.line);
        exit(-1);
    }

    // �����Ԫ���������������ɨ�裬ֱ���������ȼ�С�ڵ�ǰ���ȼ��������������levelָ���˵�ǰ�����ȼ�
    // ע�����Ե�Ӱ�죬���������Ҽ������ȼ����ߵ���������ҽ�������Ҽ������ȼ���Ȼ��߸��ߵ������
    // ��ΪҪ���ֲ�ͬ�����������ʹ�ò�ͬö��ֵ��������ָ�������ȼ��������Ӧ��ѡ�ø�һ����ͬ�����ȼ��������ö��ֵ��С����һ��
    while (_parserptr->TokenOp.token >= level)
    {
        tmp = _parserptr->expr_type;
        // struct/union��֧�ֳ�.,֮��������������ͳһ��鲢����->���л��飬������������
        if (_parserptr->expr_type >= UNION && _parserptr->expr_type < PTR && _parserptr->TokenOp.token >= Assign && _parserptr->TokenOp.token <= Brak)
        {
            LOG(ERROR, "%d: invalid operator for struct/union : %d\n", _parserptr->TokenOp.line, _parserptr->TokenOp.token);
            exit(-1);
        }

        // ���ű��ʽ�����ϣ����ȼ����
        if (_parserptr->TokenOp.token == Comma)
        {
            _parserptr->TokenOp.match(Comma);
            // ʲô������������Ĳ���������ax������Ҫ�ص��������������ǰ��Ĵ��룬������ת�ĵ�ַ(if ?:)���ܻ��������
            expression(Assign);
        }
        // var = expr;
        // ����=ǰ���Ѿ�Ϊvar�����˻����룬������ַ�ᱣ����ax��
        else if (_parserptr->TokenOp.token == Assign)
        {
            // �ҽ�ϣ�����л��ȼ����ұߵĸ�ֵ���ʽ
            _parserptr->TokenOp.match(Assign);
            // LC/LI������һ���Ǽ���ֵ��ax����ַ����ax�У���Ҳ����=�����һ����ֵ
            if (*_parserptr->TokenOp._testVM.code == LC || *_parserptr->TokenOp._testVM.code == LI)
            {
                *_parserptr->TokenOp._testVM.code = PUSH; // ȡ����һ���ı������أ�ת������ַѹ��ջ��
            }
            else
            {
                LOG(ERROR, "%d: invalid lvalue in assignment\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            expression(Assign);

            _parserptr->expr_type = tmp;
            // ����������ʽ��ֵ�浽ջ����ַ��λ�ã�ʵ�ָ�ֵ����
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? SC : SI;
        }
        // expr ? a : b ��Ŀ�������ע���м��a�൱�ڼ������ţ���Ҫʹ��������ȼ�����ߵ�expr��b�����?:�����ȼ�
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
            *++_parserptr->TokenOp._testVM.code = JNZ;  // ��·��ֵ������Ѿ�Ϊtrue����ֱ��������һ�������||���ʽ���һֱ����ĩβ
            addr = ++_parserptr->TokenOp._testVM.code;
            expression(Land);
            *addr = (long long)(_parserptr->TokenOp._testVM.code + 1);
            _parserptr->expr_type = INT;
        }
        // &&
        else if (_parserptr->TokenOp.token == Land)
        {
            _parserptr->TokenOp.match(Land);
            *++_parserptr->TokenOp._testVM.code = JZ;   // ��·��ֵ������Ѿ�Ϊfalse����ֱ��������һ�������&&���ʽ���һֱ����ĩβ
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
            if (_parserptr->expr_type > PTR && tmp == _parserptr->expr_type) // ����ָ��������õ�ƫ����
            {
                *++_parserptr->TokenOp._testVM.code = SUB;
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type);
                *++_parserptr->TokenOp._testVM.code = DIV;
                _parserptr->expr_type = INT;
            }
            else if (tmp > PTR) // ָ��ƫ��
            {
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = get_unit_size(tmp);
                *++_parserptr->TokenOp._testVM.code = MUL;
                *++_parserptr->TokenOp._testVM.code = SUB;
                _parserptr->expr_type = tmp;
            }
            else // ����������char*ָ��
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
        // ��׺ ++ -- ������ȼ�������һ�����ȼ���
        // ������ֵ++����--��ԭֵȡ��ax��
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
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? SC : SI; // �ȱ�����++/--�Ľ��
            *++_parserptr->TokenOp._testVM.code = PUSH;
            *++_parserptr->TokenOp._testVM.code = IMM;
            *++_parserptr->TokenOp._testVM.code = get_unit_size(_parserptr->expr_type);
            *++_parserptr->TokenOp._testVM.code = (_parserptr->TokenOp.token == Inc) ? SUB : ADD; // �����»ָ�ԭֵ�������
            _parserptr->TokenOp.match(_parserptr->TokenOp.token);
        }
        // []
        else if (_parserptr->TokenOp.token == Brak)
        {
            _parserptr->TokenOp.match(Brak);
            *++_parserptr->TokenOp._testVM.code = PUSH;
            expression(Comma);
            _parserptr->TokenOp.match(']');
            if (tmp > PTR) // ����char*��ָ��
            {
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = get_unit_size(tmp);
                *++_parserptr->TokenOp._testVM.code = MUL;
            }
            else if (tmp < PTR) // ����ָ��
            {
                LOG(ERROR, "%d: pointer type expected for []\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            _parserptr->expr_type = tmp - PTR;
            *++_parserptr->TokenOp._testVM.code = ADD;
            *++_parserptr->TokenOp._testVM.code = (_parserptr->expr_type == CHAR) ? LC : LI;
        }
        // . -> ������ȼ������ϣ�.��Ҫ��ֵ��->��Ҫ�󣬵õ���ֵ��һ�����ȼ��㣬����Ҫ�������Ҹ������ȼ��ı��ʽ
        else if (_parserptr->TokenOp.token == Dot || _parserptr->TokenOp.token == Gmbp)
        {
            // ->
            if (_parserptr->TokenOp.token == Gmbp)
            {
                // һ��struct/unionָ�룬��ʱax�о���->��ߵı��ʽ��ַ����ַ�Ƿ���Ч��д������˸�������Ҳ�޷����
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
            // . ��ʱ��Ҫȷ�����һ������ֵ
            else
            {
                // ȷ������ֵ����ȡ�����أ���ԭ��ַ
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

            // ��struct/union����
            if (_parserptr->expr_type >= PTR || _parserptr->expr_type < UNION)
            {
                LOG(ERROR, "%d: invalid lvalue for operator .\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            // �ṹ��/���������
            if (_parserptr->TokenOp.token != Id)
            {
                LOG(ERROR, "%d: excepted member name after operator ./->\n", _parserptr->TokenOp.line);
                exit(-1);
            }
            id = _parserptr->current_id;
            _parserptr->TokenOp.match(Id);

            cur_node = (_parserptr->expr_type >= STRUCT) ? &_parserptr->struct_symbols_list[_parserptr->expr_type - STRUCT] : &_parserptr->union_symbols_list[_parserptr->expr_type - UNION];
            cur_node = cur_node->next; // �ӵ�һ����Ա��ʼ
            for (; cur_node; cur_node = cur_node->next)
            {
                if (cur_node->hash == _parserptr->current_id->hash)
                {
                    break;
                }
            }
            // û�������Ա
            if (!cur_node)
            {
                LOG(ERROR, "%d: invalid member name for operator ./->, hash: %d\n", _parserptr->TokenOp.line, _parserptr->current_id->hash);
                exit(-1);
            }

            // ����ָ��ƫ�ƣ�����ax�б���ṹ��ĵ�ַ��struct����Ҫƫ�ƣ�unionԼ�����г�Ա��ַ����union����һ�£����Բ�����struct��һ����ԱҲ���Բ���
            if (_parserptr->expr_type >= STRUCT && cur_node->offset != 0)
            {
                *++_parserptr->TokenOp._testVM.code = PUSH;
                *++_parserptr->TokenOp._testVM.code = IMM;
                *++_parserptr->TokenOp._testVM.code = cur_node->offset;
                *++_parserptr->TokenOp._testVM.code = ADD;
            }
            // ���Ҳ����ֵ�����ؽ��
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
��ȡ�����ͽ���++/--/+/-/[]���������ĵ�Ԫ��С������ָ��Ƚ����⡣
����������Ҫ�ã���ȡ�������á�
*/
long long StateMent::get_unit_size(long long type)
{
    long long unit_size;
    if (type > PTR)
    {
        // һ���struct��unionָ��++/--ʱ����Ԫ��С�ǽṹ����������С
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
        // ���ָ�룬����ָ��
        else
        {
            unit_size = sizeof(long long);
        }
    }
    else // ���͡�char*ָ��
    {
        unit_size = sizeof(char);
    }
    return unit_size;
}