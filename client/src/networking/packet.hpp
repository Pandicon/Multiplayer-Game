#ifndef __PACKET_HPP__
#define __PACKET_HPP__

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <sstream>
#include <string>

constexpr size_t MAX_PACKET_LEN = 1024;

namespace packets {
	enum s_c_pckt_t : uint8_t {
		S_C_DISCONNECT,
		S_C_WALLS,
		S_C_ROBOTS,
		S_C_TARGET,
		S_C_FOUND_PATH=16,
		S_C_TIMEOUT,
		S_C_MOVE,
		S_C_UNDO_MOVE,
		S_C_ROBOT_RESET,
		S_C_MESSAGE=255,
	};
	inline std::string s_c_to_str(const s_c_pckt_t t) {
		switch (t) {
		case S_C_DISCONNECT: return "disconnect";
		case S_C_WALLS: return "walls";
		case S_C_ROBOTS: return "robots";
		case S_C_TARGET: return "target";
		case S_C_FOUND_PATH: return "found_path";
		case S_C_TIMEOUT: return "timeout";
		case S_C_MOVE: return "move";
		case S_C_UNDO_MOVE: return "undo_move";
		case S_C_ROBOT_RESET: return "robot_reset";
		case S_C_MESSAGE: return "message";
		default: return "?(" + std::to_string(t) + ")";
		}
	}
	enum c_s_pckt_t : uint8_t {
		C_S_DISCONNECT,
		C_S_NICKNAME,
		C_S_FOUND_PATH,
		C_S_MESSAGE=255,
	};
	inline std::string c_s_to_str(const c_s_pckt_t t) {
		switch (t) {
		case C_S_DISCONNECT: return "disconnect";
		case C_S_NICKNAME: return "nickname";
		case C_S_FOUND_PATH: return "found_path";
		case C_S_MESSAGE: return "message";
		default: return "?(" + std::to_string(t) + ")";
		}
	}
}

class packet {
public:
	inline packet(const char *d, size_t l) : len(l - 1) { memcpy(dat, d, l); }
	inline packet(uint8_t t, const char *d, size_t l) : len(l) { dat[0] = t; memcpy(dat+1, d, l); }

	inline uint8_t     type() const { return dat[0]; }
	inline const char *data() const { return dat + 1; }
	inline const char *rawd() const { return dat; }
	inline size_t      size() const { return len; }
	inline uint8_t    &type()       { return *reinterpret_cast<uint8_t *>(dat); }
	inline char       *data()       { return dat + 1; }
	inline size_t     &size()       { return len; }
protected:
	char dat[MAX_PACKET_LEN];
	size_t len;
};

template<bool recvpacket>
inline std::string to_str(const packet &p) {
	std::stringstream ss;
	if constexpr (recvpacket) {
		ss << "recv:[type:" << packets::s_c_to_str(static_cast<packets::s_c_pckt_t>(p.type()));
	} else {
		ss << "send:[type:" << packets::c_s_to_str(static_cast<packets::c_s_pckt_t>(p.type()));
	}
	ss << ";data:{";
	ss << std::hex;
	for (size_t i = 0; i < p.size(); ++i) {
		if (i != 0)
			ss << ' ';
		char c = p.data()[i];
		ss << std::setfill('0') << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
		ss << '(' << c << ')';
	}
	ss << "}]";
	return ss.str();
}

#endif
