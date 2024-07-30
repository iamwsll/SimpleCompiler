#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <chrono>
#include"Log.h"
#include"VM.h"
#include"Parser.h"



void Usage(int argc, char** argv, VM& testVM)//���������в���
{
    argc--;
    argv++;
    if (argc > 0 && (*argv)[0] == '-' && (*argv)[1] == 'd')//˵����debugģʽ
    {
        testVM.debug = 1;
        std::cout << "[###] debug mode start!" << std::endl;
        argc--;
        argv++;
    }
    if (argc < 1)//û��ָ������ʲô�ļ�
    {
		std::cout << "[###] The " << "\"text.txt\"" << " file opens by default.If this is not what you want, please open it in the following format :" << std::endl;
        std::cout<<"usage: ./WQHcompiler[.exe] [-d] xxx.c [args to main]"<<std::endl<<std::endl;
        testVM.file_flag = 1;
    }

}
int32_t main(int argc, char** argv)
{
    // ��ȡ��ʼʱ���
    auto start = std::chrono::high_resolution_clock::now();
    
    //��������ѡ��ֻ��Ҫ�ſ�һ��
	//EnableScreen();//������־,����Ļ���
	EnableFile();//������־�����ļ����
    VM testVM;//�����
	Usage(argc, argv,testVM);//���������в���

	Parser parser_test(testVM);//��ʼ��������
    parser_test.read_src(argv);//��ȡ�ļ�
    
    parser_test.TokenOp.line = 1;
    testVM.last_code = testVM.code + 1;//��ʼ��һЩ����

    parser_test.parse();//��ʼ����
	testVM.init_run_VM(&parser_test, argc, argv);//�����������ǰ��׼��

    //��ý���ʱ���
    auto end = std::chrono::high_resolution_clock::now();
    // �������ʱ��
    std::chrono::duration<double> duration = end - start;
    // �������ʱ�䣨����Ϊ��λ��
    std::cout << "[###] Compile successfully! The total compilation time is:" << duration.count() << " s" << std::endl<<std::endl;

    // run VM�����
    return testVM.run_vm();
}