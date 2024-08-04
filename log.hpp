#ifndef INCLUDE_LOG_HPP_
#define INCLUDE_LOG_HPP_

#include "json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ostream>
#include <unordered_map>
#include <forward_list>

enum class LOGLEVEL{
    INFO,
    DEBUG,
    ERROR,
};

struct LogFormat{
    LogFormat(const char* color){
        text_color = color;
    }
    const char* text_color;
};

// 定义颜色宏
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"


const std::forward_list<std::string> detail_list ={"date","time","file","function","line"} ;
const std::vector<LogFormat> log_format_list= {LogFormat(WHITE),LogFormat(GREEN),LogFormat(RED)};
using json = nlohmann::json;
class Logger {
public:
    // 构造函数，默认输出到 std::cout，并设定 logger 名称
    Logger(const std::string& name) : outStreamPtr(&std::cout), loggerName(name) {}
    
    void GetConfig(const std::string& config_file){
        std::ifstream file;
        file.open(config_file);
        json j;
        try {
            file >> j;
        } catch (json::parse_error& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            return ;
        }
        level = j["level"];
        for(const auto& detail:j["details"]){
            detail_map[detail] = true;
        }
    }
    
    // 设置日志输出到某个 std::ostream 对象
    void SetOutput(std::ostream& os) {
        outStreamPtr = &os;
    }

    // 实际负责输出日志的内部类
    class LogEntry {
    public:
        LogEntry(Logger& logger,LOGLEVEL level, std::unordered_map<std::string,std::string>& map){
            outStreamPtr=logger.outStreamPtr;
            is_outptut = logger.level<=level;
            if (outStreamPtr && is_outptut) {
                std::string detail_str;
                detail_str +=("["+logger.loggerName+"]:");
                auto& detail_map = logger.detail_map;
                detail_str += "(";
                for(auto& elem:detail_list){
                    if(detail_map[elem]){
                        detail_str+=map[elem]+"--";
                    }
                }
                detail_str.pop_back();
                detail_str.pop_back();
                detail_str+=")";
                *outStreamPtr<<log_format_list[static_cast<uint8_t>(level)].text_color<<detail_str <<" ";
            }
        }

        // 模板重载操作符 << 用于支持各种数据类型
        template <typename T>
        LogEntry& operator<<(const T& value) {
            if (outStreamPtr && is_outptut) {
                *outStreamPtr << value;
            }
            return *this;
        }

        // 处理流操纵符，如 std::endl
        typedef std::ostream& (*StreamManipulator)(std::ostream&);
        LogEntry& operator<<(StreamManipulator manip) {
            if (outStreamPtr && is_outptut) {
                manip(*outStreamPtr);
            }
            return *this;
        }

        // 析构时输出换行，确保日志条目完整性
        ~LogEntry() {
            if (outStreamPtr && is_outptut) {
                *outStreamPtr << RESET <<std::endl;
            }
        }

    private:
        std::ostream* outStreamPtr;     // 输出流指针
        bool is_outptut;
    };

    // 创建一个LogEntry对象
    LogEntry entry(LOGLEVEL level,const std::string& date,const std::string& time,const std::string& file, const std::string& function, int line) {
        std::unordered_map<std::string,std::string> map;
        map["date"] = date;
        map["time"] = time;
        map["file"] = file;
        map["function"] = function;
        map["line"] = std::to_string(line);
        return LogEntry(*this,level,map);
    }

private:
    std::ostream* outStreamPtr; // 指向当前输出流的指针
    std::string loggerName;     // Logger 的名称
    LOGLEVEL level; //打印要求最低等级
    std::unordered_map<std::string,bool> detail_map;//是否打印某个细节
};

// 定义一个宏，方便日志输出时带上文件名、函数名和行号
#define LOG(logger,level) (logger).entry(level,__DATE__,__TIME__,__FILE__, __FUNCTION__, __LINE__) 

#endif