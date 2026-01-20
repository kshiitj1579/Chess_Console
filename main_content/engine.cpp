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

std::size_t Engine::squareToIndex(std::string_view square)
{
	const std::size_t rank{ 7 - static_cast<std::size_t>(square[1] - '1') }; // X-axis
	const std::size_t file{ static_cast<std::size_t>(square[0] - 'a') }; //Y-axis

	return rank * RANK_MAX + file; //Final-Rank
}

int Engine::minimax(const State& state, const std::uint32_t depth, int alpha, int beta)
{
	m_nodes++;

	if (depth == 0)
	{
		return evaluate(state);
	}

	//time cutoff for iterative deepening
	if (m_stopSearch)
	{
		return state.whiteToMove() ? INT_MAX : INT_MIN;
	}

	//only test time every 1000 nodes to avoid frequent system calls
	if (m_timeCheckCount >= TIME_EVALUATION_NODE_DELAY)
	{
		const auto now{ std::chrono::high_resolution_clock::now() };
		const std::chrono::duration<float> duration{ now - m_searchStartTime };

		if (static_cast<size_t>(duration.count()) >= MAX_EVALUATION_TIME_SECONDS)
		{
			m_stopSearch = true;
			return state.whiteToMove() ? INT_MAX : INT_MIN;
		}

		m_timeCheckCount = 0;
	}

	m_timeCheckCount++;

	if (state.whiteToMove())
	{
		MoveList moves;
		m_moveGen.generateMoves(state, moves);
		moves.sortMoveList();

		int max_eval{ INT_MIN };
		bool anyLegalMoves{ false };

		for (Move move : moves.moves())
		{
			State new_state{ state };

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();

				const int eval = minimax(new_state, depth - 1, alpha, beta);

				//time cutoff for iterative deepening
				if (m_stopSearch)
				{
					return state.whiteToMove() ? INT_MAX : INT_MIN;
				}

				if (eval > max_eval)
				{
					max_eval = eval;

					if (depth == m_depth)
					{
						m_bestMove = move;
					}
				}

				if (alpha < eval)
				{
					alpha = eval;
				}

				if (beta <= alpha)
				{
					m_prunes++;
					break;
				}
			}
		}

		if (anyLegalMoves)
		{
			return max_eval;
		}
		else
		{
			if (kingInCheck(state))
			{
				//white checkmate
				m_mates++;
				return INT_MIN + depth + 1;
			}
			else
			{
				//white stalemate
				return 0;
			}
		}
	}
	else
	{
		MoveList moves;
		m_moveGen.generateMoves(state, moves);
		moves.sortMoveList();

		int min_eval{ INT_MAX };
		bool anyLegalMoves{ false };

		for (Move move : moves.moves())
		{
			State new_state{ state };

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();
				const int eval = minimax(new_state, depth - 1, alpha, beta);

				//time cutoff for iterative deepening
				if (m_stopSearch)
				{
					return state.whiteToMove() ? INT_MAX : INT_MIN;
				}

				if (eval < min_eval)
				{
					min_eval = eval;

					if (depth == m_depth)
					{
						m_bestMove = move;
					}
				}

				if (beta > eval)
				{
					beta = eval;
				}

				if (beta <= alpha)
				{
					m_prunes++;
					break;
				}
			}
		}
		if (anyLegalMoves)
		{
			return min_eval;
		}
		else
		{
			if (kingInCheck(state))
			{
				//black checkmate
				m_mates++;
				return INT_MAX - depth - 1;
			}
			else
			{
				//black stalemate
				return 0;
			}
		}
	}
}



void Engine::iterativeMinimax(const State& state)
{
	//to implement minimax function with iterative depths 
	std::uint32_t depth{ 1 };
	m_searchStartTime = std::chrono::high_resolution_clock::now();
	m_timeCheckCount = 0;
	m_stopSearch = false;

	while (!m_stopSearch)
	{
		m_depth = depth;
		minimax(m_state, depth, INT_MIN, INT_MAX);
		m_depthSearched = depth; //TODO: fix these vars there should only be one
		depth++;

		if (!m_stopSearch)
		{
			m_bestMoveFinal = m_bestMove;
		}
	}
}