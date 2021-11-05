#ifndef __PACKET_HPP__
#define __PACKET_HPP__

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

constexpr size_t MAX_PACKET_LEN = 1024;

namespace packets {
	enum s_c_pckt_t : uint8_t {
		S_C_DISCONNECT,
		S_C_WALLS,
		S_C_ROBOTS,
		// TODO: some more
	};
}

class packet {
public:
	inline packet(const char *d, size_t l) : len(l - 1) { memcpy(dat, d, l); }

	inline uint8_t     type() const { return dat[0]; }
	inline const char *data() const { return dat + 1; }
	inline size_t      size() const { return len; }
	inline uint8_t    &type()       { return *reinterpret_cast<uint8_t *>(dat); }
	inline char       *data()       { return dat + 1; }
	inline size_t     &size()       { return len; }
protected:
	char dat[MAX_PACKET_LEN];
	size_t len;
};

#endif
