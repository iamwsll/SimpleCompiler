#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include<vector>
#include <ctime>
#include <cstdarg>
#include <mutex>
extern bool gIsSave;
extern const std::string logname;

// 1. 日志是有等级的
enum Level
{
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

void SaveFile(const std::string& filename, const std::string& message);

std::string LevelToString(int level);
std::string GetTimeString();

// 2. 日志是有格式的
// 日志等级 时间 代码所在的文件名/行数 日志的内容
void LogMessage(std::string filename, int line, bool issave, int level, const char* format, ...);
// C99新特性__VA_ARGS__
#define LOG(level, format, ...)                                                \
    do                                                                         \
    {                                                                          \
        LogMessage(__FILE__, __LINE__, gIsSave, level, format, ##__VA_ARGS__); \
    } while (0)

#define EnableFile()    \
    do                  \
    {                   \
        gIsSave = true; \
    } while (0)
#define EnableScreen()   \
    do                   \
    {                    \
        gIsSave = false; \
    } while (0)


