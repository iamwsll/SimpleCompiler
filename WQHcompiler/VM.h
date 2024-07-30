#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"Log.h"
class Parser;
// 指令集，最多一个操作数
//主体类似于x86指令集，但是函数跳转部分借鉴了JVM，同时整体精简了。这部分借鉴了@tch0
enum Instruction
{
    LEA = 100, //Load effective address to ax ax中存储 bp的地址+*pc 
	IMM, //Load immediate value to ax 将pc指向的内容立即加载到寄存器里
    JMP, // jump    pc这时候存储了一个地址，pc跳转到这个地址去
    JSR, //jump subroutine，跳转到子程序，操作数为子程序的地址 即sp--，然后里面存储pc的下一个位置（返回地址），同时pc jump操作一下
	JZ, // jump zero 看ax的值是不是0，如果是，那么跳转，如果不是，pc正常向下走
    JNZ,// jump not zero  这两个指令依靠判断寄存器的值来判断是否要jump
    ENT, //enter subroutine，进入子程序，放在子程序开头，切换栈帧，并为子程序内的局部变量分配内存，操作数为内存单元数量
	//具体就是：sp向下走，把当前函数的bp的位置存储进来。bp跳转到sp，sp再向下走*pc 个位置（给局部变量留空间），pc++
    ADJ,//adjust stack，调整栈指针sp，比如用于快速释放主调函数为参数分配的栈内存，操作数是栈帧数量。
    //sp快速返回,向上走*pc个位置。
    
    // 1个操作数，剩余的都没有操作数
    LEV, //leave subroutine，退出子程序，放在子程序的末尾，将栈帧切换回主调函数，释放子程序中的内存。
	//具体就是sp跳转到bp的位置，然后bp指向sp上一个位置（也就是返回地址），sp++（指向返回地址），然后pc指向sp位置的内容（返回地址），sp++（来恢复栈桢）
    LI, //load long long，将内存中的值加载到寄存器ax中。也就是，ax这时候存的是一个指向数据区某个int的指针，LI之后就把ax的值变成int
    LC, //load char
	SI, // save long long 这时候ax里应该存的是一个int，然后栈顶存储这个int，然后sp++（pop）
    SC, // save char
	PUSH,//寄存器ax压到栈顶 sp--，然后把ax的值存到栈顶
	//算术指令：这些操作都是把ax和栈顶的值进行操作，然后把结果存到ax里面，同时sp++（pop）
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
    OPEN, //栈顶是打开方式，栈顶下的第一个元素是文件名的地址，ax中存储fopen结果（如果是-1代表打开失败）
	READ,//栈顶是写多大，栈顶下的第一个元素是要读入到哪里，第二个元素是文件指针 ，ax中存储fread结果
	CLOS,//栈顶是文件指针，这是要关闭这个文件。ax中存储fclose结果，如果非0，代表失败，ax记录-1.否则记录0。
    WRIT,//与read类似
    PRTF,//pc+1处存储了有多少个可变参数。这些可变参数在调用PRTF之前就已经push到栈顶了。同时栈顶还存储了char*。这样把这些数据依次填充就好了 
    SCAN,//与PRTF类似
	MALC,// malloc    栈顶存储了需要分配的内存大小，ax存储了分配的内存地址
    FREE, // free    free栈顶存储的指针
    MSET, // memset   sp[2], sp[1], *sp    分别存储了 (dest, val, size)，然后ax存储了返回值
	MCMP, // memcmp   sp[2], sp[1], *sp    分别存储了 (dest, val, size)，然后ax存储了返回值
	MCPY, // memcpy   sp[2], sp[1], *sp    分别存储了 (dest, src, count)，然后ax存储了返回值
	EXIT, // exit   退出，这时候栈顶的值是返回值
};
//VM的组织架构：在那张经典的进程内存模型图中，最地下是0地址，最上面是最大地址。然后在此基础上，代码区是向上生长的，栈区是向下生长的。
//bp，sp分别指向当前函数栈桢的栈底和栈顶，pc指向下一条指令，ax是通用寄存器，cycle是指令计数器
//函数：从上到下依次是：形参，返回地址（pc指针返回后应该指向的位置），栈底（bp目前指向的位置）（内部存储了当前栈桢释放后，bp应该跳转的位置）局部变量存储，栈顶 sp
//每当push的时候，就会把一个数据放到栈顶，然后sp--，
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
	VM(long long input_poolsize = 2048 * 1024);//2048KB的data stack code区
    void init_run_VM(Parser* parserptr, int argc, char** argv);
	~VM();
    long long run_vm();
};
