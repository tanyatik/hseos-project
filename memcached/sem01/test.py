import socket
import sys

port = int(sys.argv[1])

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", port))

msg = "hello!"

s.send("hello!")
res = s.recv(len(msg))

assert msg == ""
