#include "client.hpp"

#include <iostream>

void recwrap(client *c) {
	c->rec();
}

client::client(recvCallback rc, void *d) : onRecv(rc), data(d), running(false),
	sck(io_context), res(io_context) { }
void client::connect(const std::string &ip, const std::string &port) {
	run_iocontext = false;
	asio::connect(sck, res.resolve(ip, port));
	recvthr = new std::thread(recwrap, this);
	running = true;
	listen();
	run_iocontext = true;
	std::cout << "[Networking]: Connected to server (" << ip << " " << port << ")" << std::endl;
}
void client::send(const packet &p) {
#ifdef DEBUG_NETWORKING
	std::cout << "[Debug-Networking]: " << to_str<false>(p) << std::endl;
#endif
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
	while (!run_iocontext) {
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
		if (!running)
			return;
	}
	while (running) {
		io_context.run();
	}
}
asio::streambuf buf;
void client::listen() {
	sck.async_receive(buf.prepare(MAX_PACKET_LEN),
		[this](asio::error_code ec, size_t len){
			if (ec) {
				if (ec.value() == asio::error::operation_aborted)
					std::cout << "[Networking]: Stopped listening from server!" << std::endl;
				else if (ec.value() == asio::error::eof)
					std::cout << "[Networking]: Connection closed by server!" << std::endl;
				else
					std::cout << "[Networking]: " << ec << " " << ec.message() << std::endl;
			} else {
				packet pck(static_cast<const char *>(buf.data().data()), len);
#ifdef DEBUG_NETWORKING
				std::cout << "[Debug-Networking]: " << to_str<true>(pck) << std::endl;
#endif
				onRecv(pck, data);
			}
			if (running)
				listen();
		});
}
