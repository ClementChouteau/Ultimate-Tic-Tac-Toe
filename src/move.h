#ifndef MOVE_H
#define MOVE_H

class Move {
public:
	uint8_t j;

	const static Move end;
	const static Move skip;
	const static Move any;

	Move() { }
	Move(uint8_t _val) { j = _val; }
	Move(uint8_t Y, uint8_t X, uint8_t y, uint8_t x) {
		j = Y*27 + X*9 + y*3 + x; // 0..80
	}

	inline Move operator++ (int) //(int means that is is move++ operator and not ++move)
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

} __attribute__ ((packed));

const Move Move::end = Move(2, 2, 2, 2+1);
const Move Move::skip = Move(2, 2, 2, 2+2);
const Move Move::any = Move(2, 2, 2, 2+3);

typedef struct MoveValued {
	Move move;
	score_t value;
} MoveValued;

#endif // MOVE_H
