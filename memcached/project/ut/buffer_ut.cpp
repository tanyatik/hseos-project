#include "gtest/gtest.h"
#include "buffer.h"

TEST(RBuffer, ReadChar) {
    StringRBuffer rbuf(2, "abc");

    EXPECT_EQ('a', rbuf.ReadChar());
    EXPECT_EQ('b', rbuf.ReadChar());
    EXPECT_EQ('c', rbuf.ReadChar());
    EXPECT_THROW(rbuf.ReadChar(), std::runtime_error);
}

TEST(RBuffer, ReadUint32) {
    StringRBuffer rbuf(4, "123,234232903  132 ");

    EXPECT_EQ(123, rbuf.ReadUint32());
    EXPECT_EQ(',', rbuf.ReadChar());
    EXPECT_EQ(234232903, rbuf.ReadUint32());
    EXPECT_EQ(' ', rbuf.ReadChar());
    EXPECT_EQ(' ', rbuf.ReadChar());
    EXPECT_EQ(132, rbuf.ReadUint32());
}

TEST(RBuffer, ReadField) {
    StringRBuffer rbuf(4, "ba na banana ");
    EXPECT_EQ("ba", rbuf.ReadField(' '));
    EXPECT_EQ(' ', rbuf.ReadChar());
    EXPECT_EQ("na", rbuf.ReadField(' '));
    EXPECT_EQ(' ', rbuf.ReadChar());
    EXPECT_EQ("banana", rbuf.ReadField(' '));
}


TEST(WBuffer, WriteChar) {
    std::string buffer;
    StringWBuffer wbuf(2, &buffer);
    wbuf.WriteChar('a');
    wbuf.Flush();
    EXPECT_EQ("a", buffer);
    wbuf.WriteChar('b');
    wbuf.Flush();
    EXPECT_EQ("ab", buffer);
    wbuf.WriteChar('c');
    wbuf.Flush();
    EXPECT_EQ("abc", buffer);
}

TEST(WBuffer, WriteUint32) {
    std::string buffer;
    StringWBuffer wbuf(4, &buffer);
    wbuf.WriteUint32(132);
    wbuf.Flush();
    EXPECT_EQ("132", buffer);
    wbuf.WriteUint32(99809883);
    wbuf.WriteUint32(456);
    EXPECT_EQ("13299809883", buffer);
    wbuf.Flush();
    EXPECT_EQ("13299809883456", buffer);
}

TEST(WBuffer, WriteField) {
    std::string buffer;
    StringWBuffer wbuf(4, &buffer);
    wbuf.WriteField("banana", ' ');
    wbuf.Flush();
    EXPECT_EQ("banana ", buffer);
    wbuf.WriteField("nana");
    wbuf.Flush();
    EXPECT_EQ("banana nana", buffer);
}

TEST(WBuffer, WriteBytes) {
    std::string buffer;
    StringWBuffer wbuf(4, &buffer);
    wbuf.WriteBytes({'\x62', '\x61', '\x6e', '\x61', '\x6e', '\x61'});
    wbuf.Flush();
    EXPECT_EQ("banana", buffer);
}
