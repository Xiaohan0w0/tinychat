#pragma once
#include "Const.h"
struct SectionInfo
{
    SectionInfo() = default;

    SectionInfo(const SectionInfo&src)
    {
        this->_section_datas = src._section_datas;
    }

    SectionInfo& operator=(const SectionInfo&src)
    {
        if(&src==this)
            return *this;
        this->_section_datas = src._section_datas;
            return *this;
    }

    ~SectionInfo(){_section_datas.clear();}
    
    std::string operator[](const std::string& key)
    {
        if(_section_datas.find(key)==_section_datas.end())
            return "";
        return _section_datas[key];
    }

    std::unordered_map<std::string, std::string> _section_datas;
};
class ConfigMgr
{
public:
    
    ~ConfigMgr();
    SectionInfo operator[](const std::string& section)
    {
        if(_config_map.find(section)==_config_map.end())
        {
            return SectionInfo();
        }
        return _config_map[section];
    }
    ConfigMgr& operator=(const ConfigMgr&src)
    {
        if(&src==this)
            return *this;
        _config_map = src._config_map;
        return *this;
    }
    ConfigMgr(const ConfigMgr&src)
    {
        _config_map = src._config_map;
    }
    static ConfigMgr& GetInstance()
    {
        // 生命周期和进程同步的，可见范围是局部函数的
        static ConfigMgr instance;
        return instance;
    }
private:
    ConfigMgr();
    std::unordered_map<std::string, SectionInfo> _config_map;
};

