#include "minesweeper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <queue>
#include <stdexcept>

namespace minesweeper {

namespace {

constexpr std::array<int, 8> kRowOffsets = {-1, -1, -1, 0, 0, 1, 1, 1};
constexpr std::array<int, 8> kColumnOffsets = {-1, 0, 1, -1, 1, -1, 0, 1};

std::mt19937 makeRandomEngine() {
    const auto now = std::chrono::high_resolution_clock::now();
    const auto seed = static_cast<unsigned int>(
        now.time_since_epoch().count());
    return std::mt19937(seed);
}

}  // namespace

bool operator==(Position left, Position right) {
    return left.row == right.row && left.column == right.column;
}

MinesweeperGame::MinesweeperGame()
    : cells_(static_cast<std::size_t>(kRows * kColumns)),
      randomEngine_(makeRandomEngine()) {}

MinesweeperGame MinesweeperGame::withMineLayoutForTesting(
    const std::vector<Position>& candidateMines) {
    return MinesweeperGame(candidateMines, true);
}

MinesweeperGame::MinesweeperGame(
    const std::vector<Position>& candidateMines, bool useCandidateMines)
    : cells_(static_cast<std::size_t>(kRows * kColumns)),
      candidateMines_(candidateMines),
      randomEngine_(1U),
      useCandidateMines_(useCandidateMines) {
    if (candidateMines_.size() > static_cast<std::size_t>(kMineCount)) {
        throw std::invalid_argument("test layout has too many mines");
    }
    for (Position position : candidateMines_) {
        if (!isValid(position)) {
            throw std::invalid_argument("test layout contains invalid position");
        }
        const auto duplicate = std::count(candidateMines_.begin(),
                                           candidateMines_.end(), position);
        if (duplicate != 1) {
            throw std::invalid_argument("test layout contains duplicate position");
        }
    }
}

bool MinesweeperGame::isValid(Position position) {
    return position.row >= 0 && position.row < kRows && position.column >= 0 &&
           position.column < kColumns;
}

int MinesweeperGame::indexOf(Position position) {
    return position.row * kColumns + position.column;
}

Position MinesweeperGame::positionOf(int index) {
    return {index / kColumns, index % kColumns};
}

std::vector<Position> MinesweeperGame::neighbors(Position position) {
    std::vector<Position> result;
    result.reserve(kRowOffsets.size());
    for (std::size_t offset = 0; offset < kRowOffsets.size(); ++offset) {
        Position neighbor = {position.row + kRowOffsets[offset],
                             position.column + kColumnOffsets[offset]};
        if (isValid(neighbor)) {
            result.push_back(neighbor);
        }
    }
    return result;
}

ActionResult MinesweeperGame::rejected(const std::string& message) const {
    return {false, false, status_, message};
}

void MinesweeperGame::initializeMines(Position firstReveal) {
    std::vector<Position> selected;
    selected.reserve(kMineCount);

    if (useCandidateMines_) {
        for (Position position : candidateMines_) {
            if (!(position == firstReveal) &&
                std::find(selected.begin(), selected.end(), position) ==
                    selected.end()) {
                selected.push_back(position);
            }
        }
    } else {
        std::vector<Position> available;
        available.reserve(kRows * kColumns - 1);
        for (int index = 0; index < kRows * kColumns; ++index) {
            Position position = positionOf(index);
            if (!(position == firstReveal)) {
                available.push_back(position);
            }
        }
        std::shuffle(available.begin(), available.end(), randomEngine_);
        selected.assign(available.begin(), available.begin() + kMineCount);
    }

    for (int index = 0; index < kRows * kColumns &&
                           static_cast<int>(selected.size()) < kMineCount;
         ++index) {
        Position position = positionOf(index);
        if (position == firstReveal ||
            std::find(selected.begin(), selected.end(), position) !=
                selected.end()) {
            continue;
        }
        selected.push_back(position);
    }

    for (Position position : selected) {
        cells_[static_cast<std::size_t>(indexOf(position))].mine = true;
    }
    calculateAdjacentMines();
    minesInitialized_ = true;
}

void MinesweeperGame::calculateAdjacentMines() {
    for (int index = 0; index < kRows * kColumns; ++index) {
        Position position = positionOf(index);
        int count = 0;
        for (Position neighbor : neighbors(position)) {
            if (cells_[static_cast<std::size_t>(indexOf(neighbor))].mine) {
                ++count;
            }
        }
        cells_[static_cast<std::size_t>(index)].adjacentMines = count;
    }
}

void MinesweeperGame::revealSafeArea(Position start) {
    std::queue<Position> pending;
    pending.push(start);

    while (!pending.empty()) {
        Position position = pending.front();
        pending.pop();
        Cell& cell = cells_[static_cast<std::size_t>(indexOf(position))];
        if (cell.revealed || cell.flagged || cell.mine) {
            continue;
        }

        cell.revealed = true;
        if (cell.adjacentMines != 0) {
            continue;
        }
        for (Position neighbor : neighbors(position)) {
            const Cell& neighborCell =
                cells_[static_cast<std::size_t>(indexOf(neighbor))];
            if (!neighborCell.revealed && !neighborCell.flagged &&
                !neighborCell.mine) {
                pending.push(neighbor);
            }
        }
    }
}

void MinesweeperGame::revealAllMines() {
    for (Cell& cell : cells_) {
        if (cell.mine) {
            cell.revealed = true;
        }
    }
}

bool MinesweeperGame::hasWon() const {
    int revealedSafeCells = 0;
    for (const Cell& cell : cells_) {
        if (cell.revealed && !cell.mine) {
            ++revealedSafeCells;
        }
    }
    return revealedSafeCells == kRows * kColumns - kMineCount;
}

ActionResult MinesweeperGame::reveal(Position position) {
    if (!isValid(position)) {
        return rejected("Coordinates are outside the board.");
    }
    if (status_ == GameStatus::Won || status_ == GameStatus::Lost) {
        return rejected("The game is already over.");
    }

    Cell& cell = cells_[static_cast<std::size_t>(indexOf(position))];
    if (cell.flagged) {
        return rejected("Unflag the cell before revealing it.");
    }
    if (cell.revealed) {
        return rejected("That cell is already revealed.");
    }

    if (!minesInitialized_) {
        initializeMines(position);
        status_ = GameStatus::InProgress;
    }

    if (cell.mine) {
        cell.revealed = true;
        revealAllMines();
        status_ = GameStatus::Lost;
        return {true, true, status_, "You hit a mine."};
    }

    revealSafeArea(position);
    if (hasWon()) {
        status_ = GameStatus::Won;
        return {true, true, status_, "You cleared the board."};
    }
    return {true, true, status_, "Cell revealed."};
}

ActionResult MinesweeperGame::toggleFlag(Position position) {
    if (!isValid(position)) {
        return rejected("Coordinates are outside the board.");
    }
    if (status_ == GameStatus::Won || status_ == GameStatus::Lost) {
        return rejected("The game is already over.");
    }

    Cell& cell = cells_[static_cast<std::size_t>(indexOf(position))];
    if (cell.revealed) {
        return rejected("Revealed cells cannot be flagged.");
    }
    cell.flagged = !cell.flagged;
    return {true, true, status_, cell.flagged ? "Cell flagged." : "Flag removed."};
}

CellView MinesweeperGame::cellAt(Position position) const {
    if (!isValid(position)) {
        throw std::out_of_range("Coordinates are outside the board.");
    }
    const Cell& cell = cells_[static_cast<std::size_t>(indexOf(position))];
    return {cell.revealed, cell.flagged, cell.mine, cell.adjacentMines};
}

GameStatus MinesweeperGame::status() const { return status_; }

int MinesweeperGame::rows() const { return kRows; }

int MinesweeperGame::columns() const { return kColumns; }

int MinesweeperGame::mineCount() const { return kMineCount; }

}  // namespace minesweeper
