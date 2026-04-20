# Page 1

Chess console
- future position using minimax + alpha beta pruning
- std:: uint64_t — Unsigned 64bit integer, a standard fixed width integer.
- chessboard has 64 squares
- move operations become &, or, shift. not loops.
- 12 piece types :- pawns, knights, bishop, rook, queen, king
- 12 x 64bits = 768 bits = 96 bytes (£'s)
- chessboard is encoded as bits inside a 64 bit integer. this is called bitboard.
- instead of bit[0][0], we use uint64_t whitePawn;
- FEN :- forsyth Edward notation, standardised text format.
- slider pieces that move in a strict line until blocked (Bishop, rook, queen), depends on blockers, makes them expensive to compute.
- Minimax :- Game tree search algorithm to choose moves assuming the opponent plays optimally.

search optimisation technique
skipping branches that cannot affect the final move.
→ if the engine already found a better move, it stops exploring worse alternatives.

search depth, no. of half moves the engine explores. (ply)

not one integer for the whole board, one integer per piece. the engine uses 12 separate uint64_t.
type, not variable.
each uint64_t one piece type across all 64 squares.

whitePawns & (1ULL<<e4) says is there a white pawn on e4.

→ a single line that format that fully describes a chess board.
a full FEN has 6 space separated fields.
we use the board layout:
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
capital = white king side, full move no. after block moves.
rook / bishop
while to move
no castling
pawn move
50 move draw out.
half move clock.

A knight on d4
d → 4th rank
file → vertical line
rank → horizontal line

Magic Numbers
- for slider pieces, move many squares but blocked by other pieces.
brute:- check every square, stop when blocked, too slow.
optimal method:- we need to find index(x)
(x)ind = (occupancy & mask) * magic >> shift.

I) You have rook on certain square, you create a bitmap (board with 0 x 1) that highlights only squares that the rook can move.

II) apply that bitmask to the actual board state to get relevant blocker sites/bits.

III) multiply magic no. & shift
why? → we end up with a huge no. after x, large than range. shift to scale down & bring it into a smaller range.

IV) now we have the index (tell rook can move 4 up, 2 left, 3 down, 2 right)
its a bitboard, another 64bit integer, where each bit represents whether the rook can move to that square.
each bit in that 64 bit corresponds to 1 sq on 8x8 chessboard.
it points to a precomputed set of moves for that specific blocker config.

Magic numbers are found by trial & error beforehand.
Precomputation, not only the magic numbers (can be multiple) are prestored but the index value.

---

# Page 2

lets take an example of rook on d4.
no. of possible blocker patterns = 2^12 = 4096
files, ranks, (except) edges.

now we will have 4096 bitboards
showcasing all the possible permutations of blockers config along the path of rook.(d4)
we will take the x op with the masked bit matrix.

occupancy - 0
occupancy - 4095

shows one block is 2 steps up, one is 3 steps left.

why can't we just calculate the specific occupancy mask that we need during the game, no need for precomputation?
→ so that the engine handles any situation instantly.
the engine never has to calculate any move on the fly.
it looks up the move in constant time.
by precomputing them we also calculate/identify the magic no. for each square, for each rook, for each knight.
so we have 4096 blocker config, for 64 places, for different side pieces.
precomputation = 4096 x 64 x different side pieces.
once & then reuse it forever.
this makes the engine incredibly fast.

The most bitboard will have bits set to 1 along its rank & file.
you take current boards blocker config & apply x op with (1 of 4096) the mask that isolates just the relevant squares.

Now we generate large 64 bit num that we hope will work, generate candidates.
total 120 magic numbers.

(ii) test candidates.
we map all 4096 occupancies, we check if this magic num(m) can produce a unique index without collisions.
→ meaning occupancy after x op with mask.
2000 x 3000 multiply both by m (& shift) should give unique index.

if same index:- two different blocker config are pointing to same set of moves.

now you can store that magic num & calculate index values for all squares, for all side pieces beforehand.

In the game use this index, to look up precomputed bitboard, it tells you all the squares the piece can move with that specific blocker config.

→ it is easier for humans to check the 4 sides but an engine is not just making one more, its exploring endless possibilities, evaluating millions of positions per second. it can figure out legal moves instantly.

