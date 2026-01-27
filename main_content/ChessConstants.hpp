#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <string_view>
#include <cstddef>

using namespace std::literals::string_view_literals;


constexpr std::uint64_t single_bit									= 1;
constexpr std::size_t   RANK_MAX									= 8;
constexpr std::size_t   FILE_MAX									= 8;
constexpr std::size_t   MAX_BOARD_POSITIONS = FILE_MAX * RANK_MAX;	//64
constexpr std::size_t   WHITE_COLOR									= 0;
constexpr std::size_t   BLACK_COLOR									= 1;
constexpr std::size_t   MAX_COLORS									= 2;
constexpr std::size_t   MAGIC_ITERATION_COUNT						= 1000000;
constexpr std::size_t   MAX_BISHOP_ATTACKS							= 512;
constexpr std::size_t   MAX_ROOK_ATTACKS							= 4096;
constexpr std::size_t   PIECE_COUNT									= 12;
constexpr std::size_t   MAX_MOVELIST_COUNT							= 256;
constexpr std::uint32_t MAX_MINIMAX_DEPTH							= INT_MAX - 1;
constexpr std::size_t   TIME_EVALUATION_NODE_DELAY					= 1000;
constexpr std::size_t   MAX_EVALUATION_TIME_SECONDS					= 1; //maybe remove

constexpr bool USING_PREGENERATED_MAGICS = true;
constexpr bool PRINT_GENERATED_MAGICS = false;
constexpr bool ENGINE_PLAY_ITSELF = false;
constexpr bool PLAYER_PLAY_ITSELF = false;

const std::string start_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
const std::string tricky_position_fen = "r3k2r/p11pqpb1/bn2pnp1/2pPN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
