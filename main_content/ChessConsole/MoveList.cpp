#include "MoveList.h"


MoveList::MoveList()
	: m_moves(), m_count()
{
	m_moves.reserve(256);
}

const std::vector<Move>& MoveList::moves() const 
{
	//for (Move(Move type) move : moves.moves()) 
	//{
    // explore this move
	//}
	return m_moves;
}

std::size_t MoveList::count() const
{
	return m_moves.size(); //O(1)
}

void MoveList::sortMoveList() //average O(n log n)
{
	std::sort(m_moves.begin(), m_moves.end(), move_compare); //[begin, end)
}

bool MoveList::move_compare(const Move a, const Move b)
{
	return a.value() > b.value();
}
void MoveList::popMove(const std::size_t move_index)
{
	std::swap(m_moves[move_index], m_moves.back());
	m_moves.pop_back();
}

bool MoveList::findMove(const std::size_t source, const std::size_t target, Move& move_out) const //reference to move variable, this move has to be used for makemove 
{ //to validate the move that human entered "e2e4"
	for (Move move : m_moves)
	{
		if (move.source() == source)
		{
			if (move.target() == target)
			{
				move_out = move;
				return true;
			}
		}
	}

	return false;
}

bool MoveList::findCastleMove(const std::size_t source) const
{ //player type "WK",Engine::inputAndParseMove calls findCastleMove(g1) 
	// checking if a castle move to g1 exists in the generated moves. 
	//This confirms the castle is actually legal in the current position 
	for (Move move : m_moves)
	{
		if (move.castle()) //bool func
		{
			if (move.source() == source)
			{
				return true;
			}
		}
	}
//no refernce to move bcz here the castle will be created fresh in the createcastle in MOVE
//we just need to check if the move is feasable or not
	return false;
}

void MoveList::printMoves() const
{
	for (std::size_t i{}; i < m_moves.size(); i++)
	{
		m_moves[i].print();
	}
}

