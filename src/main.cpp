#include <iostream>

#include "solver.hpp"

int main() {
    std::array<std::array<Number, 9>, 9> values = {};

    for (Row row = 0; row < 9; ++row) {
        for (Col col = 0; col < 9; ++col) {
            std::cout << "Enter number at row " << row << " and col " << col
                      << ": ";
            std::cin >> values[row][col];
            std::cout << "\n";
        }
    }

    Board board(values);
    std::cout << "Board to solve:" << board << "Solving...\n";

    board.solve();

    std::cout << "Solved!" << board;

    return 0;
}
