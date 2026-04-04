#include "Engine.h"

Engine::Engine()
	: m_moveGen(), m_state(), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth(), m_depthSearched(), m_stopSearch(), m_timeCheckCount(),
	  m_bestMoveFinal(), m_moveSource() {}

Engine::Engine(std::string_view fen)
	: m_moveGen(), m_state(State::parse_fen(fen)), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth(), m_depthSearched(), m_stopSearch(),
	  m_timeCheckCount(), m_bestMoveFinal(), m_moveSource() {}
// giving arguments

void Engine::setState(std::string_view fen)
{
	m_state = State::parse_fen(fen);
}

int Engine::evaluate(const State &state)
{
	m_evaluations++; // no of evaluations made

	int evaluation{};
	//just means evaluation =0; protects from garbage values
	for (std::size_t piece{}; piece < PIECE_COUNT; piece++)
	{
		BitBoard piece_board = state.positions()[piece];
//state.positions()== is a member function call ,return 12 bitboards 
		while (piece_board.board()) 
		// piece_board.board(), also a member function call of Bitboard class
		{
			const std::size_t square = piece_board.find_1lsb();
			const BitBoard attack = m_moveGen.getPieceAttack(piece, square, state);
			//m_movegen is a object of the Movegen class and getpieceattack() returns a bitboard 
			//containing all the squares this piece can attack from this "square"
			evaluation += piece_value[piece];
			//defined in chessconstant.hpp
			evaluation += static_cast<int>(attack.bitCount());

			// calculating score
			piece_board.reset(square); // removing the least set bit (processed piece from the board)
		}
	}

	return evaluation;
}

bool Engine::kingInCheck(const State &state) const //bool to know if king in check or not 
{
	if (state.whiteToMove()) // white move
	{
		const std::size_t king_square = state.positions()[KING].find_1lsb(); //king square loop is bigo 1

		if (king_square == SIZE_MAX) //defensive programming gaurd 
		{ //king is missing from the board , something went very wrong in the prev step 
			return true;
		}
		else
		{
			return m_moveGen.isSquareAttacked(state, king_square, Color::WHITE);
		}
	}
	else
	{ // black side to move
		const std::size_t king_square = state.positions()[BKING].find_1lsb();

		if (king_square == SIZE_MAX) // king in check
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
	const std::size_t rank{7 - static_cast<std::size_t>(square[1] - '1')}; // X-axis
	const std::size_t file{static_cast<std::size_t>(square[0] - 'a')};	   // Y-axis

	return rank * RANK_MAX + file; // Final-Rank
}

int Engine::minimax(const State &state, const std::uint32_t depth, int alpha, int beta)
{
	m_nodes++;

	if (depth == 0)
	{
		return evaluate(state);
	}

	// time cutoff for iterative deepening
	if (m_stopSearch)
	{
		return state.whiteToMove() ? INT_MAX : INT_MIN;
	}

	// this loop tells enough time has passed and 1000 nodes have been evaluated , 
	//so we can test the time now to check if it hasnt passed time limitation , 
	// to avoid freqyenct system calls 
	if (m_timeCheckCount >= TIME_EVALUATION_NODE_DELAY)
	{
		const auto now{std::chrono::high_resolution_clock::now()}; //time point , not a number 
		const std::chrono::duration<float> duration{now - m_searchStartTime};

		if (static_cast<size_t>(duration.count()) >= MAX_EVALUATION_TIME_SECONDS) //constexpr constant
		{ //duration.count() == returns raw number (seconds)
			m_stopSearch = true;
			return state.whiteToMove() ? INT_MAX : INT_MIN; //returns extreme value so branch is ignored to abort deeper search
		}

		m_timeCheckCount = 0; //Have to Otherwise The mtimecheckcount Will go to 1001, 1002 
		//And everytime it will come under this if loop
	}

	m_timeCheckCount++; 

	if (state.whiteToMove())
	{
		MoveList moves;
		m_moveGen.generateMoves(state, moves);
		moves.sortMoveList();
		//sorting so we get the best moves first (pruning efficiency increases)
		
		
		int max_eval{INT_MIN}; //best score found so far 
		bool anyLegalMoves{false}; //for detecting checkmates / stalemates 

		for (Move move : moves.moves()) //moves.moves() will return vector<move>
		{
			State new_state{state}; // we will simulate the move on a copied board 

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();
				//recursion to get opponents best reply score 
				const int eval = minimax(new_state, depth - 1, alpha, beta);

				// time cutoff for iterative deepening
				if (m_stopSearch)
				{
					return state.whiteToMove() ? INT_MAX : INT_MIN;
				}
				//update best score
				if (eval > max_eval)
				{
					max_eval = eval;

					if (depth == m_depth)
					{ //only storing move at the root level so that we dont overwrite best moves from deeper levels, thats th job of iterativeminimax
						m_bestMove = move;
					}
				}

				if (alpha < eval)
				{
					alpha = eval; //best gaurenteed score for white so far 
				}

				if (beta <= alpha) //opponent will never allow this branch , prune it 
				{
					m_prunes++;
					break; //prune
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
				// white checkmate
				m_mates++;
				return INT_MIN + depth + 1; //why depth +1 , bcz faster matesz are better and slower loss is better 
			}
			else
			{
				// white stalemate
				return 0;
			}
		}
	}
	else //black to move 
	{
		MoveList moves;
		m_moveGen.generateMoves(state, moves);
		moves.sortMoveList();

		int min_eval{INT_MAX};
		bool anyLegalMoves{false};

		for (Move move : moves.moves())
		{
			State new_state{state};

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();
				const int eval = minimax(new_state, depth - 1, alpha, beta);

				// time cutoff for iterative deepening
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
				// black checkmate
				m_mates++;
				return INT_MAX - depth - 1;
			}
			else
			{
				// black stalemate
				return 0;
			}
		}
	}
}

