#include "puzzle.hh"

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>

#define CORD_FMT        "(%d, %d)"

bool TileSet::read(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Cannot open '%s'\n", filename);
        return false;
    }

    int row, column;
    char ch;
    bool whiteSet = false;

    for (row = 0; row < MAP_DIMENSION; ++row) {
        column = 0;

        // Read char by char instead of a whole line (maybe the latter is better?)
        // Not count '\n' into column, so while loop is preferred here
        while (column < MAP_DIMENSION) {
            if (feof(fp)) {
                printf("Reach unexpected eof\n");
                fclose(fp);
                return false;
            }

            fscanf(fp, "%c", &ch);
            if (ch == '\n') {
                continue;
            }

            switch (ch) {
            case 'w': case 'W':
                if (whiteSet) {
                    printf("Second white tile found! Previously at (%d, %d)\n",
                        mWhiteRow, mWhiteColumn);
                    fclose(fp);
                    return false;
                }
                else {
                    mWhiteRow = row;
                    mWhiteColumn = column;
                    whiteSet = true;
                    mTiles[row][column] = Tile::WHITE;
                    break;
                }

            case 'r': case 'R':
                mTiles[row][column] = Tile::RED;
                break;

            case 'b': case 'B':
                mTiles[row][column] = Tile::BLUE;
                break;

            default:
                printf("Invalid char with ASCII %d\n", ch);
                return false;
            }
            ++column;
        }
    }
    fclose(fp);
    return true;
};

void TileSet::print() const
{
    for (int row = 0; row < MAP_DIMENSION; ++row) {
        for (int column = 0; column < MAP_DIMENSION; ++column) {
            switch (mTiles[row][column]) {
            case Tile::WHITE:
                printf("W ");
                break;
            case Tile::RED:
                printf("R ");
                break;
            case Tile::BLUE:
                printf("B ");
                break;
            default:
                break;
            }
        }
        printf("\n");
    }
    printf("\nWhite at " CORD_FMT "\n\n", mWhiteRow, mWhiteColumn);
}

void TileSet::move(Tile::Direction dir)
{
    if (!canMove(dir)) {
        printf("Can't move %d with white at " CORD_FMT "\n",
            dir, mWhiteRow, mWhiteColumn);
        return;
    }

    switch (dir) {
    case Tile::UP:
        std::swap(mTiles[mWhiteRow][mWhiteColumn], mTiles[mWhiteRow - 1][mWhiteColumn]);
        --mWhiteRow;
        break;
    case Tile::DOWN:
        std::swap(mTiles[mWhiteRow][mWhiteColumn], mTiles[mWhiteRow + 1][mWhiteColumn]);
        ++mWhiteRow;
        break;
    case Tile::RIGHT:
        std::swap(mTiles[mWhiteRow][mWhiteColumn], mTiles[mWhiteRow][mWhiteColumn + 1]);
        ++mWhiteColumn;
        break;
    case Tile::LEFT:
        std::swap(mTiles[mWhiteRow][mWhiteColumn], mTiles[mWhiteRow][mWhiteColumn - 1]);
        --mWhiteColumn;
        break;
    default:
        break;
    }
}

bool TileSet::canMove(Tile::Direction dir) const
{
    switch (dir) {
    case Tile::UP:
        return (mWhiteRow > 0);
    case Tile::DOWN:
        return (mWhiteRow < MAP_DIMENSION - 1);
    case Tile::RIGHT:
        return (mWhiteColumn < MAP_DIMENSION - 1);
    case Tile::LEFT:
        return (mWhiteColumn > 0);
    default:
        return false;
    }
}

void TileSet::record(Tile::Direction dir)
{
    switch (dir) {
    case Tile::UP:
        mPath += 'U';
        break;
    case Tile::DOWN:
        mPath += 'D';
        break;
    case Tile::RIGHT:
        mPath += 'R';
        break;
    case Tile::LEFT:
        mPath += 'L';
        break;
    default:
        break;
    }
}

// Frustrating comparason functions
// Need by std::set
// memcmp seems not to work
bool TileSet::operator ==(const TileSet& ref) const
{
    for (int row = 0; row < MAP_DIMENSION; ++row) {
        for (int column = 0; column < MAP_DIMENSION; ++column) {
            if (this->mTiles[row][column] != ref.mTiles[row][column]) {
                return false;
            }
        }
    }
    return true;
}

bool TileSet::operator <(const TileSet& ref) const
{
    for (int row = 0; row < MAP_DIMENSION; ++row) {
        for (int column = 0; column < MAP_DIMENSION; ++column) {
            if (this->mTiles[row][column] < ref.mTiles[row][column]) {
                return true;
            }
            else if (this->mTiles[row][column] > ref.mTiles[row][column]) {
                return false;
            }
        }
    }

    return false;
}

Puzzle::Puzzle()
{
    if (!mStart.read("start.input")) {
        exit(1);
    }

    if (!mTarget.read("target.input")) {
        exit(1);
    }
}

void* printBroad(void* arg)
{
    const std::set<TileSet>* p = 
        (const std::set<TileSet>*)(arg);

    while (true) {
        sleep(5);
        printf("%d status traversed\n", (int)(p->size()));
    }
    return 0;
}

bool Puzzle::solve()
{
    TileSet currentStatus;
    TileSet nextStatus;

    // history is used to store all the status that have been encountered
    // It only adds and never erases elements, unlike the queue
    // If using list, or vector, then searching will become extremely slow
    // as the container size grows because these are O(n)
    // Search for set is O(log(n)), much faster
    // The whole programs finishes within one second
    std::set<TileSet> history;

    // This better be std::queue, but I used to think it needs to be tranversed,
    // so list is used instead
    std::list<TileSet> queue;
    history.insert(mStart);
    queue.push_front(mStart);

    pthread_t tr;
    pthread_create(&tr, 0, printBroad, &history);

    // Here comes the main loop
    // Algorithm:
    // First push the start status into queue
    // While queue is not empty, pop
    // If pop equals the target status, done
    // Try move all directions
    // If the move comes to a new status, push the new status into queue,
    // as well as storing the new status
    while (!queue.empty()) {
        currentStatus = queue.back();
        queue.pop_back();

        if (currentStatus == mTarget) {
            printf("%s\n", currentStatus.mPath.c_str());
            return true;
        }

        for (int dir = Tile::UP; dir <= Tile::LEFT; ++dir) {
            nextStatus = currentStatus;

            if (!nextStatus.canMove((Tile::Direction)(dir))) {
                continue;
            }

            nextStatus.move( (Tile ::Direction)(dir) );
            // Put into history only if the status has not been met before
            if ( history.find(nextStatus) == history.end() ) {
                nextStatus.record( (Tile ::Direction)(dir) ); 
                queue.push_front(nextStatus);
                history.insert(nextStatus);
            }
        }
    }
    printf("\n\nNo path found. %d status traversed\n", (int)history.size());
    return false;
}

