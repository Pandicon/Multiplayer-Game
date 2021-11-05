#include "client.hpp"

char recvbuf[MAX_PACKET_LEN];

client::client(recvCallback rc, void *d) : onRecv(rc), data(d), sck(io_context), res(io_context) { }

void client::connect(const std::string &ip, const std::string &port) {
	asio::connect(sck, res.resolve(ip, port));
	asio::async_read(sck, asio::buffer(recvbuf, MAX_PACKET_LEN),
		[this](asio::error_code ec, size_t len){
			if (ec) {
				; // FIXME: do something on error
			} else {
				packet pck(recvbuf, len);
				onRecv(pck, data);
			}
		});
}
void client::send(const packet &p) {
	asio::write(sck, asio::buffer(p.data(), p.size()));
}