before the x op, the occupancy bitboard is a snapshot of all pieces position (yours & opponents) on the board.
The engine keeps separate sets of tables for different piece types, set of magic num for rooks, set for knights.
index for rook on d4 ≠ ind, knight, d4.

Total 21 files:-
i) Bitboard → .cpp / .h
ii) Chessconsole → .vcxproj / .vcxproj.filters
iii) Chessconstants.hpp
iv) Engine → .cpp / .h
v) Main.cpp
vi) Move → .cpp / .h
vii) Movegen → .cpp / .h
viii) Movelist → .cpp / .h
ix) Pregen → .cpp / .h
x) PregeneratedMagics.hpp
xi) Random → .cpp / .h
xii) State → .cpp / .h

Magic bitboard hashing system.
A magic number is the one that perfectly hashes all possible occupancy patterns for that square to unique index.

---

# Page 3

.cpp file:- place where things actually happen
contains real code that runs.
the computer compiles .cpp files into machine code.

.h file:- it tells the compiler that what exists, but not how it works.
it contains function declarations, constants.
no real work happens here.
it tells the compiler, there exists a function called add, takes 2 ints and returns int.

why do we split .h & .cpp:-
its like menu & kitchen
list/dishes → cooks dishes
→ faster compilation
→ clean separation
→ beneficial in large projects.
used as "#include bitboard.h".
if a function is declared but not implemented → linker error
if implemented but not declared → compiler error.

.hpp file:- same as .h → generic header
it is same as C++ specific header.
(just human convention)
.h → constbank, declarations, class definitions
.hpp → constexpr logic, templates
often no .cpp file
cannot live in .cpp file because templates must be visible at compile time.

what is constexpr logic:
const int x = 5
"means x cannot be changed after this"
but the value may still be decided at runtime.
constexpr int x = 5
"this value is known at compile time not runtime"
means → no memory allocation
→ no runtime computation
→ faster program, safer code.
board size, no. of squares, piece values, they never change, so the compiler can precompute & optimise aggressively.
used for magic tables as they must be fixed, cannot be modified at runtime.
it makes bit shift & bit masks fast.
"this value is frozen before program starts"
useful if certain values/calculations done at compile time to improve performance.

.vcxproj?
instructions of visual code on how to build program. (Build config)
it contains which files are part of proj, compiler flags.
.vcxproj.filters?
folder organisation for visual studio UI.
clutching it smooth in the VS view becoming messy.
it decides which files appear under source files or header files.
IDE layout.

Main.cpp ← program Entry
↓
Engine.cpp ← game loop/controller
↓
Movegen.cpp ← move generation logic
↓
Pregen.cpp ← precomputed tables
↓
Bitboard/state/Move ← core DS
↓
chessconstants ← Global constants

main.cpp includes Engine.cpp → only starts the program.
Engine.cpp includes Engine.h (for engine controls), state.h ("game state")
movegen.h & movelist.h (for move generation & execution)
Engine.h includes state.h, movegen.h, movelist.h
movegen.h has movegen.h which has Bitboard, Pregen, State, move, movelist.h
pregen.cpp → pregen.h → bitboard.h → chessconstant.hpp → move.h
Bitboard.cpp → Bitboard.h → chessconstant.hpp → move.cpp → more.h

---

# Page 4

move.cpp movelist.cpp
→ contains .h
→ move.h

state.h
→ Bitboard.h
→ chessconstants.h
→ move.h

random.cpp
→ .h
→ Bitboard.h
→ chessconstants.h

i) bitboard.cpp/.h
role: fundamental data structure of engine.
it represents a chessboard using one 64 bit int.
every other file depends on it.
it is the alphabet of the engine.

ii) chessconstants.hpp
role: dictionary & rulebook
contains: board size, colors, pieces, piece values.

iii) Random.h/.cpp
for random number generation.
used only for finding magic numbers.
only precomputation.

iv) PregeneratedMagics.hpp
role:- stores already found magic numbers.
this avoids recomputation, as "once found never change."

v) Pregen.cpp/.h
role: the engine factory
this file → precomputes everything the engine will need.
→ runs once when movegen is created.
it generates:- → all blocker combinations
→ pawn, knight, king attack tables.
→ bishops & rooks relevant marks / attack table (indexed by magic)
this is when (magic bitboards) actually loads.
after this, sliders become O(1) lookups.

