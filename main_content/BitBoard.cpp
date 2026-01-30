#include "BitBoard.h"

BitBoard::BitBoard()
	: m_board() {}

BitBoard::BitBoard(std::uint64_t board)
	: m_board(board) {}

std::uint64_t BitBoard::board() const 
{ 
	return m_board; 
}

void BitBoard::setBoard(std::uint64_t board) 
{ 
	m_board = board; 
}

void BitBoard::print() const
{
	//loop over ranks 
	for (std::size_t r{}; r < RANK_MAX; r++)
	{
		std::cout << RANK_MAX - r << "    ";
		//loop over files
		for (std::size_t f{}; f < FILE_MAX; f++)
		{
			const std::uint32_t bit{ static_cast<std::uint32_t>(test(bitset_index(f, r)) ? 1 : 0 )};
			std::cout << bit << " ";
		}
		//end the line
		std::cout << std::endl;
	}
	std::cout << "\n     A B C D E F G H\n\n";
}