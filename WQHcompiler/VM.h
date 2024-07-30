#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"Log.h"
class Parser;
// ָ������һ��������
//����������x86ָ������Ǻ�����ת���ֽ����JVM��ͬʱ���徫���ˡ��ⲿ�ֽ����@tch0
enum Instruction
{
    LEA = 100, //Load effective address to ax ax�д洢 bp�ĵ�ַ+*pc 
	IMM, //Load immediate value to ax ��pcָ��������������ص��Ĵ�����
    JMP, // jump    pc��ʱ��洢��һ����ַ��pc��ת�������ַȥ
    JSR, //jump subroutine����ת���ӳ��򣬲�����Ϊ�ӳ���ĵ�ַ ��sp--��Ȼ������洢pc����һ��λ�ã����ص�ַ����ͬʱpc jump����һ��
	JZ, // jump zero ��ax��ֵ�ǲ���0������ǣ���ô��ת��������ǣ�pc����������
    JNZ,// jump not zero  ������ָ�������жϼĴ�����ֵ���ж��Ƿ�Ҫjump
    ENT, //enter subroutine�������ӳ��򣬷����ӳ���ͷ���л�ջ֡����Ϊ�ӳ����ڵľֲ����������ڴ棬������Ϊ�ڴ浥Ԫ����
	//������ǣ�sp�����ߣ��ѵ�ǰ������bp��λ�ô洢������bp��ת��sp��sp��������*pc ��λ�ã����ֲ��������ռ䣩��pc++
    ADJ,//adjust stack������ջָ��sp���������ڿ����ͷ���������Ϊ���������ջ�ڴ棬��������ջ֡������
    //sp���ٷ���,������*pc��λ�á�
    
    // 1����������ʣ��Ķ�û�в�����
    LEV, //leave subroutine���˳��ӳ��򣬷����ӳ����ĩβ����ջ֡�л��������������ͷ��ӳ����е��ڴ档
	//�������sp��ת��bp��λ�ã�Ȼ��bpָ��sp��һ��λ�ã�Ҳ���Ƿ��ص�ַ����sp++��ָ�򷵻ص�ַ����Ȼ��pcָ��spλ�õ����ݣ����ص�ַ����sp++�����ָ�ջ�壩
    LI, //load long long�����ڴ��е�ֵ���ص��Ĵ���ax�С�Ҳ���ǣ�ax��ʱ������һ��ָ��������ĳ��int��ָ�룬LI֮��Ͱ�ax��ֵ���int
    LC, //load char
	SI, // save long long ��ʱ��ax��Ӧ�ô����һ��int��Ȼ��ջ���洢���int��Ȼ��sp++��pop��
    SC, // save char
	PUSH,//�Ĵ���axѹ��ջ�� sp--��Ȼ���ax��ֵ�浽ջ��
	//����ָ���Щ�������ǰ�ax��ջ����ֵ���в�����Ȼ��ѽ���浽ax���棬ͬʱsp++��pop��
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
    OPEN, //ջ���Ǵ򿪷�ʽ��ջ���µĵ�һ��Ԫ�����ļ����ĵ�ַ��ax�д洢fopen����������-1�����ʧ�ܣ�
	READ,//ջ����д���ջ���µĵ�һ��Ԫ����Ҫ���뵽����ڶ���Ԫ�����ļ�ָ�� ��ax�д洢fread���
	CLOS,//ջ�����ļ�ָ�룬����Ҫ�ر�����ļ���ax�д洢fclose����������0������ʧ�ܣ�ax��¼-1.�����¼0��
    WRIT,//��read����
    PRTF,//pc+1���洢���ж��ٸ��ɱ��������Щ�ɱ�����ڵ���PRTF֮ǰ���Ѿ�push��ջ���ˡ�ͬʱջ�����洢��char*����������Щ�����������ͺ��� 
    SCAN,//��PRTF����
	MALC,// malloc    ջ���洢����Ҫ������ڴ��С��ax�洢�˷�����ڴ��ַ
    FREE, // free    freeջ���洢��ָ��
    MSET, // memset   sp[2], sp[1], *sp    �ֱ�洢�� (dest, val, size)��Ȼ��ax�洢�˷���ֵ
	MCMP, // memcmp   sp[2], sp[1], *sp    �ֱ�洢�� (dest, val, size)��Ȼ��ax�洢�˷���ֵ
	MCPY, // memcpy   sp[2], sp[1], *sp    �ֱ�洢�� (dest, src, count)��Ȼ��ax�洢�˷���ֵ
	EXIT, // exit   �˳�����ʱ��ջ����ֵ�Ƿ���ֵ
};
//VM����֯�ܹ��������ž���Ľ����ڴ�ģ��ͼ�У��������0��ַ��������������ַ��Ȼ���ڴ˻����ϣ������������������ģ�ջ�������������ġ�
//bp��sp�ֱ�ָ��ǰ����ջ���ջ�׺�ջ����pcָ����һ��ָ�ax��ͨ�üĴ�����cycle��ָ�������
//���������ϵ��������ǣ��βΣ����ص�ַ��pcָ�뷵�غ�Ӧ��ָ���λ�ã���ջ�ף�bpĿǰָ���λ�ã����ڲ��洢�˵�ǰջ���ͷź�bpӦ����ת��λ�ã��ֲ������洢��ջ�� sp
//ÿ��push��ʱ�򣬾ͻ��һ�����ݷŵ�ջ����Ȼ��sp--��
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
	VM(long long input_poolsize = 2048 * 1024);//2048KB��data stack code��
    void init_run_VM(Parser* parserptr, int argc, char** argv);
	~VM();
    long long run_vm();
};