void Engine::printAllBoardAttacks(Color C) const  //which squares are attacked by a given side (color c) ?*********
{ // this will use pregenearted attack tables 
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

void Engine::printBoard(const bool flipped) const //thin wrapper
{
	m_state.printBoard(flipped, RF::no_sqr);
}

void Engine::iterativeMinimax(const State &state)
{
	// to implement minimax function with iterative depths
	std::uint32_t depth{1};
	m_searchStartTime = std::chrono::high_resolution_clock::now();
	m_timeCheckCount = 0;
	m_stopSearch = false;

	while (!m_stopSearch)
	{
		m_depth = depth;
		minimax(m_state, depth, INT_MIN, INT_MAX); //alpha is int_min and beta is int_max
		m_depthSearched = depth;
		depth++;

		if (!m_stopSearch)
		{
			m_bestMoveFinal = m_bestMove; //storing the best move 
		}
	}
}

bool Engine::inputAndParseMove(MoveList &list, Move &move)
{ 
	std::string input;
	std::getline(std::cin, input);  // reads the entire user line including spaces and creates a string variable names input 
	// std::transform(input.begin(), input.end(), input.begin(), ::tolower);

	if (input == "wk"s)
	{
		if (list.findCastleMove(g1))  // checks if white king-side castle is legal right now , as castling rights may be gone or path may be blocked 
		// where g1 is kings destination square **
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
		const std::string source{input.substr(0, 2)};
		const std::string target{input.substr(2, 2)};
		const std::size_t source_square{Engine::squareToIndex(source)};
		const std::size_t target_square{Engine::squareToIndex(target)};

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
{ //game controller loop
	m_state.printBoard(flip_board, RF::no_sqr); //shows the starting position , visual context
	//RF::no_sqr -- no move has been made so no square has to be highlighted 
	m_depth = depth;

	while (true) //withput it only one move will be played , thats ehy game loop 
	{
		const auto start_time = std::chrono::high_resolution_clock::now(); //time for a single move 

		if (m_state.whiteToMove() == engine_side_white)
		//m_state.whiteToMove() -- tells whose turn is on the board , engine_side_white== which side the engine controls 
		{ // if white to play and engine plays is whitee
			if constexpr (PLAYER_PLAY_ITSELF) //this allows different modes , player vs engine , player vs player , engine vs engine  
			{//player turn
				
				MoveList list; 					
				m_moveGen.generateMoves(m_state, list); //fills the list ^^

				// movegen generates all thge legal moves and movelist stores them 
				Move move;
				while (true) 
				{
					if (inputAndParseMove(list, move))
					{
						State new_state{m_state};   // we will test the move on a dummy board 
						new_state.printBoard(flip_board, m_moveSource);
						move.print();

						if (makeMove(move, new_state))
						{ // we have to ensure if the move doesnt leave our own king in check ****
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
				// engine move
				std::cout << "thinking" << std::endl;
				iterativeMinimax(m_state);     //deciding best move 
				makeMove(m_bestMoveFinal, m_state);   //apllying that best move 

				m_moveSource = m_bestMoveFinal.source();
			}
		}
		else //black move 
		{
			if constexpr (ENGINE_PLAY_ITSELF)
			{
				// engine move
				std::cout << "thinking" << std::endl;
				iterativeMinimax(m_state);
				makeMove(m_bestMoveFinal, m_state);

				m_moveSource = m_bestMoveFinal.source();
			}
			else
			{
				// player move
				MoveList list;
				m_moveGen.generateMoves(m_state, list);

				Move move;
				while (true)
				{
					if (inputAndParseMove(list, move))
					{
						State new_state{m_state};
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
		// this has to be double and also we have to calculate the time duration that has expired
		//  so we subtract from the current time pooint

		system("cls");  //for cleaning UI
		m_state.printBoard(flip_board, m_moveSource);

		std::cout << "move: ";
		m_bestMoveFinal.print();

		std::cout << "depth: " << m_depthSearched << std::endl;
		std::cout << "nodes: " << m_nodes << std::endl;   //position searched
		std::cout << "evaluations: " << m_evaluations << std::endl;
		std::cout << "prunes: " << m_prunes << std::endl;
		std::cout << "mates: " << m_mates << std::endl;   //detected 
		std::cout << duration.count() << " seconds" << std::endl;

		m_nodes = 0;
		m_evaluations = 0;
		m_prunes = 0;
		m_mates = 0;
		m_depthSearched = 0;
//each move has fresh stats 
		m_state.flipSide();
	}
}

bool Engine::makeMove(const Move move, State &state) const
{
	state.setEnpassantSquare(no_sqr);

	// unpack
	const std::size_t source = move.source();
	const std::size_t target = move.target();
	const Piece piece = move.piece();
	const bool promoted = move.promoted();
	const bool capture = move.capture();
	const bool double_pawn = move.doublePawnPush();
	const bool enpassant = move.enpassant();
	const bool castle = move.castle();

	// if statements in most efficient order for least number of branching
	if (castle) // TODO: remove moveQuiet and moveCapture they have unnessesary loops and checks
	{
		if (state.whiteToMove())
		{
			state.moveQuiet(KING, e1, source);

			if (source == g1)
			{
				state.moveQuiet(ROOK, h1, f1);
				state.setCastleRights(h1);
			}
			else
			{
				state.moveQuiet(ROOK, a1, d1);
				state.setCastleRights(a1);
			}
		}
		else
		{
			state.moveQuiet(BKING, e8, source);

			if (source == g8)
			{
				state.moveQuiet(BROOK, h8, f8);
				state.setCastleRights(h8);
			}
			else
			{
				state.moveQuiet(BROOK, a8, d8);
				state.setCastleRights(a8);
			}
		}
	}
	else
	{
		state.setCastleRights(source);
		state.setCastleRights(target);

		// captures
		if (capture)
		{
			if (promoted)
			{
				state.popPiece(state.whiteToMove() ? Piece::PAWN : Piece::BPAWN, source);
				state.popSquare(target);
				state.setPiece(piece, target);
			}
			else if (enpassant)
			{
				if (state.whiteToMove())
				{
					state.popPiece(Piece::PAWN, source);
					state.popPiece(Piece::BPAWN, target + 8);
					state.setPiece(Piece::PAWN, target);
				}
				else
				{
					state.popPiece(Piece::BPAWN, source);
					state.popPiece(Piece::PAWN, target - 8);
					state.setPiece(Piece::BPAWN, target);
				}
			}
			else
			{
				state.moveCapture(piece, source, target);
			}
		}
		// quiets (no capture moves)
		else
		{
			if (double_pawn)
			{ // we know its a pawn we dont have to loop through pieces
				state.moveQuiet(state.whiteToMove() ? PAWN : BPAWN, source, target);
				state.setEnpassantSquare(state.whiteToMove() ? source - 8 : source + 8);
			}
			else if (promoted)
			{
				state.popPiece(state.whiteToMove() ? Piece::PAWN : Piece::BPAWN, source);
				state.setPiece(piece, target);
			}
			else
			{
				state.moveQuiet(piece, source, target);
			}
		}
	}

	if (kingInCheck(state))
	{	// TO CHECK WHETHER KING IS IN CHECKMATE 
		return false;
	}
	else
	{
		return true;
	}
}
