#include "../minesweeper.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

using minesweeper::ActionResult;
using minesweeper::CellView;
using minesweeper::GameStatus;
using minesweeper::MinesweeperGame;
using minesweeper::Position;

const std::vector<Position> kMineLayout = {
    {0, 1}, {1, 1}, {2, 2}, {3, 3}, {4, 4},
    {5, 5}, {6, 6}, {7, 7}, {8, 8}, {0, 8},
};

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void expectStatus(const ActionResult& result, GameStatus status,
                  const std::string& message) {
    expect(result.status == status, message);
}

MinesweeperGame makeGame() {
    return MinesweeperGame::withMineLayoutForTesting(kMineLayout);
}

void testInitialStateAndPreRevealFlags() {
    MinesweeperGame game = makeGame();

    expect(game.rows() == 9, "game has nine rows");
    expect(game.columns() == 9, "game has nine columns");
    expect(game.mineCount() == 10, "game has ten mines");
    expect(game.status() == GameStatus::Ready, "game starts ready");
    expect(!game.cellAt({0, 0}).revealed, "cells start hidden");

    ActionResult flag = game.toggleFlag({0, 1});
    expect(flag.accepted && flag.changed, "hidden cells can be flagged before reveal");
    expect(game.cellAt({0, 1}).flagged, "pre-reveal flag is visible");

    ActionResult revealFlagged = game.reveal({0, 1});
    expect(!revealFlagged.accepted, "flagged cells cannot be revealed");
    expect(game.status() == GameStatus::Ready, "rejected reveal does not start game");
}

void testFirstRevealIsSafeAndPreservesFlags() {
    MinesweeperGame game = makeGame();
    game.toggleFlag({0, 1});

    ActionResult reveal = game.reveal({0, 0});
    expect(reveal.accepted, "first safe reveal is accepted");
    expect(game.status() == GameStatus::InProgress, "first reveal starts game");
    expect(game.cellAt({0, 0}).revealed, "first cell is revealed");
    expect(!game.cellAt({0, 0}).mine, "first cell is not a mine");
    expect(game.cellAt({0, 1}).flagged, "pre-reveal flag is preserved");

    int mineCells = 0;
    for (int row = 0; row < game.rows(); ++row) {
        for (int column = 0; column < game.columns(); ++column) {
            if (game.cellAt({row, column}).mine) {
                ++mineCells;
            }
        }
    }
    expect(mineCells == game.mineCount(), "initialized board has ten mines");
}

void testAdjacentCountsAndFloodReveal() {
    MinesweeperGame game = makeGame();
    game.reveal({0, 0});

    CellView first = game.cellAt({0, 0});
    expect(first.adjacentMines == 2, "corner count is correct");
    expect(game.cellAt({0, 2}).adjacentMines == 2, "edge count is correct");

    MinesweeperGame emptyArea = MinesweeperGame::withMineLayoutForTesting({
        {8, 8}, {8, 7}, {7, 8}, {6, 6}, {5, 5},
        {4, 4}, {3, 3}, {2, 2}, {4, 8}, {8, 0},
    });
    emptyArea.reveal({0, 0});
    expect(emptyArea.cellAt({0, 0}).revealed, "zero-area origin is revealed");
    expect(emptyArea.cellAt({0, 1}).revealed, "zero-area expansion reveals neighbors");
    expect(emptyArea.cellAt({1, 0}).revealed, "zero-area expansion reaches adjacent rows");
}

void testFlagsAndInvalidActions() {
    MinesweeperGame game = makeGame();

    expect(!game.reveal({-1, 0}).accepted, "negative row is rejected");
    expect(!game.toggleFlag({9, 0}).accepted, "out-of-range row is rejected");

    game.reveal({0, 0});
    expect(!game.toggleFlag({0, 0}).accepted, "revealed cells cannot be flagged");
    game.toggleFlag({0, 1});
    expect(!game.reveal({0, 1}).accepted, "flagged mines cannot be revealed");
    game.toggleFlag({0, 1});
    expect(game.reveal({0, 1}).accepted, "unflagged mines can be revealed");
    expect(game.status() == GameStatus::Lost, "unflagged mine reveal loses the game");
}

void testLossAndWin() {
    MinesweeperGame lossGame = makeGame();
    lossGame.reveal({0, 0});
    lossGame.toggleFlag({0, 1});
    lossGame.toggleFlag({0, 1});
    ActionResult loss = lossGame.reveal({0, 1});
    expectStatus(loss, GameStatus::Lost, "revealing a mine loses the game");
    expect(lossGame.cellAt({0, 1}).mine, "mine remains inspectable after loss");
    expect(!lossGame.reveal({1, 0}).accepted, "actions after loss are rejected");

    MinesweeperGame winGame = makeGame();
    for (int row = 0; row < MinesweeperGame::kRows; ++row) {
        for (int column = 0; column < MinesweeperGame::kColumns; ++column) {
            if (!(row == 0 && column == 1) && !(row == 1 && column == 1) &&
                !(row == 2 && column == 2) && !(row == 3 && column == 3) &&
                !(row == 4 && column == 4) && !(row == 5 && column == 5) &&
                !(row == 6 && column == 6) && !(row == 7 && column == 7) &&
                !(row == 8 && column == 8) && !(row == 0 && column == 8)) {
                winGame.reveal({row, column});
            }
        }
    }
    expect(winGame.status() == GameStatus::Won, "revealing every safe cell wins");
    expect(!winGame.toggleFlag({0, 1}).accepted, "actions after win are rejected");
}

}  // namespace

int main() {
    testInitialStateAndPreRevealFlags();
    testFirstRevealIsSafeAndPreservesFlags();
    testAdjacentCountsAndFloodReveal();
    testFlagsAndInvalidActions();
    testLossAndWin();
    std::cout << "All Minesweeper model tests passed.\n";
    return EXIT_SUCCESS;
}
