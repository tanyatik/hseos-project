#pragma once

#include "buffer.h"

#include <vector>

enum MC_COMMAND {
    CMD_UNKNOWN,
    CMD_SET,
    CMD_ADD,
    CMD_GET,
    CMD_DELETE,
    MC_COMMANDS_NUMBER  // always goes last
};

MC_COMMAND CommandName2Code(const std::string& param);

enum MC_RESULT_CODE {
    R_STORED,
    R_NOT_STORED,
    R_EXISTS,
    R_NOT_FOUND,
    R_DELETED
};

std::string ResultCode2String(MC_RESULT_CODE code);

class McValue {
private:
    std::string key_;
    int flags_;
    std::vector<char> data_;

public:
    McValue(std::string key, int flags, const std::vector<char> data_block)
        : key_(key)
        , flags_(flags)
        , data_(data_block)
    {}

    void Serialize(WBuffer* buffer) const;
};

struct McCommand {
    MC_COMMAND command = CMD_UNKNOWN;
    std::string key;
    int32_t flags = 0;
    time_t exp_time = 0;
    std::vector<char> data;

    void Deserialize(RBuffer* buffer);
};

class McResult {
private:
    enum RESULT_TYPE {
        RT_CODE,
        RT_VALUE,
        RT_ERROR
    } type_;

    MC_RESULT_CODE code_;
    std::vector<McValue> values_;
    std::string error_message_;

public:
    McResult(MC_RESULT_CODE result_code)
        : type_(RT_CODE)
        , code_(result_code)
    {}
    McResult(const std::vector<McValue>& values)
        : type_(RT_VALUE)
        , values_(values)
    {}
    McResult(const std::string& error_message)
        : type_(RT_ERROR)
        , error_message_(error_message)
    {}

    void Serialize(WBuffer* buffer) const;
};

/*
std::vector<char> ProcessMessage(const std::vector<char>& input);
*/
McResult ProcessCommand(const McCommand& command);
