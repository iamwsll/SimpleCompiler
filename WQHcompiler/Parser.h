#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include<stdlib.h>
#include"VM.h"
#include"Token.h"
#include "StateMent.h"

class StateMent;
// 标识符的类型，用在符号表中的Class域
enum Identifier_type
{
    EnumVal = 200,  // 常量枚举值
    Fun,            // 函数
    Sys,            // 系统调用
    Glo,            // 全局变量
    Loc,            // 局部变量
    EnumType,       // 用户定义的枚举类型
    StructType,     // 用户定义的结构体类型
    UnionType,      // 用户定义的联合类型
    Label           // goto的标签
};
/*
变量类型，如果是char*则表示为CHAR + PTR，char**则表示为CHAR + PTR + PTR。
枚举最终解释为整型常量，不支持命名枚举。
用最终类型模PTR得到基本类型，整除PTR则是指针层数，只存在指针这一种复合类型情况下很取巧的做法。
union的类型从100开始，struct从500开始，每定义一个类型就+1,各自减去UNION/STRUCT就是他们在
struct_symbols_list/union_symbols_list中的下标。
*/
enum Var_type { CHAR = 0, INT, ENUM, UNION = 100, STRUCT = 500, PTR = 1000 };
class Parser
{
public:
    // 符号表项：支持struct后用struct替换数组实现。
    struct Symbol_item
    {
        long long token =0;      // 标记，值应该是Token_type类型的，除了关键字之外，一定是Id。
        long long hash=0;       // 根据名称计算出的一个哈希值，加速查找，不需要每次都去遍历名字比较。
        char* name=0;     // 名称，计算出哈希值之后就不需要用它了，指向源文件某位置的char*指针。
        long long IdClass=0;      // 标识符的类型，Id类型的token才需要，值为Identifier_type中枚举。
        long long type=0;       // 标识符的变量类型或者函数返回值类型，值为Var_type枚举中普通类型与PTR组合得到的值。
        long long value=0;      // 标识符的值。如果标识符是函数，则是函数地址，如果是变量就是相对/绝对地址，如果是枚举常量则是具体的值，标号的话就是地址，自定义类型不使用。
        long long gclass=0, gtype=0, gvalue=0;  // 同class,type,value用于全局作用域覆盖局部作用域时存放全局符号。
    };

    // break和continue列表项
    struct Bc_list_item
    {
        long long* loop=0;              // 循环入口地址，仅用于唯一标识一个循环
        long long* bc_address=0;        // 需要填充的code段指令中的break或者continue跳转地址的地址
    };

    // goto列表项
    struct Label_list_item
    {
        long long label_hash=0;         // 标号的哈希
        long long line=0;               // goto语句的行号
        long long* goto_address=0;      // 需要填充的code段指令中标号地址的地址
    };

    // 尚未定义的函数调用列表
    struct Func_call_item
    {
        long long hash=0;               // 函数名哈希
        long long line=0;               // 函数调用的行号
        long long* call_addresss=0;     // 需要填充的code段指令中函数跳转地址的地址
    };
    // parser
     struct Symbol_item* symbols=0;        // 符号表
    struct Symbol_item* idmain=0;         // main函数的符号表记录
     struct Symbol_item* current_id=0;         // 当前标识符的符号表记录
    long long basetype=0;                       // 变量、函数和类型定义的基本类型，是指针类型时使用
    long long expr_type=0;                      // 表达式类型
    long long index_of_bp=0;                    // 函数调用时第一个参数相对bp的位置，函数的参数数量+1
    struct Bc_list_item* break_list=0;    // break语句跳转地址的列表
    struct Bc_list_item* continue_list=0; // continue语句跳转地址列表
    long long* cur_loop=0;                      // 保存当前正在解析的循环的地址，用来唯一标识一个循环，for break & continue
    struct Label_list_item* label_list=0; // goto语句跳转地址列表
    struct Func_call_item* func_list=0;   // 待填充的函数调用列表
    // union or struct domain
    struct us_domain
    {
        long long hash=0;               // hash of type name or var name
        long long type=0;               // type of user defined struct or domain
        long long size=0;               // size of type or domain
        long long offset=0;             // offset in struct of domain
        struct us_domain* next=0; // first domain for type or next domain for domain, indicate whether a struct/union has been already defined or not.
    };

    // 结构体和联合体信息的列表，每个元素都是一个链表的表头，存储struct和union的信息，链表中余下的节点按顺序存储它的域
    struct us_domain* struct_symbols_list=0;
    struct us_domain* union_symbols_list=0;
    struct us_domain* us_domains_list=0;      // 集中存储链表上述两个列表中链表的节点，用malloc单独分配链表节点的话给人感觉就不整体了，不是必要

    long long cur_struct_type=0;        // 当前struct类型
    long long cur_union_type=0;         // 当前union类型
	long long poolsize=0;			   // 符号表大小,注意这里和VM的符号表大小可以不一样

    TokensClass TokenOp;//内部类token
    StateMent* statementobj;// 内部类，描述语句

public:
    Parser(VM& testVM,long long input_poolsize = 256 * 1024);
    ~Parser();
    void init_symbols();
    void read_src(char** argv);
    void parse();
    void global_declaration();
    void enum_body();
    long long function_declaration();
    void struct_union_body(long long su_type, long long struct_or_union);
    void function_body();
    void function_parameter();
    long long parse_type();
};