vi) Move.cpp/.h
role: a single chess action
role: defines what a move is?
it packs a move into 32 bits.
stores: source square (6 bits), target square (6 bits), capture, comparison, common, promotion, piece type, captured piece, move ordering score.
A move is data, not logic — of one piece.

vii) Movelist.cpp/.h
role: A container for moves generated.
it stores all legal moves for a position.
sorts them (MVV-LVA)
find moves by source → target
Add moves using Templates
(acts as a shopping cart for moves)
move.cpp → represents one move
movelist → manages many moves

viii) State.h/.cpp
tells the actual position at any moment.
it stores:- 12 bitboard (1 per piece type)
3 occupancy boards (B, W, both)
castling rights
if Bitboard → alphabet
state → sentence

updates state of piece boards.
updates occupancy
updates castle & enpassant info

enpassant:
when a pawn moves two steps forward from its starting rank,
it skips a square, an enemy pawn can move to that block (skipped block)
and capture the pawn as if it moved only one square.
(allowed only on the next move immediately)

ix) Moregen.cpp/.h
stores the rules.
this is where what can move where is decided.
Uses pregen attack tables, magic bitboards for sliders.
checks attacks, castle, enpassant.

movegen for each piece, square:
fetch attack masks,
remove friendly collisions,
add capture moves, it does generation not validation.

→ it generates all moves.
tries each move, make it on a copy of state.
Reject if king is in check.
Recursively evaluate.
Use α-β to prune then pick best move.

x) Engine.cpp/.h
role: the brain
it does: minimax search, alpha-beta pruning.
Evaluation, move Execution, this file thinks, but does not know chess rules.
it asks moregen.

xi) Main.cpp
(entry point): it creates an Engine.
sets initial FEN.
starts game loop.
No brain, just wiring.

Clean minimax.
Easy move ordering.

---

# Page 5

In minimax:-
→ we create a new state of the board, a new state object.
we try to make a move & for(Move move: Move.moves()) on that board, returns vector.
we flip the side to move, so now its opponents turn.
we call minimax again (recursively), now whats the best outcome for the opponent after this move.

α,β pruning:-
eval → evaluation score we just got from the recursive call.
alpha (α) → our best guaranteed score (for maximising player) from elsewhere in tree. (here white)
beta → best score opponent can force.

Why prune when beta <= alpha.
beta = best score minimising player (black) is already guaranteed elsewhere, passed down from parent minimising node, it represents black's current best alternative, i.e. black already has a path better elsewhere.
when β ≤ α, black will never let this position be reached, as it has a better option elsewhere, so no matter how good white's moves get from here, black will choose another branch.

suppose β = 3 (black can guarantee a score of 3 from another branch).
& α = 5, black will never choose this branch because it already has a path that limits white to 3. exploring further is pointless, so prune.

its not white scored 5, black scored 3, so white wins this branch, they both represent boards evaluation from white's perspective.

α = 5 → white found a move in this branch that leads to a board worth +5 for white.
β = 3 → black, at the parent node above, already has a different branch that keeps the score at +3 for white.
black is the minimiser, so black will choose the other branch (score = 3) & will never let white play on this branch. black has the choice of whether to enter a branch at all. white doesn't get to celebrate +5 because black simply refuses to go there.

In a bitboard representation, each type of piece has its own board, which is basically its own 64 bit number, each bit in that no. corresponds to a square on the chessboard, so instead of looping over all 64 squares every time to find a piece, you can just look at the bitboard & quickly see where each piece type is. faster.
So 12 bitboards meaning the board is represented in 12 std::uint64_t's.

why code special:
→ use const & constexpr wherever possible.
→ created a header file to store every reusable num.
→ tried to use correct data type for every scenario. using uint32_t whenever possible (no -ve num needed).
is std::size_t whenever accessing an array or inside a for loop.
→ entire code runs on the stack, don't know if its good or bad because it uses over 2.3mb of stack, but wanted to code to run as fast as possible & most of the data is used by the pregenerated moves which needs to be accessed millions of time per second, so because of that gave a little more than a default stack reserve. (4mb).

Minimax search:
means maximise your advantage, minimise opponents.

int Engine::minimax(const state& state, depth, alpha, beta)
meaning if its white's turn,
try all legal moves &
choose the max evaluation.
if its black, choose the min.

