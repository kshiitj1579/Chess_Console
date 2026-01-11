#pragma once

#include "MoveGen.h"
#include "BitBoard.h"
#include "ChessConstants.hpp"
#include <string>
#include <string_view>
#include <cstddef>
#include <algorithm>
#include <chrono>

using namespace std::literals::string_literals;


class Engine
{
private:
	MoveGen m_moveGen;

	State m_state;
	Move m_bestMove;
	Move m_bestMoveFinal;

	std::uint32_t m_depth;

	bool m_stopSearch;
	int m_timeCheckCount;
	std::chrono::steady_clock::time_point m_searchStartTime;

	std::uint32_t m_depthSearched;
	std::uint32_t m_evaluations;
	std::uint32_t m_nodes;
	std::uint32_t m_prunes;
	std::uint32_t m_mates;
	std::size_t m_moveSource;
	std::chrono::duration<double> m_seconds;
}