#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>

using Number = unsigned short;
using Possibilities = std::array<bool, 9>;
using Cell = std::pair<unsigned short, unsigned short>;

class Board {
public:
  explicit Board(std::array<std::array<Number, 9>, 9> const &values) {
    for (unsigned short row = 0; row < 9; ++row) {
      for (unsigned short col = 0; col < 9; ++col) {
        this->possibilitiesBoard[row][col] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
        this->possibilitiesRows[row][col] = &(this->possibilitiesBoard[row][col]);
        this->possibilitiesCols[col][row] = &(this->possibilitiesBoard[row][col]);

        this->remainingCells.insert({row, col});
      }
    }

    for (unsigned short row = 0; row < 9; ++row) {
      for (unsigned short col = 0; col < 9; ++col) {
        if (!values[row][col]) {
          continue;
        }

        this->setCell(row, col, values[row][col]);
      }
    }
  }

  bool solve() {
    short countCopy = this->remainingCells.size();

    while (!this->remainingCells.empty()) {
      for (auto const &[value, _] : this->remainingValues) {
        for (unsigned short row = 0; row < 9; row += 3) {
          for (unsigned short col = 0; col < 9; col += 3) {
            unsigned short zeroCount = 0;
            unsigned short valueRow = 0;
            unsigned short valueCol = 0;

            for (unsigned short i = 0; i < 3; ++i) {
              for (unsigned short j = 0; j < 3; ++j) {
                if (!this->possibilitiesBoard[row + i][col + j][value - 1]) {
                  zeroCount += 1;
                } else {
                  valueRow = row + i;
                  valueCol = col + j;
                }
              }
            }

            if (zeroCount != 8)
              continue;

            this->setCell(valueRow, valueCol, value);
          }
        }

        for (unsigned short i = 0; i < 9; ++i) {
          // Checks if there are any unique cells in a row and col where only this number can occur

          this->checkRow(value, i);

          this->checkCol(value, i);

          // Narrows down cells where this value should occur at

          unsigned short iStart = i - i % 3;

          narrowRows(value, i, iStart);

          narrowCols(value, i, iStart);
        }
      }

      // Checks if theres any cell where only one number is possible

      for (auto const &[row, col] : this->remainingCells) {
        if (std::ranges::count(this->possibilitiesBoard[row][col], 0) != 8)
          continue;

        this->setCell(
            row, col,
            std::ranges::find(this->possibilitiesBoard[row][col], 1) - this->possibilitiesBoard[row][col].begin() + 1
        );
      }

      // Checks if all useful moves have been used

      if (size_t count = this->remainingCells.size(); count != countCopy) {
        countCopy = count;

        continue;
      }

      // Starts guessing values

      auto const &[row, col] = *this->remainingCells.begin();

      if (std::ranges::count(this->possibilitiesBoard[row][col], 0) == 9) {
        return false;
      }

      size_t const index =
          std::ranges::find(this->possibilitiesBoard[row][col], 1) - this->possibilitiesBoard[row][col].begin();

      Board copy(*this);

      copy.setCell(row, col, index + 1);

      if (copy.solve()) {
        this->board = copy.board;

        return true;
      }

      this->possibilitiesBoard[row][col][index] = 0;

      if (std::ranges::count(this->possibilitiesBoard[row][col], 0) == 9) {
        return false;
      }
    }

    return true;
  }

  friend std::ostream &operator<<(std::ostream &stream, Board const &board) {
    stream << '\n';

    for (int _ = 0; _ < 37; ++_) {
      stream << '-';
    }

    stream << '\n';

    for (std::array<Number, 9> const &row : board.board) {
      stream << "|";

      for (Number const &cell : row) {
        if (cell) {
          stream << ' ' << cell << ' ';
        } else {
          stream << "   ";
        }

        stream << '|';
      }

      stream << '\n';

      for (int _ = 0; _ < 37; ++_) {
        stream << '-';
      }

      stream << '\n';
    }

    return stream;
  }

private:
  Board(Board const &board) : Board(board.board) {}

  std::array<std::array<Number, 9>, 9> board = {};
  std::array<std::array<Possibilities, 9>, 9> possibilitiesBoard = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesRows = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesCols = {};
  std::map<Number, unsigned short> remainingValues = {
      {1, 9}, {2, 9}, {3, 9}, {4, 9}, {5, 9}, {6, 9}, {7, 9}, {8, 9}, {9, 9},
  };
  std::set<Cell> remainingCells;

  void setCell(unsigned short const row, unsigned short const col, Number const value) {
    this->board[row][col] = value;

    this->possibilitiesBoard[row][col].fill(0);

    for (Possibilities *p : this->possibilitiesRows[row])
      (*p)[value - 1] = 0;

    for (Possibilities *p : this->possibilitiesCols[col])
      (*p)[value - 1] = 0;

    unsigned short startRow = row - row % 3;
    unsigned short startCol = col - col % 3;

    for (int i = startRow; i < startRow + 3; ++i) {
      for (int j = startCol; j < startCol + 3; ++j) {
        this->possibilitiesBoard[i][j][value - 1] = 0;
      }
    }

    if (!--this->remainingValues[value]) {
      this->remainingValues.erase(value);
    }

    remainingCells.erase({row, col});
  }

