#pragma once

#include "BitBoard.h"
#include <array>
#include "ChessConstants.hpp"
#include <string>
#include <string_view>
#include "Move.h"

struct State
{
private:
	std::array<BitBoard, 12> m_positions;
	std::array<BitBoard, 3> m_occupancy;

	std::uint8_t m_castleRights;

	std::size_t m_enpassantSquare;

	bool m_whiteToMove;
//omplete snapshot of a chess position stored in approximately 130 bytes entirely on the stack
public:
	State();

	State(const State& state);

	const std::array<BitBoard, 12>& positions() const; 
	//This method doesn't modify State, 
	//and returns a reference to the positions array that the caller can read but not modify.

	const std::array<BitBoard, 3>& occupancy() const;

	Piece testPieceType(const std::size_t square) const; // checks all 12 bitboards , to find which type of piece is being captured 

	std::uint8_t castleRights() const; // target swuare , here for inspection 

	void setCastleRights(std::size_t square); // to revoke from target or source (if to or from)

	bool testCastleRights(const Castle C) const;  //m_castleRights & C , if non zero castle is allowed 

	std::size_t enpassantSquare() const; //target square 

	void setEnpassantSquare(const std::size_t square); //called in two situations  

	bool whiteToMove() const; // white side 

	void flipSide(); // to flip the board after every move 

	void printBoard(const bool flipped, const std::size_t source_square) const;

	void setPiece(const Piece P, const std::size_t square); // must update 3 things 
	//1) m_occupancy [P/6]   ----- p/6=0 fro white and 1 for black 
	//2) m_posiiton [p]
	//3) m_occupancy [both]

	void popPiece(const Piece P, const std::size_t square);

	void popSquare(const std::size_t square);

	void moveQuiet(const Piece P, const std::size_t source, const std::size_t target);

	void moveCapture(const Piece P, const std::size_t source, const std::size_t target);

	static State parse_fen(const std::string_view fen);

	static std::array<std::string, RANK_MAX> split_fen(std::string_view fen);
};