# 实现一个简单的编译器 WQHcompiler V1.0.0
这个项目许多设计是基于[rswier/c4](https://github.com/rswier/c4)的。在原有500行代码的基础上，进行封装和拓展，最终整个编译器大致有3000多行。
你可以在源文件的注释当中得到这个编译器具体实现的各种细节，包括虚拟机的指令架构等。
## 编译器特征
- OnePass的设计，只对源代码进行一次解析，直接生成虚拟机代码。
- 支持g++/gcc MVSC(即VS2022系列IDE) 32位和64位的机器，具有良好的跨平台特性。
- 指令集完全与[rswier/c4](https://github.com/rswier/c4)相同，
- 优异的报错与日志功能：在程序运行的路径下，会生成"log.txt"，里面会详细记录程序运行的情况和报错信息。这使得编译器不会轻易因为非法行为而崩溃。
- debug模式：你可以开启debug模式，这样会自动在程序运行路径下看到"compile_record" 和 "running_code_record" 两份文件，分别记录了编译时源代码中每条语句对应生成的中间代码，以及虚拟机将要执行的代码。

实现上几乎是c语言的子集。你可以把它当做一个基础版的c语言编译器进行使用。
## 目前主要支持的特性：
- 基本的数据类型 int char 指针
- 自定义类型 enum(匿名 命名) struct union
- 控制流语句：if-else while for do-while goto break continue
- 常见的运算符 , = ?: || && | ^ & == != < > <= >= << >> + - * / % ++ -- [] () . ->
- 支持自定义函数，包括函数的前向声明。
- 支持以下库函数：printf,scanf,malloc,free,memset,memcmp,memcpy

## 目前的缺点（与c语言相比）：
- 变量定义时不能初始化，也就是说，你必须 int var_name;name = 1;才可以
- 一个函数内部局部变量的定义必须集中放在函数开头。
- struct union 支持有限，不能当做函数返回值 （但是指针可以） 不能嵌套声明 不能匿名 不能使用赋值（但可以memcpy）等等
- 库函数无法调用、不支持多文件编译。
## 使用方法
- 懒人版：在vs2022环境下：你可以下载文件夹，找到文件夹中的"test.txt"中写你需要的代码  然后通过vs2022打开WQHcompiler.sln，点击运行即可。你可以找到新生成的log.txt文件来查看你程序的日志。
- 命令行版（方便启动debug模式）你可以把所有的.h .cpp 文件进行编译，生成WQHcompiler可执行文件。在命令行中使用./WQHcompiler 文件名     就能对文件进行编译。
- 启动debug模式：在命令行下，直接通过./WQHcomiler -d 文件名   就能对文件进行编译。
    如果你是使用懒人版的方式进行启动，请你在VM.h中把debug的默认初始值改成1，就能以debug模式启动。
## 启发与参考
- 项目基于[rswier/c4](https://github.com/rswier/c4)，采用了相同的指令集架构，语言风格，以及相似的内存管理方式（统一使用指针）
- [Bilibili-700行手写编译器](https://www.bilibili.com/video/BV1Kf4y1V783) 一个基于c4的700行编译器讲解与设计视频，学会了基础的编译原理知识，便于对c4的解读。
- 有关的代码参考与设计灵感来源：[手把手教你构建 C 语言编译器](https://lotabout.me/2015/write-a-C-interpreter-0/)系列，[JustAToyCCompiler](https://github.com/tch0/JustAToyCCompiler)，[lotabout/write-a-C-interpreter](https://github.com/lotabout/write-a-C-interpreter)，