  void narrowRows(Number const value, unsigned short const row, unsigned short const rowStart) {
    std::array<Possibilities *, 9> possibilitiesArray = this->possibilitiesRows[row];

    for (unsigned short colStart = 0; colStart < 9; colStart += 3) {
      unsigned short gridCount = 0;

      for (unsigned short offset = 0; offset < 3; ++offset) {
        gridCount += std::count_if(
            this->possibilitiesBoard[rowStart + offset].begin() + colStart,
            this->possibilitiesBoard[rowStart + offset].begin() + colStart + 3,
            [value](Possibilities p) { return p[value - 1]; }
        );
      }

      unsigned short rowCount = std::count_if(
          possibilitiesArray.begin() + colStart, possibilitiesArray.begin() + colStart + 3,
          [value](Possibilities *p) { return (*p)[value - 1]; }
      );

      if (gridCount && gridCount == rowCount) {
        for (unsigned short col = 0; col < colStart; ++col) {
          (*possibilitiesArray[col])[value - 1] = 0;
        }

        for (unsigned short col = colStart + 3; col < 9; ++col) {
          (*possibilitiesArray[col])[value - 1] = 0;
        }
      }

      unsigned short totalCount =
          std::ranges::count_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; });

      if (totalCount && totalCount == rowCount) {
        for (unsigned short rowOffset = 0; rowOffset < 3; ++rowOffset) {
          if (rowStart + rowOffset == row)
            continue;

          for (unsigned short colOffset = 0; colOffset < 3; ++colOffset) {
            this->possibilitiesBoard[rowStart + rowOffset][colStart + colOffset][value - 1] = 0;
          }
        }
      }
    }
  }

  void narrowCols(Number const value, unsigned short const col, unsigned short const colStart) {
    std::array<Possibilities *, 9> &possibilitiesArray = this->possibilitiesCols[col];

    for (unsigned short rowStart = 0; rowStart < 9; rowStart += 3) {
      unsigned short gridCount = 0;

      for (unsigned short offset = 0; offset < 3; ++offset) {
        gridCount += std::count_if(
            this->possibilitiesBoard[rowStart + offset].begin() + colStart,
            this->possibilitiesBoard[rowStart + offset].begin() + colStart + 3,
            [value](Possibilities p) { return p[value - 1]; }
        );
      }

      unsigned short colCount = std::count_if(
          possibilitiesArray.begin() + rowStart, possibilitiesArray.begin() + rowStart + 3,
          [value](Possibilities *p) { return (*p)[value - 1]; }
      );

      if (gridCount && gridCount == colCount) {
        for (unsigned short row = 0; row < rowStart; ++row) {
          (*possibilitiesArray[row])[value - 1] = 0;
        }

        for (unsigned short row = rowStart + 3; row < 9; ++row) {
          (*possibilitiesArray[row])[value - 1] = 0;
        }
      }

      unsigned short totalCount =
          std::ranges::count_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; });

      if (totalCount && totalCount == colCount) {
        for (unsigned short colOffset = 0; colOffset < 3; ++colOffset) {
          if (colStart + colOffset == col)
            continue;

          for (unsigned short rowOffset = 0; rowOffset < 3; ++rowOffset) {
            this->possibilitiesBoard[rowStart + rowOffset][colStart + colOffset][value - 1] = 0;
          }
        }
      }
    }
  }

  void checkRow(Number const value, unsigned short const row) {
    if (std::ranges::count_if(this->possibilitiesRows[row], [value](Possibilities *p) { return (*p)[value - 1]; }) != 1)
      return;

    this->setCell(
        row,
        std::ranges::find_if(this->possibilitiesRows[row], [value](Possibilities *p) { return (*p)[value - 1]; }) -
            this->possibilitiesRows[row].begin(),
        value
    );
  }

  void checkCol(Number const value, unsigned short const col) {
    if (std::ranges::count_if(this->possibilitiesCols[col], [value](Possibilities *p) { return (*p)[value - 1]; }) != 1)
      return;

    this->setCell(
        std::ranges::find_if(this->possibilitiesCols[col], [value](Possibilities *p) { return (*p)[value - 1]; }) -
            this->possibilitiesCols[col].begin(),
        col, value
    );
  }
};

int main() {
  std::array<std::array<Number, 9>, 9> values = {};

  for (int row = 0; row < 9; ++row) {
    for (int col = 0; col < 9; ++col) {
      std::cout << "Enter number at row " << row << " and col " << col << ": ";
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
