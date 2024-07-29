#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"Log.h"
class Parser;
// ָ������룬���һ��������
enum Instruction
{
    LEA = 100, //Load effective address to ax 
    IMM, //Load immediate value to ax �������ص��Ĵ�����
    JMP, // jump
    JSR, //jump subroutine����ת���ӳ��򣬲�����Ϊ�ӳ���ĵ�ַ
    JZ, // jump zero
    JNZ,// jump not zero  ������ָ�������жϼĴ�����ֵ���ж��Ƿ�Ҫjump
    ENT, //enter subroutine�������ӳ��򣬷����ӳ���ͷ���л�ջ֡����Ϊ�ӳ����ڵľֲ����������ڴ棬������Ϊ�ڴ浥Ԫ����
    ADJ,//adjust stack������ջָ��sp���������ڿ����ͷ���������Ϊ���������ջ�ڴ棬��������ջ֡������
    // 1����������ʣ��Ķ�û�в�����
    LEV, //leave subroutine���˳��ӳ��򣬷����ӳ����ĩβ����ջ֡�л��������������ͷ��ӳ����е��ڴ档
    LI, //load long long�����ڴ��е�ֵ���ص��Ĵ���ax��
    LC, //load char
    SI, // save long long
    SC, // save char
    PUSH,//�Ĵ���axѹ��ջ��
    //����ָ��
    OR, //��λ��
    XOR,//��λ���
    AND, //��λ��
    EQ, // ==
    NE, // !=
    LT,// <
    GT,// >
    LE,// <=
    GE, // >=
    SHL, // <<
    SHR, // >>
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    //ϵͳָ��
    OPEN,
    READ,
    CLOS,
    WRIT,
    PRTF,//��ʵ����д����Ļ 
    MALC,// malloc
    FREE, // free
    MSET, // memset
    MCMP, // memcmp
    MCPY, // memcpy
    EXIT, // exit
};

class VM
{
public:
	char* data = nullptr;         // ���ݶ�
    long long* code = nullptr;          // �����
	long long* stack = nullptr;         // ����ջ
    long long* pc = nullptr;
    long long* sp = nullptr;
    long long* bp = nullptr;
    long long ax = 0;
    long long cycle = 0; // �Ĵ���
    long long poolsize = 0; // �����η���Ĵ�С,��λ���ֽ�
    // debug
    long long debug = 0;                  // ����ģʽ
    long long* last_code = nullptr;             // ��һ�δ�ӡ����code��ָ��
    long long file_flag = 0;
public:
	VM(long long input_poolsize = 256 * 1024);
    void init_run_VM(Parser* parserptr, int argc, char** argv);
	~VM();
    long long run_vm();
};
