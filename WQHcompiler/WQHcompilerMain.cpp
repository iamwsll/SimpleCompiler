#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <chrono>
#include"Log.h"
#include"VM.h"
#include"Parser.h"



void Usage(int argc, char** argv, VM& testVM)//处理命令行参数
{
    argc--;
    argv++;
    if (argc > 0 && (*argv)[0] == '-' && (*argv)[1] == 'd')//说明是debug模式
    {
        testVM.debug = 1;
        std::cout << "[###] debug mode start!" << std::endl;
        argc--;
        argv++;
    }
    if (argc < 1)//没有指明输入什么文件
    {
		std::cout << "[###] The " << "\"text.txt\"" << " file opens by default.If this is not what you want, please open it in the following format :" << std::endl;
        std::cout<<"usage: ./WQHcompiler[.exe] [-d] xxx.c [args to main]"<<std::endl<<std::endl;
        testVM.file_flag = 1;
    }

}
int32_t main(int argc, char** argv)
{
    // 获取起始时间点
    auto start = std::chrono::high_resolution_clock::now();
    
    //以下两个选项只需要放开一个
	//EnableScreen();//开启日志,向屏幕输出
	EnableFile();//开启日志，向文件输出
    VM testVM;//虚拟机
	Usage(argc, argv,testVM);//处理命令行参数

	Parser parser_test(testVM);//初始化解析器
    parser_test.read_src(argv);//读取文件
    
    parser_test.TokenOp.line = 1;
    testVM.last_code = testVM.code + 1;//初始化一些数据

    parser_test.parse();//开始解析
	testVM.init_run_VM(&parser_test, argc, argv);//虚拟机在运行前的准备

    //获得结束时间点
    auto end = std::chrono::high_resolution_clock::now();
    // 计算持续时间
    std::chrono::duration<double> duration = end - start;
    // 输出运行时间（以秒为单位）
    std::cout << "[###] Compile successfully! The total compilation time is:" << duration.count() << " s" << std::endl<<std::endl;

    // run VM虚拟机
    return testVM.run_vm();
}