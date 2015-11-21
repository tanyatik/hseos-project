#pragma once

#include <cstdio>
#include <string>
#include <vector>

class RBuffer {
public:
    explicit RBuffer(size_t buffer_size)  // buffer_size -- internal size of buffer, if larger -- rare reads from socket
        : buffer_(buffer_size)
        , pos_(0)
        , end_(buffer_size)
    {}
    virtual ~RBuffer() {}

    char ReadChar();
    void ReadCharCheck(char check);
    uint32_t ReadUint32();

    std::string ReadField(char sep);  // reads all data before 'sep' into string
    std::vector<char> ReadBytes(size_t bytes_num);

protected:
    virtual void ReadMore() = 0;

    std::vector<char> buffer_;
    size_t pos_;
    size_t end_;
};


class WBuffer {
public:
    explicit WBuffer(size_t buffer_size)  // buffer_size -- internal size of buffer, if larger -- rare writes to socket
        : buffer_(buffer_size)
        , pos_(0)
    {}
    virtual ~WBuffer() {}

    void WriteChar(char);  // writes single character to buffer. Flushes if necessary
    void WriteUint32(uint32_t);  // Writes 32-bit integer to buffer in text format

    void WriteField(std::string field, char sep); // writes 'field' and 'sep'
    void WriteField(std::string field);  // writes 'field'
    void WriteBytes(const std::vector<char>& buffer); // writes raw data

    virtual void Flush() = 0;  // flushes all data from buffer to output stream

protected:
    std::vector<char> buffer_;
    size_t pos_;
};


class SocketRBuffer : public RBuffer {
public:
    SocketRBuffer(size_t buf_size, int fd);
    ~SocketRBuffer();
    bool Closed() const { return closed_; }
    void Clear() { pos_ = 0; end_ = 0; }

protected:
    virtual void ReadMore() override;

private:
    int fd_;
    bool closed_;
};


class SocketWBuffer : public WBuffer {
public:
    SocketWBuffer(size_t buf_size, int fd) : WBuffer(buf_size), fd_(fd), closed_(false) {}
    ~SocketWBuffer();
    bool Closed() const { return closed_; }
    void Clear() { pos_ = 0; }

    virtual void Flush() override;

private:
    int fd_;
    bool closed_;
};


class StringRBuffer : public RBuffer {
public:
    StringRBuffer(size_t buf_size, const std::string& s);

protected:
    virtual void ReadMore() override;

private:
    std::string string_;
    std::string::iterator string_iter_;
};


class StringWBuffer : public WBuffer {
public:
    StringWBuffer(size_t buf_size, std::string* s) : WBuffer(buf_size), string_(s) {}

    virtual void Flush() override;

private:
    std::string* string_;
};

