import socket
import sys

port = 3249


def check_echo(msg):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", port))

    s.send(msg)
    s.shutdown(socket.SHUT_WR)  # break reading cycle in echo server
    res = s.recv(len(msg))
    assert msg == res


def test_hello():
    msg = "hello!"
    check_echo(msg)


def test_one_byte():
    msg = "1"
    check_echo(msg)


def test_big_message():
    msg = "BIGDATA" * 1000
    check_echo(msg)


if __name__ == "__main__":
    port = int(sys.argv[1])
    test_hello()
    test_one_byte()
    test_big_message()
