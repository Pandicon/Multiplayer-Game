#include "client.hpp"

#include <iostream>

char recvbuf[MAX_PACKET_LEN];

void recwrap(client *c) {
	c->rec();
}

client::client(recvCallback rc, void *d) : onRecv(rc), data(d), running(false),
	sck(io_context), res(io_context) { }
void client::connect(const std::string &ip, const std::string &port) {
	asio::connect(sck, res.resolve(ip, port));
	std::cout << "[Networking]: Connected to server (" << ip << " " << port << ")" << std::endl;
	running = true;
	recvthr = new std::thread(recwrap, this);
	listen();
	std::cout << "[Networking]: Started listening from server!" << std::endl;
}
void client::send(const packet &p) {
	asio::error_code ec;
	asio::write(sck, asio::buffer(p.rawd(), p.size()+1), ec);
	if (ec) {
		std::cout << "[Networking]: " << ec << " " << ec.message() << std::endl;
	}
}
void client::disconnect() {
	std::cout << "[Networking]: Disconnecting!" << std::endl;
	running = false;
	asio::error_code ec;
	sck.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
	sck.close();
	recvthr->join();
	delete recvthr;
	std::cout << "[Networking]: Disconnected!" << std::endl;
}
void client::rec() {
	while (running) {
		io_context.run();
	}
}
void client::listen() {
	sck.async_read_some(asio::buffer(recvbuf, MAX_PACKET_LEN),
		[this](asio::error_code ec, size_t len){
			if (ec) {
				if (ec.value() == asio::error::operation_aborted)
					std::cout << "[Networking]: Stopped listening from server!" << std::endl;
				else if (ec.value() == asio::error::eof)
					std::cout << "[Networking]: Connection closed by server!" << std::endl;
				else
					std::cout << "[Networking]: " << ec << " " << ec.message() << std::endl;
			} else {
				packet pck(recvbuf, len);
				onRecv(pck, data);
			}
			if (running)
				listen();
		});
}
