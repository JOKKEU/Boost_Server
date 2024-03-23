#pragma once
#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <locale>
#include <mutex>
#include <deque>


#ifndef _WIN32_WINNT

#define _WIN32_WINNT 0x0601

#endif

namespace asio = boost::asio;
using asio::ip::tcp;
using error_code = boost::system::error_code;

struct ClientSession : std::enable_shared_from_this<ClientSession> {
    ClientSession(tcp::socket s);

    ~ClientSession();

    void start();

private:
    tcp::socket             socket_;
    std::string             buffer_;
    std::thread             console_thread;
    std::deque<std::string> outbox_;

    void do_read_loop();
    void do_write_loop();
    void do_send(std::string mess);
};


struct Server {
    Server(asio::any_io_executor ex, tcp::endpoint ep);

private:
    tcp::acceptor acc;
    void acceptLoop();
};

namespace util_server {
    static std::mutex console_mx;

    std::string readMessage() {
        std::lock_guard lock(console_mx);

        if (std::string message; getline(std::cin, message))
            return message;

        throw std::runtime_error("readMessage(): IO Error");
    }

    tcp::endpoint promptEndpoint() {
        uint16_t    port;
        std::string ipAddress;

        if (std::cout << "IP: " && getline(std::cin, ipAddress) //
            && std::cout << "Port: " && (std::cin >> port).ignore()) {
            if (ipAddress.empty())
                return { {}, port };
            else
                return { asio::ip::make_address(ipAddress), port };
        }
        else {
            throw std::runtime_error("readEndpoint(): Invalid input");
        }
    }

}