#include "../headers/Server.h"


ClientSession::ClientSession(tcp::socket s) : socket_(std::move(s)) {
    std::cout << "ClientSession created for " << socket_.remote_endpoint() << std::endl;
}

ClientSession::~ClientSession() {
    post(socket_.get_executor(), [self = shared_from_this(), this] { socket_.cancel(); });
    std::cout << "ClientSession closing" << std::endl;
    console_thread.join();
}

void ClientSession::start() {
    assert(!console_thread.joinable());

    do_read_loop();
    console_thread = std::thread([self = shared_from_this()] {
        for (;;) {
            post(self->socket_.get_executor(),
                [self, m = util_server::readMessage()]() mutable { self->do_send(std::move(m)); });
        }
        });
}

void ClientSession::do_read_loop() {
    async_read_until( //
        socket_, asio::dynamic_buffer(buffer_), "\n",
        [this, self = shared_from_this()](error_code ec, size_t bytes) {
            if (!ec) {
                auto message = std::string_view(buffer_).substr(0, bytes - 1); // exclude the delimiter
                std::cout << "Client message: " << std::quoted(message) << std::endl;

                // Consume the data that was read
                buffer_.erase(0, bytes);

                // Handle the next message
                do_read_loop();
            }
            else {
                std::cout << "do_read_loop: " << ec.message() << std::endl;
            }
        });
}

void ClientSession::do_write_loop() {
    if (outbox_.empty())
        return;

    async_write(socket_, asio::buffer(outbox_.front()),
        [this, self = shared_from_this()](error_code ec, size_t /*bytes*/) {
            if (!ec) {
                outbox_.pop_front();
                do_write_loop();
            }
            else {
                std::cout << "do_write_loop: " << ec.message() << std::endl;
            }
        });
}

void ClientSession::do_send(std::string mess) {
    if (!mess.ends_with('\n'))
        mess += '\n';
    outbox_.push_back(std::move(mess));
    if (outbox_.size() == 1) {
        do_write_loop();
    }
}



Server::Server(asio::any_io_executor ex, tcp::endpoint ep) : acc(ex, ep) {
    try {
        std::cout << "Server starting at " << ep << std::endl;
        acceptLoop();
    }
    catch (std::exception const& e) {
        std::cerr << "Error constructor MainServer(): " << e.what() << std::endl;
    }
}
void Server::acceptLoop() {
    acc.async_accept(make_strand(acc.get_executor()), [this](error_code ec, tcp::socket s) {
        if (!ec) {
            std::make_shared<ClientSession>(std::move(s))->start();
            acceptLoop();
        }
        else {
            std::cout << "acceptLoop: " << ec.message() << std::endl;
        }
        });
}

int main()
try {
    setlocale(LC_ALL, "Rus");
    auto ep = util_server::promptEndpoint();

#ifdef MULTI_THREADING
    asio::thread_pool ioc(1);
    Server            server(ioc.get_executor(), result);
    ioc.join();
#else
    asio::io_context ioc(1);
    Server           server(ioc.get_executor(), ep);
    ioc.run();
#endif
}
catch (std::exception const& e) {
    std::cerr << "main(): " << e.what() << std::endl;
}