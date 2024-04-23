#include <algorithm>
#include <array>
#include <iostream>

using Number = unsigned short;
using Possibilities = std::array<bool, 9>;

struct Board {
  int count = 81;
  std::array<std::array<Number, 9>, 9> board = {};
  std::array<std::array<Possibilities, 9>, 9> possibilitiesBoard = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesRows = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesCols = {};

  explicit Board(std::array<std::array<Number, 9>, 9> const &values) {
    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        this->possibilitiesBoard[row][col] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
        this->possibilitiesRows[row][col] = &(this->possibilitiesBoard[row][col]);
        this->possibilitiesCols[col][row] = &(this->possibilitiesBoard[row][col]);
      }
    }

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        if (!values[row][col]) {
          this->board[row][col] = 0;

          continue;
        }

        this->setCell(row, col, values[row][col]);
      }
    }
  }

  Board(Board &board) : Board(board.board) {}

  void setCell(int const row, int const col, Number const value) {
    this->board[row][col] = value;

    this->possibilitiesBoard[row][col].fill(0);

    for (Possibilities *p : this->possibilitiesRows[row])
      (*p)[value - 1] = 0;

    for (Possibilities *p : this->possibilitiesCols[col])
      (*p)[value - 1] = 0;

    int startRow = row - row % 3;
    int startCol = col - col % 3;

    for (int i = startRow; i < startRow + 3; ++i) {
      for (int j = startCol; j < startCol + 3; ++j) {
        this->possibilitiesBoard[i][j][value - 1] = 0;
      }
    }

    count--;
  }

  void checkRows(Number const value, int const row, int const rowStart) {
    std::array<Possibilities *, 9> possibilitiesArray = this->possibilitiesRows[row];
    for (int colStart = 0; colStart < 9; colStart += 3) {
      int gridCount = 0;

      for (int offset = 0; offset < 3; ++offset) {
        gridCount += std::count_if(this->possibilitiesBoard[rowStart + offset].begin() + colStart,
                                   this->possibilitiesBoard[rowStart + offset].begin() + colStart + 3,
                                   [value](Possibilities p) { return p[value - 1]; });
      }

      int rowCount = std::count_if(possibilitiesArray.begin() + colStart, possibilitiesArray.begin() + colStart + 3,
                                   [value](Possibilities *p) { return (*p)[value - 1]; });

      if (gridCount && gridCount == rowCount) {
        for (int col = 0; col < colStart; ++col) {
          (*possibilitiesArray[col])[value - 1] = 0;
        }

        for (int col = colStart + 3; col < 9; ++col) {
          (*possibilitiesArray[col])[value - 1] = 0;
        }
      }

      int totalCount = std::ranges::count_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; });

      if (totalCount && totalCount == rowCount) {
        for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
          if (rowStart + rowOffset == row)
            continue;

          for (int colOffset = 0; colOffset < 3; ++colOffset) {
            this->possibilitiesBoard[rowStart + rowOffset][colStart + colOffset][value - 1] = 0;
          }
        }
      }
    }
  }

  void checkCols(Number const value, int const col, int const colStart) {
    std::array<Possibilities *, 9> &possibilitiesArray = this->possibilitiesCols[col];

    for (int rowStart = 0; rowStart < 9; rowStart += 3) {
      int gridCount = 0;

      for (int offset = 0; offset < 3; ++offset) {
        gridCount += std::count_if(this->possibilitiesBoard[rowStart + offset].begin() + colStart,
                                   this->possibilitiesBoard[rowStart + offset].begin() + colStart + 3,
                                   [value](Possibilities p) { return p[value - 1]; });
      }

      int colCount = std::count_if(possibilitiesArray.begin() + rowStart, possibilitiesArray.begin() + rowStart + 3,
                                   [value](Possibilities *p) { return (*p)[value - 1]; });

      if (gridCount && gridCount == colCount) {
        for (int row = 0; row < rowStart; ++row) {
          (*possibilitiesArray[row])[value - 1] = 0;
        }

        for (int row = rowStart + 3; row < 9; ++row) {
          (*possibilitiesArray[row])[value - 1] = 0;
        }
      }

      int totalCount = std::ranges::count_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; });

      if (totalCount && totalCount == colCount) {
        for (int colOffset = 0; colOffset < 3; ++colOffset) {
          if (colStart + colOffset == col)
            continue;

          for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
            this->possibilitiesBoard[rowStart + rowOffset][colStart + colOffset][value - 1] = 0;
          }
        }
      }
    }
  }

  bool solve() {
    int countCopy = this->count;

    while (this->count > 0) {
      for (Number value = 1; value <= 9; ++value) {
        for (int row = 0; row < 9; ++row) {
          if (std::ranges::count_if(this->possibilitiesRows[row],
                                    [value](Possibilities *p) { return !((*p)[value - 1]); }) != 8)
            continue;

          int col = std::ranges::find_if(this->possibilitiesRows[row],
                                         [value](Possibilities *p) { return (*p)[value - 1]; }) -
                    this->possibilitiesRows[row].begin();

          this->setCell(row, col, value);
        }

        for (int col = 0; col < 9; ++col) {
          if (std::ranges::count_if(this->possibilitiesCols[col],
                                    [value](Possibilities *p) { return !((*p)[value - 1]); }) != 8)
            continue;

          int row = std::ranges::find_if(this->possibilitiesCols[col],
                                         [value](Possibilities *p) { return (*p)[value - 1]; }) -
                    this->possibilitiesCols[col].begin();

          this->setCell(row, col, value);
        }

        for (int row = 0; row < 9; row += 3) {
          for (int col = 0; col < 9; col += 3) {
            int zeroCount = 0;
            int valueRow = 0;
            int valueCol = 0;

            for (int i = 0; i < 3; ++i) {
              for (int j = 0; j < 3; ++j) {
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

        // Narrows down cells where this value should occur at

        for (int i = 0; i < 9; ++i) {
          int iStart = i - i % 3;

          checkRows(value, i, iStart);

          checkCols(value, i, iStart);
        }
      }

      // Checks if theres any cell where only one number is possible

      for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
          if (std::ranges::count(this->possibilitiesBoard[row][col], 0) != 8)
            continue;

          Number const val =
              std::ranges::find(this->possibilitiesBoard[row][col], 1) - this->possibilitiesBoard[row][col].begin() + 1;

          this->setCell(row, col, val);
        }
      }

      // Checks if all usefull moves have been used

      if (this->count == countCopy) {
        for (int row = 0; row < 9; ++row) {
          for (int col = 0; col < 9; ++col) {
            for (Number const value : this->possibilitiesBoard[row][col]) {
              Board copy(*this);

              copy.setCell(row, col, value);

              if (!copy.solve())
                continue;

              this->board = copy.board;

              return true;
            }
          }
        }

        return false;
      }

      countCopy = this->count;
    }

    return true;
  }

  friend std::ostream &operator<<(std::ostream &stream, Board board) {
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

    stream << '\n';

    return stream;
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
