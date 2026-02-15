// Chess_Console by Kshitij Bhatnagar

Thank you for checking out my chess engine!  I was super proud of this project, ,CChess (stands for console chess).

followed tutorial (or shall i say inspired) by Nicholas Maurer

to input a normal move you need to give the starting position and the ending position in all lowercase with no spaces, it cannot parse real 
chess moves like e4 or bxh6. some examples of moves it accepts are d2d4, d7d5, c1f4. If you give a move in the wrong format or a move that doesnt exist the 
engine will display "move does not exist". Even though the engine will display an x inbetween the positions when capturing it cannot 
parse that as an input so even when capturing only input the start and end position. To castle you have four options:
bk - black kings side castle
bq - black queens side castle
wk - white kings side castle
wq - white queens side castle

 This new engine is better in almost every way! The C++ code is more organized and more readable. The algorithms are way more efficient, especially the MoveGen::generateMoves(). It also has a GUI! And a static library with a C-style interface.