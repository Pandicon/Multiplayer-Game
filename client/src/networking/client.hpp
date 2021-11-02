#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include <string>
#include "packet.hpp"

using recvCallback = void (*)(packet &p, void *data);

class client {
public:
	recvCallback onRecv;
	void *data;

	client(recvCallback rc, void *d);

	void connect(const std::string &ip, const std::string &port);
	void send(const packet &p);
private:
};

#endif
