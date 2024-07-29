#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <chrono>
#include"Log.h"
#include"VM.h"
#include"Parser.h"


VM testVM;//�����
void Usage(int argc, char** argv)//���������в���
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
    // ��ȡ��ʼʱ���
    auto start = std::chrono::high_resolution_clock::now();
    
    //����ѡ��ֻ��Ҫ�ſ�һ��
	//EnableScreen();//������־,����Ļ���
	EnableFile();//������־�����ļ����
	Usage(argc, argv);//���������в���
	Parser parser_test(testVM);//������
    parser_test.read_src(argv);
    parser_test.TokenOp.line = 1;
    testVM.last_code = testVM.code + 1;
    parser_test.parse();
	testVM.init_run_VM(&parser_test, argc, argv);

    //��ý���ʱ���
    auto end = std::chrono::high_resolution_clock::now();
    // �������ʱ��
    std::chrono::duration<double> duration = end - start;
    // �������ʱ�䣨����Ϊ��λ��
    std::cout << "[###] Compile successfully! The total compilation time is:" << duration.count() << " s" << std::endl<<std::endl;

    // run VM
    return testVM.run_vm();
}