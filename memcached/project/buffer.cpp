#include <stdexcept>

#include "buffer.h"

char RBuffer::ReadChar() {
    if (pos_ == end_) {
        ReadMore();
    }
    return buffer_[pos_++];
}

void RBuffer::ReadCharCheck(char check) {
    if (ReadChar() != check) {
        throw std::runtime_error("Invalid character in input " + std::to_string(buffer_[pos_]) + " should be " + std::to_string(check));
    }
}

uint32_t RBuffer::ReadUint32() {
    uint32_t value = 0;
    char ch;
    while (isdigit(ch = ReadChar())) {
        int digit = ch - '0';
        value *= 10;
        value += digit;
    }
    --pos_;

    return value;
}

std::string RBuffer::ReadField(char sep) {
    std::string field;
    char ch;
    while ((ch = ReadChar()) != sep) {
        field.push_back(ch);
    }
    --pos_;
    return field;
}

std::vector<char> RBuffer::ReadBytes(size_t bytes_num) {
    std::vector<char> bytes(bytes_num);
    for (size_t i = 0; i < bytes_num; ++i) {
        bytes[i] = ReadChar();
    }

    return bytes;
}

void WBuffer::WriteChar(char ch) {
    buffer_[pos_++] = ch;
    if (pos_ == buffer_.size()) {
        Flush();
    }
}

void WBuffer::WriteUint32(uint32_t v) {
    char buf[32];
    int chw;
    snprintf(buf, 32, "%u%n", v, &chw);
    for (int i = 0; i < chw; ++i) {
        WriteChar(buf[i]);
    }
}

void WBuffer::WriteField(std::string field) {
    for (size_t i = 0; i < field.size(); ++i) {
        WriteChar(field[i]);
    }
}

void WBuffer::WriteField(std::string field, char sep) {
    WriteField(field);
    WriteChar(sep);
}

void WBuffer::WriteBytes(const std::vector<char>& buffer) {
    for (char c : buffer) {
        WriteChar(c);
    }
}


StringRBuffer::StringRBuffer(size_t buf_size, const std::string& s)
        : RBuffer(buf_size)
        , string_(s)
        , string_iter_(string_.begin()) {
    ReadMore();
}

void StringRBuffer::ReadMore() {
    pos_ = 0;

    if (string_iter_ == string_.end()) {
        throw std::runtime_error("Failed to read more from StringRBuffer");
    }

    int to_copy = std::min(std::distance(string_iter_, string_.end()), (long int) buffer_.size());
    std::copy(string_iter_, string_iter_ + to_copy, buffer_.begin());
    string_iter_ += to_copy;
    end_ = to_copy;
}

void StringWBuffer::Flush() {
    string_->append(buffer_.begin(), buffer_.begin() + pos_);
    pos_ = 0;
}

