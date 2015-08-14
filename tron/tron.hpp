struct Coord {
	int X, Y;

	static int MaxX;

	Coord (int x, int y) : X{x}, Y{y} {}

	int Index () {
		int result = MaxX*Y + X;
		return result;
	}

	Coord& operator+= (const Coord& a) {
		X += a.X;
		Y += a.Y;
		return *this;
	}

	static const Coord Invalid;

};