minimax lets the engine see traps, avoid losing material, without it the engine would only look at one move & would blunder.

alpha-beta pruning → to make computer's decision making efficient.
without pruning minimax grows exponentially while searching:-
α → best score current player can guarantee.
β → best score opponent can force.

the computer is looking ahead at all possible moves, like a big tree of decisions.
(depth = how many moves ahead its looking)
a depth of two means its looking at the next move plus the opponents response.
what α-β-p does is it cuts off parts of that tree, it won't look if the branch is not helpful.
it helps focusing on the most promising moves.
it does this by keeping a track of the best score.
it guarantees for that (α) & β.
→ opponents best outcome if they play optimally.

if it finds that looking further down a certain path isn't going to help, because there is already a better option somewhere else, it just stops looking down that path.

More is just a single piece of data that represents one chess move. (It will only hold data about the latest move.)
it tells where a piece came from & other stuff.

A MoveList, basically a collection of those moves. helps keep track. for eg. if my rook is on d4, my movelist will contain all the paths my rook can move to (all the current possibilities), after, we choose a move that is stored into the move file.
move will hold info where rook started & stuff.
also the piece info (which piece).

State
↓
Movegen → Movelist → figures out all the possible moves & the movelist holds them
↓
Minimax loops over Movelist
↓
Move applied → New state

iterativeMinimax fn (manager):
tracks best move.
calls minimax repeatedly.
increases depth gradually.

iii) Engine.cpp [brain + solver + game controller]

i) initialise & manage the engine's internal state.
creates the engine.
loading a board position (with FEN).

ii) Evaluate a chess position numerically.
scoring fn used by minimax.

iii) search for the best move.
(Minimax + α&β pruning)
heart of the engine.

iv) Running game loop.
printing board.
decide turn.
time elapsed.

v) Apply & validate move.
makeMove(-).
kingInCheck(-).
this is rule enforcement system.

---

# Page 6

In C++, memory is divided into heap → dynamic memory, requires OS calls.
& stack → fast, automatic memory, used for local variables, function calls.
2.3mb of stack in the project, most of the data is used up by the pregenerated moves, also dynamically attack tables.

why stack, not heap:- stack access is faster, data accessed millions of time per second.
for us speed > memory flexibility.

stack does not store constant values forever, it stores objects created inside functions, our pregenerated tables live in stack memory only because the objects live on the stack.

string_view:
just a view, a window.
does not own or allocate memory.
only stores pointer to characters & length of string so faster & saves memory.

cstddef: for size_t. Unsigned integer, large enough to hold any memory size, for array indexing & sizing.

when we create an object
→ memory is allocated.
→ constructor runs automatically.
→ Engine is initialised.

A class contains → private:
data members → MorGen m_moregen, state m_state.
only the engine can access or modify these.
public:
member function variables.

default constructor:- doesn't take any arguments, if want to create an Engine without specifying any starting condition.
second, overloaded constructor, initialise it using FEN string.
job is to set up object, give initial values. if illegal → move.

default constructor is in case, if we want to create an Engine without any starting position, create object first then set its state later. more flexible.

void setState, accesses the same Engine object.
x loads a new position, without creating new Engine.

static void xyz(State& state) const:
belongs to the class not object.
don't need object to call it, doesn't need data about the current object. reference to the object of the State class.
if fn(const State& state) → send only reference to a state object.
function will not modify the object, is called on. only read.

i) Int main() calls the constructor of Engine, & the constructor takes a FEN string.
create a chess Engine & load this position.
Engine.step() takes 3 values → which side the engine plays.
flip board when displaying.
depth. (8 ply)

ii) Engine.h
int m_time_check_count: counts nodes before checking time, so don't call clock too often.
m_searchStartTime:- when Engine started thinking, used to calculate elapsed time, set once per move.
m_seconds: holds duration, total time for a move, elapsed time stored.

a) default constructor:- doesn't take any arguments, if want to create an Engine object without specifying any starting condition.
second, overloaded constructor, initialise it using FEN string.
job is to set up object, give initial values. if illegal → move.

b) default constructor is in case, if we want to create an Engine without any starting position, create object first then set its state later. more flexible.

c) void setState, accesses the same Engine object.
x loads a new position, without creating new Engine.

