#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include<stdlib.h>
#include"VM.h"
#include"Token.h"
#include "StateMent.h"

class StateMent;
// ��ʶ�������ͣ����ڷ��ű��е�Class��
enum Identifier_type
{
    EnumVal = 200,  // ����ö��ֵ
    Fun,            // ����
    Sys,            // ϵͳ����
    Glo,            // ȫ�ֱ���
    Loc,            // �ֲ�����
    EnumType,       // �û������ö������
    StructType,     // �û�����Ľṹ������
    UnionType,      // �û��������������
    Label           // goto�ı�ǩ
};
/*
�������ͣ������char*���ʾΪCHAR + PTR��char**���ʾΪCHAR + PTR + PTR��
ö�����ս���Ϊ���ͳ�������֧������ö�١�
����������ģPTR�õ��������ͣ�����PTR����ָ�������ֻ����ָ����һ�ָ�����������º�ȡ�ɵ�������
union�����ʹ�100��ʼ��struct��500��ʼ��ÿ����һ�����;�+1,���Լ�ȥUNION/STRUCT����������
struct_symbols_list/union_symbols_list�е��±ꡣ
*/
enum Var_type { CHAR = 0, INT, ENUM, UNION = 100, STRUCT = 500, PTR = 1000 };
class Parser
{
public:
    // ���ű��֧��struct����struct�滻����ʵ�֡�
    struct Symbol_item
    {
        long long token =0;      // ��ǣ�ֵӦ����Token_type���͵ģ����˹ؼ���֮�⣬һ����Id��
        long long hash=0;       // �������Ƽ������һ����ϣֵ�����ٲ��ң�����Ҫÿ�ζ�ȥ�������ֱȽϡ�
        char* name=0;     // ���ƣ��������ϣֵ֮��Ͳ���Ҫ�����ˣ�ָ��Դ�ļ�ĳλ�õ�char*ָ�롣
        long long IdClass=0;      // ��ʶ�������ͣ�Id���͵�token����Ҫ��ֵΪIdentifier_type��ö�١�
        long long type=0;       // ��ʶ���ı������ͻ��ߺ�������ֵ���ͣ�ֵΪVar_typeö������ͨ������PTR��ϵõ���ֵ��
        long long value=0;      // ��ʶ����ֵ�������ʶ���Ǻ��������Ǻ�����ַ������Ǳ����������/���Ե�ַ�������ö�ٳ������Ǿ����ֵ����ŵĻ����ǵ�ַ���Զ������Ͳ�ʹ�á�
        long long gclass=0, gtype=0, gvalue=0;  // ͬclass,type,value����ȫ�������򸲸Ǿֲ�������ʱ���ȫ�ַ��š�
    };

    // break��continue�б���
    struct Bc_list_item
    {
        long long* loop=0;              // ѭ����ڵ�ַ��������Ψһ��ʶһ��ѭ��
        long long* bc_address=0;        // ��Ҫ����code��ָ���е�break����continue��ת��ַ�ĵ�ַ
    };

    // goto�б���
    struct Label_list_item
    {
        long long label_hash=0;         // ��ŵĹ�ϣ
        long long line=0;               // goto�����к�
        long long* goto_address=0;      // ��Ҫ����code��ָ���б�ŵ�ַ�ĵ�ַ
    };

    // ��δ����ĺ��������б�
    struct Func_call_item
    {
        long long hash=0;               // ��������ϣ
        long long line=0;               // �������õ��к�
        long long* call_addresss=0;     // ��Ҫ����code��ָ���к�����ת��ַ�ĵ�ַ
    };
    // parser
     struct Symbol_item* symbols=0;        // ���ű�
    struct Symbol_item* idmain=0;         // main�����ķ��ű��¼
     struct Symbol_item* current_id=0;         // ��ǰ��ʶ���ķ��ű��¼
    long long basetype=0;                       // ���������������Ͷ���Ļ������ͣ���ָ������ʱʹ��
    long long expr_type=0;                      // ���ʽ����
    long long index_of_bp=0;                    // ��������ʱ��һ���������bp��λ�ã������Ĳ�������+1
    struct Bc_list_item* break_list=0;    // break�����ת��ַ���б�
    struct Bc_list_item* continue_list=0; // continue�����ת��ַ�б�
    long long* cur_loop=0;                      // ���浱ǰ���ڽ�����ѭ���ĵ�ַ������Ψһ��ʶһ��ѭ����for break & continue
    struct Label_list_item* label_list=0; // goto�����ת��ַ�б�
    struct Func_call_item* func_list=0;   // �����ĺ��������б�
    // union or struct domain
    struct us_domain
    {
        long long hash=0;               // hash of type name or var name
        long long type=0;               // type of user defined struct or domain
        long long size=0;               // size of type or domain
        long long offset=0;             // offset in struct of domain
        struct us_domain* next=0; // first domain for type or next domain for domain, indicate whether a struct/union has been already defined or not.
    };

    // �ṹ�����������Ϣ���б�ÿ��Ԫ�ض���һ������ı�ͷ���洢struct��union����Ϣ�����������µĽڵ㰴˳��洢������
    struct us_domain* struct_symbols_list=0;
    struct us_domain* union_symbols_list=0;
    struct us_domain* us_domains_list=0;      // ���д洢�������������б�������Ľڵ㣬��malloc������������ڵ�Ļ����˸о��Ͳ������ˣ����Ǳ�Ҫ

    long long cur_struct_type=0;        // ��ǰstruct����
    long long cur_union_type=0;         // ��ǰunion����
	long long poolsize=0;			   // ���ű��С,ע�������VM�ķ��ű��С���Բ�һ��

    TokensClass TokenOp;//�ڲ���token
    StateMent* statementobj;// �ڲ��࣬�������

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
