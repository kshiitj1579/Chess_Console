#include "State.h"




// There are three categories of functions here:

// Constructors == initializing 
// Piece operations == setPiece, popPiece, popSquare, moveQuiet, moveCapture 
// FEN parsing == parse_fen and split_fen 
// Display === printBoard 

State::State()
	: m_positions(), m_occupancy(), m_whiteToMove(true), m_enpassantSquare(no_sqr), m_castleRights(0b1111) {}


State::State(const State& state)
	: m_positions(state.m_positions), 
	m_occupancy(state.m_occupancy),
	m_whiteToMove(state.m_whiteToMove), 
	m_enpassantSquare(no_sqr), //always gets reset to no square
	m_castleRights(state.m_castleRights) 
{}

std::uint8_t State::castleRights() const
{
	return m_castleRights;
}

void State::setCastleRights(std::size_t square)
{
	m_castleRights &= castling_rights[square];
}

const std::array<BitBoard, 12>& State::positions() const
{
	return m_positions;
}

const std::array<BitBoard, 3>& State::occupancy() const
{
	return m_occupancy;
}

std::size_t State::enpassantSquare() const
{
	return m_enpassantSquare;
}

Piece State::testPieceType(const std::size_t square) const
{  // during capturing at the atget square , we need to check the piece that is being captured 
	for (std::size_t piece{}; piece < PIECE_COUNT; piece++)
	{
		if (m_positions[piece].test(square))
		{
			return static_cast<Piece>(piece);  // piece is an size_t integer, Piece is an enum 
		}
	}

	return Piece::NO_PIECE; // sentinal value of 12
}

bool State::testCastleRights(const Castle C) const
{
	return static_cast<bool>(m_castleRights & C);
}

void State::setEnpassantSquare(const std::size_t square)
{
	m_enpassantSquare = square;
}

bool State::whiteToMove() const
{
	return m_whiteToMove;
}

void State::flipSide()
{
	m_whiteToMove = !m_whiteToMove;
}

void State::setPiece(const Piece P, const std::size_t square)
{
	m_positions[static_cast<size_t>(P)].set(square); // static_cast<size_t>(P)====Converts piece enum (internally int) to size_t
	m_occupancy[static_cast<size_t>(P / 6)].set(square);
	m_occupancy[Occupancy::BOTH].set(square);
//have to update all three ATOMICALLY

}

void State::popPiece(const Piece P, const std::size_t square)
{
	m_positions[static_cast<size_t>(P)].reset(square);
	m_occupancy[static_cast<size_t>(P / 6)].reset(square);
	m_occupancy[Occupancy::BOTH].reset(square);
}

//line 87 function popsquare 

void State::popSquare(const std::size_t square)
{
	if(m_whiteToMove)
	{
		for (std::size_t piece{ Piece::BPAWN }; piece < PIECE_COUNT; piece++)
		{
			if (m_positions[piece].test(square))
			{
				popPiece(static_cast<Piece>(piece), square);
				break;
			}
		}
	}
	else
	{
		for (std::size_t piece{ Piece::PAWN }; piece < Piece::BPAWN; piece++)
		{
			if (m_positions[piece].test(square))
			{
				popPiece(static_cast<Piece>(piece), square);
				break;
			}
		}
	}
}

void State::moveQuiet(const Piece piece, const std::size_t source, const std::size_t target)
{
	popPiece(piece, source);
	setPiece(piece, target);
}

void State::moveCapture(const Piece piece, const std::size_t source, const std::size_t target)
{
	if (m_whiteToMove)
	{
		for (std::size_t piece{ Piece::BPAWN }; piece < PIECE_COUNT; piece++)
		{
			if (m_positions[piece].test(target))
			{
				popPiece(static_cast<Piece>(piece), target);
				break;
			}
		}
	}
	else
	{
		for (std::size_t piece{ Piece::PAWN }; piece < Piece::BPAWN; piece++)
		{
			if (m_positions[piece].test(target))
			{
				popPiece(static_cast<Piece>(piece), target);
				break;
			}
		}
	}

	moveQuiet(piece, source, target);
}