d) static void xyz(State& state) const:
belongs to the class not object.
don't need object to call it, doesn't need data about current object.
reference to the object of the State class.
if fn(const State& state) → send only reference to a state object.
function will not modify the object is called on. only read.

bool inputAndParseMove(MoveList& list, Move& move)
→ reference to the class, don't want to copy as slow & wasteful.
the function will fill this object. (move object)
if move exists in Movelist
true → write into Move.

bool MakeMove(const Move move)
Move is very small (32 bit data), copying cheap.
this fn applies move to the board & updates state.

---

# Page 7

minimax fn → returns an evaluation score.
explores game tree.
it does not decide the final move.
does not track best move globally.
only evaluates one branch.

iterativeMinimax fn (manager):
tracks best move.
calls minimax repeatedly.
increases depth gradually.

iii) Engine.cpp [brain + solver + game controller]

i) initialise & manage the engine's internal state.
creates the engine.
loading a board position (with FEN).

ii) Evaluate a chess position numerically.
scoring fn used by minimax.

iii) search for the best move.
(Minimax + α&β pruning)
heart of the engine.

iv) Running game loop.
printing board.
decide turn.
time elapsed.

v) Apply & validate move.
makeMove(-).
kingInCheck(-).
this is rule enforcement system.

full engine lifecycle:-
main.cpp
↓
Engine created (constructor)
↓
board initialised
↓
step()

print board
while game not over:
if engine's turn
minimax
apply move
else
used user move
validate & apply
print state
switch side

the same Engine instance is used throughout the game session.

Scoring fn → evaluate:
looks at all the pieces on board,
adds the value & how much each piece can move, all possible moves,
then minimax algo, engine maximises its score & minimises opponents.

setState is used to load a saved pos.
not typically when you move a new piece, makeMove handles that. Using state object.
setState → start of game
→ resetting board.

reference is just another name for existing object, existing variable, another name.
not a pointer:- pointer holds memory address, can be changed to point somewhere else. reference once set always refers to same variable.

enum (enumeration) user defined class in c++, give it a lot of possible enum names (enumerators), each corresponds to a int val.

void Engine::setState → defining how the function works.
you are saying "this is what the setState function of the engine class does, you are not calling it on any object yet. just defining."
engine.setState(fen) → actually using the fn, engine is object (instance) of Engine class.
you define with class & use with object.
so in engine.h → we show its existence, declare it.
.cpp → define the function.

member variables are variables that belong to object of class.
static function can't access member variable because it doesn't have this → pointer.

Engine::Engine()
: m_moregen(), m_depth()... {}
→ this line is constructor definition for Engine class.
creates empty engine.
→ this is called member initialisation list.
because C++ initialises member variables before constructor body runs.
before engine thinks, set up internal machinery.

if default constructor didn't exist, we could not safely create an Engine without FEN.
Debugging impossible.
internal variables might be uninitialised.

"All member variables of the Engine class are declared in Engine.h,
initialised when Engine object is created (using constructor defined in engine.cpp)
used & modified by member functions implemented in engine.cpp."

Bitboard piece_board = state.positions()[piece]
class type → variable name → object of type (member function call) → bitboard → returns an array of 12 bitboards.

static_cast<int>(attack.bitcount())
→ no. of squares a particular piece can attack, returns unsigned type.
convert to int. (explicit conversion)

if(king_square == size_max) defensive programming guard.
if king is missing from the board, something went very wrong earlier.

void Engine::printBoard(—) const
→ thin wrapper for the engine to print the board.
why it exists: so that external code doesn't access m_state directly, safe API.

bool input & pawn
→ reads a line of text, converts, checks whether it matches a legal move, if yes, fills Move object.

squareToIndex(string_view square)
rank(horizontal) = 7-(square[1] - '1')
file(vertical) = square[0] - 'a'
in static_cast<std::size_t>
index = rank * Rank_Max + file
{ rows x 8 + column}

double colon operator, belongs to Engine class.
you define with class & use with object.

---

# Page 8

if default constructor didn't exist, we could not safely create an Engine without FEN.
Debugging impossible.
internal variables might be uninitialised.

void Engine::printBoard(—) const
→ thin wrapper for the engine to print the board.
why it exists: so that external code doesn't access m_state directly, safe API.

