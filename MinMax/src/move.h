#pragma once

class Move {
public:
	uint8_t j:7;

	const static Move end;
	const static Move skip;
	const static Move any;

	Move() { }
	Move(uint8_t _val) { j = _val; }
	Move(uint8_t Y, uint8_t X, uint8_t y, uint8_t x) {
		j = Y*27 + X*9 + y*3 + x;
	}

	inline Move operator++ (int)
	{ j++; return *this; }

	inline Move operator-- (int)
	{ j--; return *this; }

	bool operator==(const Move &rhs) const {
		return rhs.j == j;
	}

	bool operator!=(const Move &rhs) const {
		return rhs.j != j;
	}

	// getters
	inline int Y() const { return j/27; }
	inline int X() const { return (j%27)/9; }
	inline int y() const { return (j%9)/3; }
	inline int x() const { return (j%3); }

	inline int YX() const { return j/9; }
	inline int yx() const { return j%9; }

	void print() const {
		if (*this == Move::end)
			std::cerr << "end" << std::endl;
		else if (*this == Move::skip)
			std::cerr << "skip" << std::endl;
		else if (*this == Move::any)
			std::cerr << "any" << std::endl;
		else
			std::cerr << Y() << " " << X() << " " << y() << " " << x() << std::endl;
	}

} __attribute__ ((packed));

const Move Move::end = Move(2, 2, 2, 2+1);
const Move Move::skip = Move(2, 2, 2, 2+2);
const Move Move::any = Move(2, 2, 2, 2+3);

typedef struct MoveValued {
	Move move;
	score_t value;
} MoveValued;
