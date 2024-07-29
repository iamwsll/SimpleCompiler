#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <chrono>
#include"Log.h"
#include"VM.h"
#include"Parser.h"


VM testVM;//虚拟机
void Usage(int argc, char** argv)//处理命令行参数
{
    argc--;
    argv++;
    if (argc > 0 && (*argv)[0] == '-' && (*argv)[1] == 'd')
    {
        testVM.debug = 1;
        argc--;
        argv++;
    }
    //if (argc < 1)
    //{
    //    std::cout<<"usage: ./SimpleCompilerWQH [-d] xxx.c [args to main]"<<std::endl;
    //    exit(-1);
    //}

}
int32_t main(int argc, char** argv)
{
    // 获取起始时间点
    auto start = std::chrono::high_resolution_clock::now();
    
    //两个选项只需要放开一个
	//EnableScreen();//开启日志,向屏幕输出
	EnableFile();//开启日志，向文件输出
	Usage(argc, argv);//处理命令行参数
	Parser parser_test(testVM);//解析器
    parser_test.read_src(argv);
    parser_test.TokenOp.line = 1;
    testVM.last_code = testVM.code + 1;
    parser_test.parse();
	testVM.init_run_VM(&parser_test, argc, argv);

    //获得结束时间点
    auto end = std::chrono::high_resolution_clock::now();
    // 计算持续时间
    std::chrono::duration<double> duration = end - start;
    // 输出运行时间（以秒为单位）
    std::cout << "[###] Compile successfully! The total compilation time is:" << duration.count() << " s" << std::endl<<std::endl;

    // run VM
    return testVM.run_vm();
}