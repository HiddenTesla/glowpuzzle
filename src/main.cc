/*
Background problem (The 3 images cannot be shown in source file):
There're 7 red tiles,
8 blue titles and one white title in a 4 x 4 plane.
We could only move the white tile.
When moving it, the white tile swaps the position with the adjacent tile.
L, R, U, D are corresponding to four directions which the tile could be moved to (Left, Right, Up, Down)
For example, starting from configuration (S), by the move sequence RDRDL we reach the configuration (E).
Now, starting from configuration (S), find the shortest way to reach configuration (T).
*/

// Reason for using C++ (not C):
// Some STL containers are needed
// otherwise have to implement your own container

#include "puzzle.hh"

int main(void)
{
    Puzzle puzzle;
    puzzle.solve();

    return 0;
}