bool input & pawn
→ reads a line of text, converts, checks whether it matches a legal move, if yes, fills Move object.

squareToIndex(string_view square)
rank(horizontal) = 7-(square[1] - '1')
file(vertical) = square[0] - 'a'
in static_cast<std::size_t>
index = rank * Rank_Max + file
{ rows x 8 + column}

→ how to ensure double pawn happens once?
if it is on rank 2 or 7.
condition checked during move generation.
double pawn flag only set when legal.
source (WP) → (BP in prev move earlier)
(current move) → target (target+8)
if(whiteToMove()) → blackPawn

In bool MakeMove:
→ state.setEnpassantSquare(no_sqr)
enpassant is one square only, stored one index,
if a pawn moves here, enpassant capture is allowed.
if no enpassant possible we store no_sqr.
we assume before applying a move, enpassant is not available.
we only set it when "double pawn" move.

→ how do we know in if(castle) or if(captured)
if it is a castle variable set?
The move object knows, it is encoded in Movegen. Move contains flags - castle, capture.
every legal move stored.
"const bool castle = move.castle()."
MoveGen::GenerateMoves() sets these flags.

→ state.setCastleRights(source) / (target).
uses lookup tables, if square is a1, e1, h1, a8, e8, h8
castling rights removed.
if source/target = c2, c4 -- → not rook/king squares.
lookup table leaves castling rights unchanged.

---

# Page 9

In minimax:-
→ we create a new state of the board, a new state object.
we try to make a move & for(Move move: Move.moves()) on that board, returns vector.
we flip the side to move, so now its opponents turn.
we call minimax again (recursively), now whats the best outcome for the opponent after this move.

α,β pruning:-
eval → evaluation score we just got from the recursive call.
alpha (α) → our best guaranteed score (for maximising player) from elsewhere in tree. (here white)
beta → best score opponent can force.

Why prune when beta <= alpha.
beta = best score minimising player (black) is already guaranteed elsewhere, passed down from parent minimising node, it represents black's current best alternative, i.e. black already has a path better elsewhere.
when β ≤ α, black will never let this position be reached, as it has a better option elsewhere, so no matter how good white's moves get from here, black will choose another branch.

suppose β = 3 (black can guarantee a score of 3 from another branch).
& α = 5, black will never choose this branch because it already has a path that limits white to 3. exploring further is pointless, so prune.

its not white scored 5, black scored 3, so white wins this branch, they both represent boards evaluation from white's perspective.

α = 5 → white found a move in this branch that leads to a board worth +5 for white.
β = 3 → black, at the parent node above, already has a different branch that keeps the score at +3 for white.
black is the minimiser, so black will choose the other branch (score = 3) & will never let white play on this branch. black has the choice of whether to enter a branch at all. white doesn't get to celebrate +5 because black simply refuses to go there.

---

# Page 10

ChessConstants.hpp
cstdint → gives fixed width integers on different machines.
chessconstants.hpp is the foundation — every single file includes it, defines the shared language of the entire program.

constexpr array<uint8_t, Max_board_pos>
→ 64
castling_rights = {
a8 → 7 (black queenside rook)
e8 → 3 (black king)
h8 → 11 (black kingside rook)
a1 → 13
e1 → 12 (white king)
h1 → 14
rest 15
}

so when setCastleRights(square) is called:
m_castleRights &= castling_rights[square]
it ands current rights with the precomputed mask.
if the mask has a 0 in a bit position, that castle rights get permanently revoked.

enum castle {
WK = 0b0001
(white kingside = bit0)
WQ = 0b0010
BK = 0100
BQ = 0b1000
} means binary, binary literal is a C++14 feature.

the m_castleRights bytes look like this:
bit3 bit2 bit1 bit0
BQ BK WQ WK
1 1 1 1 = 0b1111 = 15
nothing special about this square.

12 → 0b1100, if anything moves to/from e1 white loses both castling rights.
0b1100 & 0b1111 = 0b1100 → WQ & WK become 0.

14 → 0b1110 = h1, if anything moves to/from h1, white loses kingside castling. clears bit 0 (wk).

Each castle type is a unique bit.

piece value for white = +ve, as white wants to maximise the score.
black = -ve (good for black) wants to minimise.
this is the negamax convention.
the values are in centipawns (1 pawn = 100 points)
allows fractional pawn advantage.

