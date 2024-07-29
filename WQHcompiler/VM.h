#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"Log.h"
class Parser;
// 指令操作码，最多一个操作数
enum Instruction
{
    LEA = 100, //Load effective address to ax 
    IMM, //Load immediate value to ax 立即加载到寄存器里
    JMP, // jump
    JSR, //jump subroutine，跳转到子程序，操作数为子程序的地址
    JZ, // jump zero
    JNZ,// jump not zero  这两个指令依靠判断寄存器的值来判断是否要jump
    ENT, //enter subroutine，进入子程序，放在子程序开头，切换栈帧，并为子程序内的局部变量分配内存，操作数为内存单元数量
    ADJ,//adjust stack，调整栈指针sp，比如用于快速释放主调函数为参数分配的栈内存，操作数是栈帧数量。
    // 1个操作数，剩余的都没有操作数
    LEV, //leave subroutine，退出子程序，放在子程序的末尾，将栈帧切换回主调函数，释放子程序中的内存。
    LI, //load long long，将内存中的值加载到寄存器ax中
    LC, //load char
    SI, // save long long
    SC, // save char
    PUSH,//寄存器ax压到栈顶
    //算术指令
    OR, //按位或
    XOR,//按位异或
    AND, //按位与
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
    //系统指令
    OPEN,
    READ,
    CLOS,
    WRIT,
    PRTF,//其实就是写到屏幕 
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
	char* data = nullptr;         // 数据段
    long long* code = nullptr;          // 代码段
	long long* stack = nullptr;         // 运行栈
    long long* pc = nullptr;
    long long* sp = nullptr;
    long long* bp = nullptr;
    long long ax = 0;
    long long cycle = 0; // 寄存器
    long long poolsize = 0; // 各个段分配的大小,单位：字节
    // debug
    long long debug = 0;                  // 调试模式
    long long* last_code = nullptr;             // 上一次打印至的code段指针
    long long file_flag = 0;
public:
	VM(long long input_poolsize = 256 * 1024);
    void init_run_VM(Parser* parserptr, int argc, char** argv);
	~VM();
    long long run_vm();
};
