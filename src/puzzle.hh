#ifndef _PUZZLE_HH_
#define _PUZZLE_HH_

#include <list>
#include <string>
#include <set>

namespace Tile
{

enum TileColor
{
    WHITE,
    RED,
    BLUE,
};

enum Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

} //namespace Tile

#define MAP_DIMENSION   4

class Puzzle;

class TileSet
{
public:
    friend class Puzzle;

    bool read(const char*);
    bool canMove(Tile::Direction) const;
    void move(Tile::Direction);
    void print() const;
    void record(Tile::Direction);

    bool operator ==(const TileSet&) const;
    bool operator <(const TileSet&) const;

private:
    std::string mPath; // Record the path to reach this status
    Tile::TileColor mTiles [MAP_DIMENSION] [MAP_DIMENSION];
    int mWhiteRow;
    int mWhiteColumn;

};

class Puzzle
{
public:
    Puzzle();
    bool solve();

private:
    TileSet mStart;
    TileSet mTarget;

};

#endif
