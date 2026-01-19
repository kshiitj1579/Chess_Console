#include "Engine.h"

Engine::Engine()
	: m_moveGen(), m_state(), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth(), m_depthSearched(), m_stopSearch(), m_timeCheckCount(), 
	m_bestMoveFinal(), m_moveSource() {}

Engine::Engine(std::string_view fen)
	: m_moveGen(), m_state(State::parse_fen(fen)), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth(), m_depthSearched(), m_stopSearch(), 
	m_timeCheckCount(), m_bestMoveFinal(), m_moveSource() {}
    //giving arguments 

void Engine::setState(std::string_view fen)
{
	m_state = State::parse_fen(fen);
}

int Engine::evaluate(const State& state)
{
	m_evaluations++; //no of evaluations made 

	int evaluation{};

	for (std::size_t piece{}; piece < PIECE_COUNT; piece++)
	{
		BitBoard piece_board = state.positions()[piece];

		while (piece_board.board())
		{
			const std::size_t square = piece_board.find_1lsb();
			const BitBoard attack = m_moveGen.getPieceAttack(piece, square, state);

			evaluation += piece_value[piece];
			evaluation += static_cast<int>(attack.bitCount());
            //calculating score
			piece_board.reset(square); //removing the least set bit 
		}
	}

	return evaluation;
}

bool Engine::kingInCheck(const State& state) const
{
	if (state.whiteToMove()) //our move 
	{
		const std::size_t king_square = state.positions()[KING].find_1lsb();

		if (king_square == SIZE_MAX)
		{
			return true;
		}
		else
		{
			return m_moveGen.isSquareAttacked(state, king_square, Color::WHITE);
		}
	}
	else
	{ //black side to move 
		const std::size_t king_square = state.positions()[BKING].find_1lsb();

		if (king_square == SIZE_MAX) //king in check 
		{
			return true;
		}
		else
		{
			return m_moveGen.isSquareAttacked(state, king_square, Color::BLACK);
		}
	}
}
