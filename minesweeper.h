#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <random>
#include <string>
#include <vector>

namespace minesweeper {

enum class GameStatus { Ready, InProgress, Won, Lost };

struct Position {
    int row;
    int column;
};

bool operator==(Position left, Position right);

struct CellView {
    bool revealed;
    bool flagged;
    bool mine;
    int adjacentMines;
};

struct ActionResult {
    bool accepted;
    bool changed;
    GameStatus status;
    std::string message;
};

class MinesweeperGame {
public:
    static constexpr int kRows = 9;
    static constexpr int kColumns = 9;
    static constexpr int kMineCount = 10;

    MinesweeperGame();

    static MinesweeperGame withMineLayoutForTesting(
        const std::vector<Position>& candidateMines);

    ActionResult reveal(Position position);
    ActionResult toggleFlag(Position position);
    CellView cellAt(Position position) const;
    GameStatus status() const;
    int rows() const;
    int columns() const;
    int mineCount() const;

private:
    struct Cell {
        bool mine = false;
        bool revealed = false;
        bool flagged = false;
        int adjacentMines = 0;
    };

    MinesweeperGame(const std::vector<Position>& candidateMines,
                    bool useCandidateMines);

    static bool isValid(Position position);
    static int indexOf(Position position);
    static Position positionOf(int index);
    static std::vector<Position> neighbors(Position position);

    ActionResult rejected(const std::string& message) const;
    void initializeMines(Position firstReveal);
    void calculateAdjacentMines();
    void revealSafeArea(Position start);
    void revealAllMines();
    bool hasWon() const;

    std::vector<Cell> cells_;
    std::vector<Position> candidateMines_;
    std::mt19937 randomEngine_;
    GameStatus status_ = GameStatus::Ready;
    bool useCandidateMines_ = false;
    bool minesInitialized_ = false;
};

}  // namespace minesweeper

#endif
