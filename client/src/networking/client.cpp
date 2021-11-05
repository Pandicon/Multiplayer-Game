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
	running = true;
	recvthr = new std::thread(recwrap, this);
	listen();
}
void client::send(const packet &p) {
	asio::error_code ec;
	asio::write(sck, asio::buffer(p.rawd(), p.size()), ec);
	if (ec) {
		std::cout << ec << std::endl;
	}
}
void client::disconnect() {
	running = false;
	asio::error_code ec;
	sck.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
	sck.close();
	recvthr->join();
	delete recvthr;
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
				std::cout << ec << std::endl;
			} else {
				packet pck(recvbuf, len);
				onRecv(pck, data);
			}
			if (running)
				listen();
		});
}
