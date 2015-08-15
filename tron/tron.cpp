#include "tron.hpp"

int Coord::MaxX = 0;

const Coord Coord::Invalid {-1,-1};
const Coord Coord::Directions [4] { {-1,0}, {0,1}, {1,0}, {0,-1} };
