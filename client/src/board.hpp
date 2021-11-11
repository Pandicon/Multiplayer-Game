#ifndef __BOARD_HPP__
#define __BOARD_HPP__

constexpr unsigned int BOARD_SIZE = 16;
constexpr unsigned int BOARD_TILES = BOARD_SIZE * BOARD_SIZE;

namespace dirs {
	enum dirs_t {
		UP,
		RIGHT,
		DOWN,
		LEFT
	};
}

class board {
public:
	bool walls[16][16][4];
};

#endif
