#include "gtest/gtest.h"
#include "protocol.h"


TEST(Protocol, McValueSerialize) {
    McValue value("user_123", 1445128601, std::vector<char>({'1', '2', '3', '4', '5', 'a'}));
    std::string result;
    StringWBuffer buffer(8, &result);
    value.Serialize(&buffer);
    buffer.Flush();

    ASSERT_EQ(result, "user_123 1445128601 6\r\n12345a\r\n");
}

TEST(Protocol, McCommandDeserializeSet) {
    McCommand setCommand;
    std::string input = "set user_123 888 1445128601 10\r\nnanananana\r\n";
    StringRBuffer buffer(8, input);
    setCommand.Deserialize(&buffer);

    ASSERT_EQ(setCommand.command, CMD_SET);
    ASSERT_EQ(setCommand.keys[0], "user_123");
    ASSERT_EQ(setCommand.flags, 888);
    ASSERT_EQ(setCommand.exp_time, 1445128601);
    ASSERT_EQ(setCommand.data, std::vector<char>({'n', 'a', 'n', 'a', 'n', 'a', 'n', 'a', 'n', 'a'}));
}

TEST(Protocol, McCommandDeserializeAdd) {
    McCommand addCommand;
    std::string input = "add user_789 123423424 1445428821 10\r\nabcdefghij\r\n";
    StringRBuffer buffer(8, input);
    addCommand.Deserialize(&buffer);

    ASSERT_EQ(addCommand.command, CMD_ADD);
}

TEST(Protocol, McCommandDeserializeGet) {
    McCommand getCommand;
    std::string input = "get user_123\r\n";
    StringRBuffer buffer(8, input);
    getCommand.Deserialize(&buffer);

    ASSERT_EQ(getCommand.command, CMD_GET);
    ASSERT_EQ(getCommand.keys[0], "user_123");
}

TEST(Protocol, McCommandDeserializeGetMultipleKeys) {
    McCommand getCommand;
    std::string input = "get user_123 user_789\r\n";
    StringRBuffer buffer(8, input);
    getCommand.Deserialize(&buffer);

    ASSERT_EQ(getCommand.command, CMD_GET);
    ASSERT_EQ(getCommand.keys[0], "user_123");
    ASSERT_EQ(getCommand.keys[1], "user_789");
}

TEST(Protocol, McCommandDeserializeDelete) {
    McCommand deleteCommand;
    std::string input = "delete user_123";
    StringRBuffer buffer(8, input);
    deleteCommand.Deserialize(&buffer);

    ASSERT_EQ(deleteCommand.command, CMD_DELETE);
}

TEST(Protocol, McCommandDeserializeError) {
    McCommand errCommand;
    {
        std::string input = "asdfwerw";
        StringRBuffer buffer(8, input);
        ASSERT_THROW(errCommand.Deserialize(&buffer), std::runtime_error);
    }
    {
        std::string input = "set ajlkjl";
        StringRBuffer buffer(8, input);
        ASSERT_THROW(errCommand.Deserialize(&buffer), std::runtime_error);
    }
    {
        std::string input = "set ajlkjl 99809jdf";
        StringRBuffer buffer(8, input);
        ASSERT_THROW(errCommand.Deserialize(&buffer), std::runtime_error);
    }
    {
        std::string input = "set user_123 888 1445128601 1024\r\n12\r\n";
        StringRBuffer buffer(8, input);
        ASSERT_THROW(errCommand.Deserialize(&buffer), std::runtime_error);
    }
}

TEST(Protocol, McResultSerialize) {
    {
        McResult codeResult(R_DELETED);
        std::string result;
        StringWBuffer buffer(3, &result);
        codeResult.Serialize(&buffer);
        buffer.Flush();
        ASSERT_EQ(result, "DELETED\r\n");
    }

    {
        McResult errResult("Length of key is too long");
        std::string result;
        StringWBuffer buffer(3, &result);
        errResult.Serialize(&buffer);
        buffer.Flush();
        ASSERT_EQ(result, "ERROR Length of key is too long\r\n");
    }

    {
        McValue value1("user_123", 1445128601, std::vector<char>({'1', '2', '3', '4', '5', 'a'}));
        McValue value2("user_876", 1446128601, std::vector<char>({'6', '7', '8', '9', '0', 'z'}));

        McResult valResult(std::vector<McValue>({value1, value2}));
        std::string result;
        StringWBuffer buffer(3, &result);
        valResult.Serialize(&buffer);
        buffer.Flush();

        ASSERT_EQ(result, "VALUE user_123 1445128601 6\r\n12345a\r\nVALUE user_876 1446128601 6\r\n67890z\r\nEND\r\n");
    }
}
