#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <memory>
#include <string>
#include <thread>
#include <asio.hpp>
#include "packet.hpp"

using recvCallback = void (*)(packet &p, void *data);

class client {
public:
	recvCallback onRecv;
	void *data;
	bool running;

	client(recvCallback rc, void *d);

	void connect(const std::string &ip, const std::string &port);
	void send(const packet &p);
	void rec();
	void disconnect();
private:
	std::thread *recvthr;
	asio::io_context io_context;
	asio::ip::tcp::socket sck;
    asio::ip::tcp::resolver res;
	bool run_iocontext;

	void listen();
};

#endif
