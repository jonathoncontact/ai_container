#include "minesweeper.h"

#include <iostream>
#include <sstream>
#include <string>

namespace {

using minesweeper::CellView;
using minesweeper::GameStatus;
using minesweeper::MinesweeperGame;
using minesweeper::Position;

void printHelp() {
    std::cout << "Commands:\n"
              << "  r <row> <column>  reveal a cell\n"
              << "  f <row> <column>  toggle a flag\n"
              << "  h                 show this help\n"
              << "  q                 quit\n"
              << "Rows and columns use one-based coordinates from 1 to 9.\n";
}

char displayCell(const CellView& cell, GameStatus status) {
    if (status == GameStatus::Lost && cell.mine) {
        return '*';
    }
    if (cell.flagged) {
        return 'F';
    }
    if (!cell.revealed) {
        return '#';
    }
    if (cell.mine) {
        return '*';
    }
    if (cell.adjacentMines == 0) {
        return '.';
    }
    return static_cast<char>('0' + cell.adjacentMines);
}

void printBoard(const MinesweeperGame& game) {
    std::cout << "\n   1 2 3 4 5 6 7 8 9\n";
    for (int row = 0; row < game.rows(); ++row) {
        std::cout << row + 1 << "  ";
        for (int column = 0; column < game.columns(); ++column) {
            std::cout << displayCell(game.cellAt({row, column}), game.status())
                      << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

bool readPosition(std::istringstream& input, Position& position) {
    int row = 0;
    int column = 0;
    if (!(input >> row >> column) || row < 1 || row > 9 || column < 1 ||
        column > 9) {
        return false;
    }
    position = {row - 1, column - 1};
    return true;
}

bool hasExtraToken(std::istringstream& input) {
    std::string extra;
    return static_cast<bool>(input >> extra);
}

void printActionResult(const minesweeper::ActionResult& result) {
    std::cout << result.message << '\n';
    if (result.status == GameStatus::Won) {
        std::cout << "You win!\n";
    } else if (result.status == GameStatus::Lost) {
        std::cout << "Game over.\n";
    }
}

}  // namespace

int main() {
    MinesweeperGame game;
    std::cout << "Minesweeper\n";
    printHelp();
    printBoard(game);

    std::string line;
    while (game.status() != GameStatus::Won &&
           game.status() != GameStatus::Lost && std::getline(std::cin, line)) {
        std::istringstream input(line);
        std::string command;
        input >> command;

        if (command.empty()) {
            std::cout << "Enter a command.\n";
            continue;
        }
        if (command == "q") {
            if (hasExtraToken(input)) {
                std::cout << "Error: extra command tokens are not allowed.\n";
                continue;
            }
            std::cout << "Goodbye.\n";
            return 0;
        }
        if (command == "h") {
            if (hasExtraToken(input)) {
                std::cout << "Error: extra command tokens are not allowed.\n";
            } else {
                printHelp();
            }
            continue;
        }
        if (command != "r" && command != "f") {
            std::cout << "Error: unknown command.\n";
            continue;
        }

        Position position = {0, 0};
        if (!readPosition(input, position) || hasExtraToken(input)) {
            std::cout << "Error: use " << command << " <row> <column> with"
                      << " coordinates from 1 to 9.\n";
            continue;
        }

        const minesweeper::ActionResult result =
            command == "r" ? game.reveal(position) : game.toggleFlag(position);
        printActionResult(result);
        printBoard(game);
    }

    if (game.status() == GameStatus::Won || game.status() == GameStatus::Lost) {
        printBoard(game);
    }
    return 0;
}
