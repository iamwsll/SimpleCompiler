#define _CRT_SECURE_NO_WARNINGS 1
#include "Log.h"
bool gIsSave = false;
const std::string logname = "log.txt";
void SaveFile(const std::string& filename, const std::string& message)
{
    std::ofstream out(filename, std::ios::app);
    if (!out.is_open())
    {
        return;
    }
    out << message;
    out.close();
}

std::string LevelToString(int level)
{
    switch (level)
    {
    case DEBUG:
        return "Debug";
    case INFO:
        return "Info";
    case WARNING:
        return "Warning";
    case ERROR:
        return "Error";
    case FATAL:
        return "Fatal";
    default:
        return "Unknown";
    }
}

std::string GetTimeString()
{
    time_t curr_time = time(nullptr);
    struct tm* format_time = localtime(&curr_time);
    if (format_time == nullptr)
        return "None";
    char time_buffer[1024];
    snprintf(time_buffer, sizeof(time_buffer), "%d-%d-%d %d:%d:%d",
        format_time->tm_year + 1900,
        format_time->tm_mon + 1,
        format_time->tm_mday,
        format_time->tm_hour,
        format_time->tm_min,
        format_time->tm_sec);
    return time_buffer;
}

std::mutex mtx;
// 2. 日志是有格式的
// 日志等级 时间 代码所在的文件名/行数 日志的内容
void LogMessage(std::string filename, int line, bool issave, int level, const char* format, ...)
{

    std::string levelstr = LevelToString(level);
    std::string timestr = GetTimeString();

    char buffer[1024];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buffer, sizeof(buffer), format, arg);
    va_end(arg);

    std::string message = "[" + timestr + "]" + "[" + levelstr + "]" +
        "[" + filename + "]" + "[" + std::to_string(line) + "] " +"\n"+ buffer;
    std::lock_guard<std::mutex> lock(mtx);


    if (!issave)
    {
        std::cout << message;
    }
    else
    {
        SaveFile(logname, message);
    }
}

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


