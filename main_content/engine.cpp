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

void Engine::printAllBoardAttacks(Color C) const
{
	BitBoard b;
	for (int i = 0; i < 64; i++)
	{
		if (m_moveGen.isSquareAttacked(m_state, i, C))
		{
			b.set(i);
		}
	}
	b.print();
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
		m_depthSearched = depth; 
		depth++;

		if (!m_stopSearch)
		{
			m_bestMoveFinal = m_bestMove;
		}
	}
}

bool Engine::inputAndParseMove(MoveList& list, Move& move)
{
	std::string input;
	std::getline(std::cin, input);
	//std::transform(input.begin(), input.end(), input.begin(), ::tolower);

	if (input == "wk"s)
	{
		if (list.findCastleMove(g1))
		{
			move = Move::createCastleMove<Castle::WK>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input == "wq"s)
	{
		if (list.findCastleMove(c1))
		{
			move = Move::createCastleMove<Castle::WQ>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input == "bk"s)
	{
		if (list.findCastleMove(g8))
		{
			move = Move::createCastleMove<Castle::BK>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input == "bq"s)
	{
		if (list.findCastleMove(c8))
		{
			move = Move::createCastleMove<Castle::BQ>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input.length() == 4)
	{
		const std::string source{ input.substr(0, 2) };
		const std::string target{ input.substr(2, 2) };
		const std::size_t source_square{ Engine::squareToIndex(source) };
		const std::size_t target_square{ Engine::squareToIndex(target) };

		if (list.findMove(source_square, target_square, move))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void Engine::step(const bool engine_side_white, const bool flip_board, const std::uint32_t depth)
{ //used 32 bcz dont need negative values for depth
	m_state.printBoard(flip_board, RF::no_sqr);
	m_depth = depth;

	while (true)
	{
		const auto start_time = std::chrono::high_resolution_clock::now();

		if (m_state.whiteToMove() == engine_side_white)
		{ //if white to play and player side is whitee
			if constexpr (PLAYER_PLAY_ITSELF)
			{
				//player move
				MoveList list;
				m_moveGen.generateMoves(m_state, list);

				Move move;
				while (true)
				{
					if (inputAndParseMove(list, move))
					{
						State new_state{ m_state };
						new_state.printBoard(flip_board, m_moveSource);
						move.print();

						if (makeMove(move, new_state))
						{
							m_state = new_state;
							m_moveSource = move.source();
							break;
						}
					}

					std::cout << "move does not exist" << std::endl;
				}
			}
			else
			{
				//engine move
				std::cout << "thinking" << std::endl;
				iterativeMinimax(m_state);
				makeMove(m_bestMoveFinal, m_state);

				m_moveSource = m_bestMoveFinal.source();
			}
		}
		else
		{
			if constexpr (ENGINE_PLAY_ITSELF)
			{
				//engine move
				std::cout << "thinking" << std::endl;
				iterativeMinimax(m_state);
				makeMove(m_bestMoveFinal, m_state);

				m_moveSource = m_bestMoveFinal.source();
			}
			else
			{
				//player move
				MoveList list;
				m_moveGen.generateMoves(m_state, list);

				Move move;
				while (true)
				{
					if (inputAndParseMove(list, move))
					{
						State new_state{ m_state };
						new_state.printBoard(flip_board, m_moveSource);
						move.print();

						if (makeMove(move, new_state))
						{
							m_state = new_state;
							m_moveSource = move.source();
							break;
						}
					}

					std::cout << "move does not exist" << std::endl;
				}
			}
		}

		const auto end_time = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double> duration = end_time - start_time;

		system("cls");
		m_state.printBoard(flip_board, m_moveSource);

		std::cout << "move: ";
		m_bestMoveFinal.print();

		std::cout << "depth: " << m_depthSearched << std::endl;
		std::cout << "nodes: " << m_nodes << std::endl;
		std::cout << "evaluations: " << m_evaluations << std::endl;
		std::cout << "prunes: " << m_prunes << std::endl;
		std::cout << "mates: " << m_mates << std::endl;
		std::cout << duration.count() << " seconds" << std::endl;

		m_nodes = 0;
		m_evaluations = 0;
		m_prunes = 0;
		m_mates = 0;
		m_depthSearched = 0;

		m_state.flipSide();
	}
}




