#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

enum typelog
{
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};

struct structlog
{
    typelog level = WARN;
};

extern structlog LOGCFG;

class LOG
{
public:
    LOG(typelog type)
    {
        msglevel = type;
        (*this) << '[' << get_label(type) << ']';
    }
    ~LOG()
    {
        if(opened)
            std::cout << std::endl;
        opened = false;
    }
    template<class T>
    LOG& operator<<(const T& msg)
    {
        if(msglevel >= LOGCFG.level)
        {
            std::cout << msg;
            opened = true;
        }
        return *this;
    }
private:
    bool opened = false;
    typelog msglevel = DEBUG;
    inline std::string get_label(typelog type)
    {
        if (type == DEBUG) return "DEBUG";
        else if(type == INFO) return "INFO";
        else if(type == WARN) return "WARN";
        else if(type == ERROR) return "ERROR";
        else return "";
    }
};


#endif