#include "client.hpp"

#include <boost/asio.hpp>

client::client(recvCallback rc, void *d) : onRecv(rc), data(d) { }