king value = 0, can never be captured, assigning value will distort the evaluation.

enums auto increment.

some functions like _bitscanforward64(index, x) maps to the x86 BSF (bit scan forward) function. (CPU intrinsic) (CPU instruction) (machine instruction).
In modern intel processors, this instruction executes in a single clock cycle.

---

# Page 11

Random class:
uses Mersenne twister: std::mt19937_64
m_rng:- the mersene twister engine object.
→ member variable.
it holds the internal state of the generator (a large array of values that get updated with each number). It is initialised with a seed in the constructor.

Random::Random()
: m_rng(std::time(NULL))
{ srand(static_cast<unsigned int>(std::time(NULL))); }
→ member initialisation list, this initialises m_rng.

when m_rng is std::mt19937_64 object, time(NULL) returns the time elapsed since 1 Jan, 1970 (1.7 billion sec). (called Unix epoch)
this calls the mt19377 constructor with time value as seed.
srand() takes unsigned & time returns time_t (64 bit int)
→ 32 so truncation needed.

m_rng() advances the internal state.

"const std::array<bitboard, 12>& positions() const;"
this method doesn't modify state, & returns a reference to the position array that the reader can read but not modify.

void setEnpassantSquare — sets enpassant square.
is called in two situations:
→ at the start of makeMove call, clear enpassant.
→ when a double pawn push happens, sets enpassant target square.

static state parse_fen(const std::string_view fen)
→ create a state from a FEN string
i) splits the FEN by / into 8 rank strings
ii) iterates thru each character.
iii) if its a digit skip that many squares
iv) if its a letter, looks up piece type using char_to_piece[] & calls setpiece.

The Invariant:
at all times, m_occupancy[Both]
= m_occupancy[White] | m_occupancy[Black]
(union)
setprice, pop, movequit & movecapture must maintain this invariant.

this is why the piece manipulation methods are encapsulation.

state:
state::state(—)
: m_positions(state.m_positions)
→ calls copy constructor. It copies 12 bitboard elements one by one, each to uint_64t, fast 96 byte memory copy.

bool state::testCastleRights(contr castle C)
return static_cast<bool>
(m_castleRight & C)
for eg: 0b1100 castle::wk
binary literal prefix → 0b0001
false (white kingside castle not available)

State state:
creates a default (empty) state, all bitboards zero, white to move, all castling rights available. filled when we parse the FEN.

return state:- returns fully populated state by value. (Return Value Optimisation)

---

# Page 12

Move
struct Move {
int source; — 4 bytes
int target; — 4 bytes
int piece: — 4 bytes
bool x 4 — 1x4 bytes
int score — 4 bytes
} // total: atleast 20 bytes per move.
generates & evaluates 20 millions moves per second during search, hundreds of moves per position.

A single uint32_t fits in a CPU register & is cache friendly.

bit layout:
28 21 20 19 16 15 14 13 12 11 6 5 0
move score | piece type | cas en cap prm | target | source square
| double pawn push |

unpack using masks:- (piece)((m_data & 0xF0000) >>16)
(bool)(m_data & 0x2000)

Six constructors handle each distinct move type.

not only can the (with the encode decode method) system store the entire data about the move in a single integer
but it can also sort, compare & copy moves as single integer operation.

MoveList:
an array lives on the stack, no heap allocation.
array<T, N>: must know N before the program runs.
in vector, size is determined at runtime, allocates memory on heap.
it allocates a chunk of memory, fills it, & when it runs out, allocates bigger chunk, copies everything over & continues.

reserve(256) pre allocates space for 256 moves upfront. prevents reallocation.

Internally a vector has 3 values:
→ pointer to heap allocated array.
→ size (how many elements currently stand).
→ capacity (space how much currently allocated).

if no reserve used: 8 reallocations & 8 heap allocations
just to generate one positions moves, each reallocation copies all existing elements.

push_back(obj) - takes an already constructed object & copies or moves it into vector.
emplace_back - takes constructor arguments & constructs the object directly inside vector memory.

Swap & pop trick:
instead of naively copying/swapping two deleting a move in the movelist which results in copying potentially all remaining elements [O(n)]
→ we swap the element to be deleted with the last element & delete it then [O(1)]
order changes but for Movelist it doesn't matter because the list gets sorted anyway.

