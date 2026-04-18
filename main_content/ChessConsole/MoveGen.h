#pragma once

#include "ChessConstants.hpp"
#include "BitBoard.h"
#include "PreGen.h"
#include "State.h"
#include "string"
#include <cstddef>
#include "Move.h"
#include "MoveList.h"

class MoveGen
{
private:
	PreGen m_preGen; //when movegen is created pregen's constructor runs (entire pregen pipeline), all 2.3MB of tables are built

public:
	MoveGen();

	void generateMoves(const State& state, MoveList& moveList);

	bool isSquareAttacked(const State& state, const std::size_t square, const Color side) const;

	BitBoard getBishopAttack(const std::size_t square, const BitBoard occupancy) const; //the payoff of the entire magic working

	//BitBoard MoveGen::getBishopAttack(const std::size_t square, 
//                                    const BitBoard occupancy) const
// {
//     const std::size_t magic_number = static_cast<size_t>(m_preGen.bishopMagics()[square]);     //bishopMagics()==returns reference to array of 64 magic numbers
//     const std::uint64_t masked_occupancy = 
//         m_preGen.maskRelevantBits<Piece::BISHOP>(occupancy, square);
//     const std::size_t magic_index = 
//         (masked_occupancy * magic_number) >> 
//         (MAX_BOARD_POSITIONS - m_preGen.bishopBitCount()[square]);
//     return m_preGen.bishopAttacks()[square][magic_index];
// }

	BitBoard getRookAttack(const std::size_t square, const BitBoard occupancy) const;

	template<Color C>
	BitBoard getPawnAttack(const std::size_t square) const;

	BitBoard getKnightAttack(const std::size_t square) const;

	BitBoard getKingAttack(const std::size_t square) const;

	BitBoard getPieceAttack(const std::size_t P, std::size_t square, const State& state);
};