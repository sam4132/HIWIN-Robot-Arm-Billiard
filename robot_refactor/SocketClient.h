#pragma once

#include <WinSock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

namespace hiwinrobot {

class SocketClient {
public:
    SocketClient(const std::string& ip, int port)
        : ip_(ip), port_(port), client_socket_(INVALID_SOCKET) {
        WSAStartup(MAKEWORD(1, 1), &wsa_data_);
    }

    ~SocketClient() {
        disconnect();
        WSACleanup();
    }

    bool connect() {
        client_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket_ == INVALID_SOCKET) {
            return false;
        }

        SOCKADDR_IN addrSrv{};
        addrSrv.sin_addr.S_un.S_addr = inet_addr(ip_.c_str());
        addrSrv.sin_family = AF_INET;
        addrSrv.sin_port = htons(port_);

        return ::connect(client_socket_, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == 0;
    }

    bool send(const std::string& message) {
        if (client_socket_ == INVALID_SOCKET) {
            return false;
        }
        return ::send(client_socket_, message.c_str(), static_cast<int>(message.size()) + 1, 0) >= 0;
    }

    SOCKET socket_handle() const {
        return client_socket_;
    }

    void disconnect() {
        if (client_socket_ != INVALID_SOCKET) {
            closesocket(client_socket_);
            client_socket_ = INVALID_SOCKET;
        }
    }

private:
    std::string ip_;
    int port_;
    SOCKET client_socket_;
    WSADATA wsa_data_{};
};

} // namespace hiwinrobot
