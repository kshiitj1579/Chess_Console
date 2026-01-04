#include "Engine.h"
#include "ChessConstants.hpp"

int main()
{
	//Engine engine{ start_position_fen };
	Engine engine{ "rnbqkbnr/pppppppp/8/P7/8/8/PPPPPPPP/RNBQKBNR" };
	engine.step(false, false, 8); 
}