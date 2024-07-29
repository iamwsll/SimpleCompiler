#define _CRT_SECURE_NO_WARNINGS 1
#include"VM.h"
#include"Parser.h"
#include <chrono>
VM::VM(long long input_poolsize)
    :poolsize(input_poolsize)
{
    // 为VM分配内存
    if (!(code = last_code = (long long*)malloc(poolsize)))
    {
        LOG(FATAL, "Could not malloc(%d) for code area\n", poolsize);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(data = (char*)malloc(poolsize)))
    {
        LOG(FATAL, "Could not malloc(%d) for data area\n", poolsize);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    if (!(stack = (long long*)malloc(poolsize)))
    {
        LOG(FATAL, "Could not malloc(%d) for stack area\n", poolsize);
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }
    memset(code, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);
}
void VM::init_run_VM(Parser* parserptr,int argc, char** argv)
{
    
    // 从main开始执行
    if (!(pc = (long long*) parserptr->idmain->value))
    {
        LOG(ERROR, "main() not defined\n");
        std::cout<<"[###] process error! please see in \"log.txt\" !"<<std::endl;exit(-1);
    }

    // 初始化VM寄存器，传递参数给main，main结束后执行PUSH和EXIT两条指令
    bp = sp = (long long*)((long long)stack+poolsize); // stack top
    *--sp = EXIT;
    *--sp = PUSH;
    long long* tmpp = sp;
    *--sp = argc;
    *--sp = (long long)argv;
    *--sp = (long long)tmpp;  // 返回地址，然后开始执行PUSH和EXIT

}
VM::~VM()
{
}
long long VM::run_vm()
{
    // 获取起始时间点
    auto run_start = std::chrono::high_resolution_clock::now();
    std::cout << "[###] process start running!" << std::endl << std::endl;
    long long op, * tmp;
    FILE* file_record = fopen("running_code_record.txt", "a");
    while (1)
    {
        op = *pc++;
        cycle++;
        if (debug == 1)
        {
            if (file_record != NULL) 
            {
                fprintf(file_record, "%d >    %.4s", cycle, &"LEA ,IMM ,JMP ,JSR ,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                    "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                    "OPEN,READ,CLOS,WRIT,PRTF,MALC,FREE,MSET,MCMP,MCPY,EXIT,"[(op - LEA) * 5]);
                if (op >= JMP && op <= JNZ)
                    fprintf(file_record, "0x%.10llX\n", *pc);
                else if (op <= ADJ)
                    fprintf(file_record, "%lld\n", *pc);
                else
                    fprintf(file_record, "\n");
            }
            else 
            {
                LOG(FATAL,"Error opening file_record");
				std::cout << "[###] process error! please see in \"log.txt\" !" << std::endl; return(-1);
            }
        }

        // load & store
        if (op == IMM) { ax = *pc++; }                      // load immediate to ax
        else if (op == LEA) { ax = (long long)(bp + *pc++); }     // load address to ax
        else if (op == LC) { ax = *(char*)ax; }             // load char to ax
        else if (op == LI) { ax = *(long long*)ax; }              // load long long to ax
        else if (op == SC) { *(char*)*sp++ = ax; }          // save char to address in stack top, then pop
        else if (op == SI) { *(long long*)*sp++ = ax; }           // save long long to address in stack top, then pop
        else if (op == PUSH) *--sp = ax;                    // push ax to stack top

        // jump & function call
        else if (op == JMP) pc = (long long*)*pc;                                  // jump to addresss
        else if (op == JZ) { pc = ax ? pc + 1 : (long long*)*pc; }                 // jump to address if ax is zero
        else if (op == JNZ) { pc = ax ? (long long*)*pc : pc + 1; }                // jump to address if ax is not zero
        else if (op == JSR) { *--sp = (long long)(pc + 1); pc = (long long*)*pc; }       // jump to subroutine
        else if (op == ENT) { *--sp = (long long)bp; bp = sp; sp = sp - *pc++; }   // enter subroutine, make new stack frame
        else if (op == ADJ) sp = sp + *pc++;                                 // adjust stack
        else if (op == LEV) { sp = bp; bp = (long long*)*sp++; pc = (long long*)*sp++; } // leave/return from subroutine

        // calculation: arithmetic/bit-wise/logical
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
        else if (op == MOD) ax = *sp++ % ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ > ax;
        else if (op == GE)  ax = *sp++ >= ax;

        // system calls, pass arguments through stack
        else if (op == OPEN) {
            FILE* file = fopen((char*)sp[1], (char*)sp[0]);          // (filename, mode)
            ax = (file != NULL) ? (long long)file : -1;
        }
        //补充一点：如果虚拟机上跑的程序调用了open，那么请确保./WQHcompiler vituralprocess.cc xxxx第三个参数xxxx存在,不能不写，否则会崩
        else if (op == READ) {
            size_t bytesRead = fread((char*)sp[1], 1, *sp, (FILE*)sp[2]);  // (buffer, size, count, fileHandle)
            ax = (long long)bytesRead;
        }
        else if (op == CLOS) {
            long long result = fclose((FILE*)*sp);                          // (fileHandle)
            ax = (result == 0) ? 0 : -1;
        }
        else if (op == WRIT) {
            size_t bytesWritten = fwrite((char*)sp[1], 1, *sp, (FILE*)sp[2]);  // (buffer, size, count, fileHandle)
            ax = (long long)bytesWritten;
        }
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char*)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC) ax = (long long)malloc(*sp);                // (size)
        else if (op == FREE) free((void*)*sp);                     // (addr)
        else if (op == MSET) ax = (long long)memset((char*)sp[2], sp[1], *sp);        // (dest, val, size)
        else if (op == MCMP) ax = (long long)memcmp((char*)sp[2], (char*)sp[1], *sp); // (dest, val, size)
        else if (op == MCPY) ax = (long long)memcpy((char*)sp[2], (char*)sp[1], *sp); // (dest, src, count)
        else if (op == EXIT) {
            LOG(DEBUG, "exit(%d), running %d codes \n\n", *sp, cycle);
            std::cout << "\n[###] process exit successfully!,return val: " << *sp << std::endl;
            // 获取结束时间点
            auto run_end = std::chrono::high_resolution_clock::now();

            // 计算持续时间
            std::chrono::duration<double> duration = run_end - run_start;

            // 输出运行时间（以秒为单位）
            std::cout << "[###] process run time:" << duration.count() << " s" << std::endl;
            fprintf(file_record, "\n");
            fclose(file_record);
            return *sp;
        }
        else { LOG(ERROR, "unkown instruction = %d! cycle = %d\n", op, cycle); 
        std::cout << "[###] process error! please see in \"log.txt\" !" << std::endl;
        fprintf(file_record, "\n");
        fclose(file_record);
        return -1; }
    }
    return 0;
}