void State::printBoard(const bool flipped, const std::size_t source_square) const
{
	if (flipped)
	{ // when black on the bottom thas why we have to do 7-r 7-f
		for (std::size_t r{}; r < RANK_MAX; r++)
		{
			std::cout << (r + 1) << "   ";
			for (std::size_t f{}; f < FILE_MAX; f++)
			{
				char piece_char = '.'; //Default character for an empty square.

				for (std::size_t i{}; i < PIECE_COUNT; i++)
				{
					if (m_positions[i].test_rf(7 - r, 7 - f))
					{
						piece_char = piece_to_char[i];
					}
				}

				const std::size_t square{ r * RANK_MAX + f };
//the changes rto the formula have been made 
				if (square == source_square)
				{
					piece_char = ' ';
				}
				//If this square is the source square of the last move, 
				//display it as a blank space regardless of what piece was there.
				// This gives a visual "ghost" effect, the departure square appears empty, making it easy to see where the last piece came from.
				std::cout << piece_char << " ";
			}
			std::cout << std::endl;
		}

		std::cout << std::endl << "    H G F E D C B A " << (m_whiteToMove ? "white" : "black") << std::endl;
	}
	else
	{
		for (std::size_t r{}; r < RANK_MAX; r++)
		{
			std::cout << (RANK_MAX - r) << "   "; //prints 8, 7, 6... 1 (top to bottom), normal board
			for (std::size_t f{}; f < FILE_MAX; f++)
			{
				char piece_char = '.';

				for (std::size_t i{}; i < PIECE_COUNT; i++)
				{

					if (m_positions[i].test_rf(r, f))
					{
						piece_char = piece_to_char[i];
					}
				}

				const std::size_t square{ r * RANK_MAX + f };

				if (square == source_square)
				{
					piece_char = ' ';
				}

				std::cout << piece_char << " ";
			}
			std::cout << std::endl;
		}

		std::cout << std::endl << "    A B C D E F G H " << (m_whiteToMove ? "white" : "black") << std::endl; //ternery operator
	}
}

State State::parse_fen(const std::string_view fen)
{
	State state;

	const std::array<std::string, RANK_MAX> ranks{ split_fen(fen) };

	std::size_t index{};

	for (const auto& rank : ranks)
	{
		for (auto& c : rank)
		{
			if (std::isdigit(c)) //cctype added
			{
				std::size_t digit = static_cast<std::size_t>(c - '0');
				index += digit;
			}
			else
			{
				const std::size_t char_index{ char_to_piece[c] }; //converts character to size_t

				//char to piece has an o(1) lookup
				const Piece piece{ static_cast<Piece>(char_index) }; //converts size_t to piece enum
				// 'R' will be converted to 3 and then 3 will be converted to Piece:: ROOK
				state.setPiece(piece, index);
				index++;
			}
		}
	}

	return state;
	// Returns the fully populated State by value.     			
	// is this Return Value Optimization (RVO)?
	// the compiler constructs the State directly my (the caller's) memory,
	// avoiding any copy. So despite appearances, no copy of state actually happens here.
}

std::array<std::string, RANK_MAX> State::split_fen(std::string_view fen)
{
	std::array<std::string, RANK_MAX> split_fen;
	std::size_t index{};

	for (std::size_t i{}; i < RANK_MAX; i++)
	{
		std::string rank;

		while (index < fen.size() && fen[index] != '/')  // Keep adding until '/' or end of string
		{
			rank += fen[index];
			index++;
		}
		//index currently pointing at the '/'

		split_fen[i] = rank;
		index++;  // Skip the '/' delimiter, but ensure we don't go past the string length
		// after the last part if we do index++ its still harmless as the i has reache dits limit of 8
	}

	return split_fen;
}