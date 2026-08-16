import socket


class SocketClient:
    def __init__(self, host: str, port: int, buffer_size: int = 1024):
        self.host = host
        self.port = port
        self.buffer_size = buffer_size
        self.sock = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        return self.sock

    def send_message(self, message: str):
        if self.sock is None:
            raise RuntimeError("Socket is not connected.")
        self.sock.sendall(message.encode("ascii"))

    def close(self):
        if self.sock is not None:
            self.sock.close()
            self.sock = None


class SocketServer:
    def __init__(self, host: str, port: int, buffer_size: int = 1024):
        self.host = host
        self.port = port
        self.buffer_size = buffer_size
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connection = None

    def start(self):
        self.server.bind((self.host, self.port))
        self.server.listen(0)
        self.connection, _ = self.server.accept()
        return self.connection

    def receive(self):
        if self.connection is None:
            raise RuntimeError("Socket server is not started.")
        data = self.connection.recv(self.buffer_size)
        if data:
            return data.decode("ascii")
        return ""

    def send_message(self, message: str):
        if self.connection is None:
            raise RuntimeError("Socket server is not connected.")
        self.connection.sendall(message.encode("ascii"))

    def close(self):
        if self.connection is not None:
            self.connection.close()
            self.connection = None
        if self.server is not None:
            self.server.close()
            self.server = None