---

# Page 13

Pregen — 64 squares
m_bishopAttackMask = 64 arrays x 512 bitboards each = 32768 bitboards
= 2^9
for Rooks = 64 x 4096 = 262144 bitboards
each bitboard is 8 bytes → 262144 x 8 = 2,097,152 = 2mb
thats why we had to increase the default stack size to 4mb, these tables live on stack.

pregen builds all attack lookup tables.
slider pieces when attacks are changed with every blocking piece configuration, attack tables are indexed via magic number hashing.
gives collision free index.

"if constexpr" for zero cost compile time branching between bishop & rook behaviour.

Template instantiation requires full body to be visible at call site.

the total precomputed data is ≈ 2.3mb, dominated by attack tables.

Pregen::createOccupancies()
bishop on d4 with 9 relevant squares, enumerate all 512 possible patterns of which of those squares is occupied.
inner loop → 0-64, when it finds a square that is in the mask (relevant)
check corresponding bit in i, if bit is set square is occupied.
outer loop i = 0 → num of occupancies - 1, i is a binary counter, which of (0 - 511) (2^9 - 1) relevant squares occupied.

say only 3 relevant squares 10, 20, 30.
i = 2 (binary: 010)
bit_position 0 (for square 10): i & (1<<0) = 0 → not set
1 (for square 20): i & (1<<1) = 1 → set
3 → not set
result: only square 20 set.
i = 3 (011):- 10 & 20 square set.
this is the enumeration trick.

why index right shift by 64-N, this keeps only top N bits of multiplication result, discarding bottom 64-N, resulting no. b/w 0 to 2^N-1.
0 - 2^N-1 → exactly in range of valid indices of lookup table.
perfect Hash function → magic hashed index → is stored.
for every occupancy pattern, the attack bitboard is precomputed.

---

# Page 14

m_pregen.bishopAttacks()[square][magic_index]
→ selects precomputed attack table bitboard for exact occupancy config.
→ returns reference to 64x512 array.
→ selects row for this square.
→ returns a bitboard containing all squares this bishop can attack given current board state.

Moregen:- Magic bitboard pipeline

startup: runs once when pregen::pregen() constructor is called.
for each of 64 squares:

step 1: compute relevant bits mask
createBishopAttackMasks()/Rook
walk rays from square, set bits for every square that can block this piece (not edges).
result stored in m_bishopRelevantBits[square]

↓

step 2: count relevant bits N
createBishopBitCount()
uses _popcnt64() on result of step 1, bishop needs 2^N table slots.
results in m_bishopBitCount[sq]

↓

step 3: load or find Magic num
createBishopMagics()
findMagicNumber()
createOccupancies(mask, numbits, numoccupancies)
results in m_bishopMagic[sq]

↓

step 5: compute attacks, hash, store
createBishopAttacks()
5a. compute actual attacks (uses edges, ray walking)
5b. compute magic index
5c. store in m_bishopAttackMask[sq][magic_ind]
= attack_bitboard
storing whole bitboard.

during gameplay, called millions of time per second:
MoveGen::getBishopAttack(sq, occupancy)
uses tables built in all 4 steps.
i) masked = occupancy & m_bishopRelevantBits[sq]
→ Pregen::maskRelevantBits<piece::Bishop>(occ, sq)
ii) ind = (masked * m_bishopMagic[sq]) >> 64-N[sq]
iii) return m_bishopAttackMask[sq][ind] → Attack table

---

# Page 15

reverse attack trick (for attack generation)
to check which squares/elements this piece is being attacked, instead of:
→ loop through every enemy piece
→ generate its attack
→ check if attack lands on square.

we instead:
→ generate attacks from square, as if it were each piece type.
→ check if those attacks intersect with actual enemy pieces.

so to check if "square" is attacked by an enemy knight, generate knight attacks from square, then AND with the enemy knights attackboard, if AND is non-zero, there's a knight that attacks this square.

Engine::minimax()
→ MoveGen::GenerateMoves — generates all pseudo legal moves.
→ Movelist — sorts all moves.

→ for each move in moves:
→ Engine::makeMove — applies move to copied state.
→ Engine::kingInCheck — checks legality.
→ MoveGen::isSquareAttacked
→ Engine minimax(depth-1) — recursion.
