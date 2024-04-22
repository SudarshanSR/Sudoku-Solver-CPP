#include <algorithm>
#include <array>
#include <iostream>
#include <set>

using Number = unsigned short;
using Possibilities = std::set<Number>;

struct Board {
  int count = 0;
  std::array<std::array<Number, 9>, 9> board = {};
  std::array<std::array<Possibilities, 9>, 9> possibilitiesBoard = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesRows = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesCols = {};

  explicit Board(std::array<std::array<Number, 9>, 9> const &values) {
    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        this->board[row][col] = values[row][col];

        if (!this->board[row][col])
          ++count;
      }
    }

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        this->possibilitiesBoard[row][col] = this->possibilities(row, col);
        this->possibilitiesRows[row][col] = &(this->possibilitiesBoard[row][col]);
        this->possibilitiesCols[col][row] = &(this->possibilitiesBoard[row][col]);
      }
    }
  }

  Board(Board &board) {
    this->count = board.count;

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        this->board[row][col] = board.board[row][col];
      }
    }

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        this->possibilitiesBoard[row][col] = this->possibilities(row, col);
        this->possibilitiesRows[row][col] = &(this->possibilitiesBoard[row][col]);
        this->possibilitiesCols[col][row] = &(this->possibilitiesBoard[row][col]);
      }
    }
  };

  Possibilities possibilities(int row, int col) {
    // Checks if current cell is occupied

    if (this->board[row][col] != 0) {
      return {};
    }

    Possibilities possibilities = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    // Checks for numbers in current row and column

    for (int i = 0; i < 9; ++i) {
      Number rowVal = this->board[row][i];

      if (rowVal != 0) {
        possibilities.erase(rowVal);
      }

      Number colVal = this->board[i][col];

      if (colVal != 0) {
        possibilities.erase(colVal);
      }
    }

    // Checks for numbers in the current grid

    int startRow = row - row % 3;
    int startCol = col - col % 3;

    for (int i = startRow; i < startRow + 3; ++i) {
      for (int j = startCol; j < startCol + 3; ++j) {
        Number val = this->board[i][j];

        if (val != 0) {
          possibilities.erase(val);
        }
      }
    }

    return possibilities;
  };

  void setCell(int const row, int const col, Number const value) {
    this->board[row][col] = value;

    this->possibilitiesBoard[row][col].clear();

    for (Possibilities *p : this->possibilitiesRows[row])
      p->erase(value);

    for (Possibilities *p : this->possibilitiesCols[col])
      p->erase(value);

    int startRow = row - row % 3;
    int startCol = col - col % 3;

    for (int i = startRow; i < startRow + 3; ++i) {
      for (int j = startCol; j < startCol + 3; ++j) {
        this->possibilitiesBoard[i][j].erase(value);
      }
    }

    count--;
  }

  void checkRows(Number const value, int const row, int const rowStart) {
    int totalCount =
        std::ranges::count_if(this->possibilitiesRows[row], [value](Possibilities *p) { return p->contains(value); });

    for (int colStart = 0; colStart < 9; colStart += 3) {
      int gridCount = 0;

      for (int offset = 0; offset < 3; ++offset) {
        gridCount += std::count_if(this->possibilitiesBoard[rowStart + offset].begin() + colStart,
                                   this->possibilitiesBoard[rowStart + offset].begin() + colStart + 3,
                                   [value](Possibilities p) { return p.contains(value); });
      }

      int rowCount = std::count_if(this->possibilitiesRows[row].begin() + colStart,
                                   this->possibilitiesRows[row].begin() + colStart + 3,
                                   [value](Possibilities *p) { return p->contains(value); });

      if (gridCount && gridCount == rowCount) {
        for (int col = 0; col < colStart; ++col) {
          if (this->possibilitiesCols[col][row]->erase(value))
            --totalCount;
        }

        for (int col = colStart + 3; col < 9; ++col) {
          if (this->possibilitiesCols[col][row]->erase(value))
            --totalCount;
        }
      }

      if (totalCount && totalCount == rowCount) {
        for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
          if (rowStart + rowOffset == row)
            continue;

          for (int colOffset = 0; colOffset < 3; ++colOffset) {
            this->possibilitiesRows[rowStart + rowOffset][colStart + colOffset]->erase(value);
          }
        }
      }
    }
  }

  void checkCols(Number const value, int const col, int const colStart) {
    int totalCount =
        std::ranges::count_if(this->possibilitiesCols[col], [value](Possibilities *p) { return p->contains(value); });

    for (int rowStart = 0; rowStart < 9; rowStart += 3) {
      int gridCount = 0;

      for (int offset = 0; offset < 3; ++offset) {
        gridCount += std::count_if(this->possibilitiesBoard[rowStart + offset].begin() + colStart,
                                   this->possibilitiesBoard[rowStart + offset].begin() + colStart + 3,
                                   [value](Possibilities p) { return p.contains(value); });
      }

      int colCount = std::count_if(this->possibilitiesCols[col].begin() + rowStart,
                                   this->possibilitiesCols[col].begin() + rowStart + 3,
                                   [value](Possibilities *p) { return p->contains(value); });

      if (gridCount && gridCount == colCount) {
        for (int row = 0; row < rowStart; ++row) {
          if (this->possibilitiesRows[row][col]->erase(value))
            --totalCount;
        }

        for (int row = rowStart + 3; row < 9; ++row) {
          if (this->possibilitiesRows[row][col]->erase(value))
            --totalCount;
        }
      }

      if (totalCount && totalCount == colCount) {
        for (int colOffset = 0; colOffset < 3; ++colOffset) {
          if (colStart + colOffset == col)
            continue;

          for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
            this->possibilitiesCols[colStart + colOffset][rowStart + rowOffset]->erase(value);
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
                                    [value](Possibilities *p) { return !p->contains(value); }) != 8)
            continue;

          int col = std::ranges::find_if(this->possibilitiesRows[row],
                                         [value](Possibilities *p) { return p->contains(value); }) -
                    this->possibilitiesRows[row].begin();

          this->setCell(row, col, value);
        }

        for (int col = 0; col < 9; ++col) {
          if (std::ranges::count_if(this->possibilitiesCols[col],
                                    [value](Possibilities *p) { return !p->contains(value); }) != 8)
            continue;

          int row = std::ranges::find_if(this->possibilitiesCols[col],
                                         [value](Possibilities *p) { return p->contains(value); }) -
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
                if (!this->possibilitiesBoard[row + i][col + j].contains(value)) {
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
          if (this->possibilitiesBoard[row][col].size() != 1)
            continue;

          Number const val = *this->possibilitiesBoard[row][col].begin();

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
