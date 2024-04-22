#include <algorithm>
#include <array>
#include <iostream>
#include <set>

using Possibilities = std::set<int>;

struct Cell {
  int value = 0;

  friend std::ostream &operator<<(std::ostream &stream, Cell &cell) {
    if (cell.value) {
      stream << ' ' << cell.value << ' ';
    } else {
      stream << "   ";
    }

    return stream;
  }
};

struct Row {
  std::array<Cell, 9> cells = {};

  Cell &operator[](size_t cell) { return cells[cell]; }

  friend std::ostream &operator<<(std::ostream &stream, Row &row) {
    stream << "|";

    for (auto &cell : row.cells) {
      stream << cell << '|';
    }

    stream << '\n';

    for (int _ = 0; _ < 37; ++_) {
      stream << '-';
    }

    stream << '\n';

    return stream;
  }
};

struct Board {
  std::array<Row, 9> board = {};
  std::array<std::array<Cell *, 9>, 9> rows = {};
  std::array<std::array<Cell *, 9>, 9> cols = {};
  std::array<std::array<Possibilities, 9>, 9> possibilitiesArray = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesRows = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesCols = {};

  explicit Board(std::array<std::array<int, 9>, 9> const &values) {
    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        board[row][col].value = values[row][col];
        rows[row][col] = &(board[row][col]);
        cols[col][row] = &(board[row][col]);
      }
    }

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        possibilitiesArray[row][col] = possibilities(row, col);
        possibilitiesRows[row][col] = &(possibilitiesArray[row][col]);
        possibilitiesCols[col][row] = &(possibilitiesArray[row][col]);
      }
    }
  }

  Board(Board &board) {
    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        this->board[row][col].value = board[row][col].value;
        rows[row][col] = &(this->board[row][col]);
        cols[col][row] = &(this->board[row][col]);
      }
    }

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        possibilitiesArray[row][col] = possibilities(row, col);
        possibilitiesRows[row][col] = &(possibilitiesArray[row][col]);
        possibilitiesCols[col][row] = &(possibilitiesArray[row][col]);
      }
    }
  };

  Possibilities possibilities(int row, int col) {
    // Checks if current cell is occupied

    if (rows[row][col]->value != 0) {
      return {};
    }

    Possibilities possibilities = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    // Checks for numbers in current row and column

    for (int i = 0; i < 9; ++i) {
      int rowVal = rows[row][i]->value;

      if (rowVal != 0) {
        possibilities.erase(rowVal);
      }

      int colVal = cols[col][i]->value;

      if (colVal != 0) {
        possibilities.erase(colVal);
      }
    }

    // Checks for numbers in the current grid

    int startRow = row - row % 3;
    int startCol = col - col % 3;

    for (int i = startRow; i < startRow + 3; ++i) {
      for (int j = startCol; j < startCol + 3; ++j) {
        int val = board[i][j].value;

        if (val != 0) {
          possibilities.erase(val);
        }
      }
    }

    return possibilities;
  };

  bool solve() {
    int count = 0;

    // Gets total number of occupied cells

    for (auto &row : board) {
      count += std::ranges::count_if(row.cells, [](Cell const cell) { return cell.value == 0; });
    }

    int countCopy = count;

    while (count > 0) {
      for (int value = 1; value <= 9; ++value) {
        for (int row = 0; row < 9; ++row) {
          if (std::ranges::count_if(possibilitiesRows[row], [value](Possibilities *possibilities) {
                return possibilities->contains(value);
              }) != 8)
            continue;

          int valueCol =
              std::ranges::find_if(possibilitiesRows[row],
                                   [value](Possibilities *possibilities) { return !possibilities->contains(value); }) -
              possibilitiesRows[row].begin();

          rows[row][valueCol]->value = value;

          possibilitiesRows[row][valueCol]->clear();

          for (int i = 0; i < 9; ++i) {
            possibilitiesArray[i][valueCol].erase(value);
            possibilitiesArray[row][i].erase(value);
          }

          int startRow = row - row % 3;
          int startCol = valueCol - valueCol % 3;

          for (int i = startRow; i < startRow + 3; ++i) {
            for (int j = startCol; j < startCol + 3; ++j) {
              possibilitiesArray[i][j].erase(value);
            }
          }

          count--;
        }

        for (int col = 0; col < 9; ++col) {
          if (std::ranges::count_if(possibilitiesCols[col], [value](Possibilities *possibilities) {
                return possibilities->contains(value);
              }) != 8)
            continue;

          int valueRow =
              std::ranges::find_if(possibilitiesCols[col],
                                   [value](Possibilities *possibilities) { return !possibilities->contains(value); }) -
              possibilitiesCols[col].begin();

          cols[col][valueRow]->value = value;

          possibilitiesArray[valueRow][col].clear();

          for (int i = 0; i < 9; ++i) {
            possibilitiesArray[valueRow][i].erase(value);
            possibilitiesArray[i][col].erase(value);
          }

          int startRow = valueRow - valueRow % 3;
          int startCol = col - col % 3;

          for (int i = startRow; i < startRow + 3; ++i) {
            for (int j = startCol; j < startCol + 3; ++j) {
              possibilitiesArray[i][j].erase(value);
            }
          }

          count--;
        }

        for (int row = 0; row < 9; row += 3) {
          for (int col = 0; col < 9; col += 3) {
            int zeroCount = 0;
            int valueRow = 0;
            int valueCol = 0;

            for (int i = 0; i < 3; ++i) {
              for (int j = 0; j < 3; ++j) {
                if (!possibilitiesArray[row + i][col + j].contains(value)) {
                  zeroCount += 1;
                } else {
                  valueRow = row + i;
                  valueCol = col + j;
                }
              }
            }

            if (zeroCount != 8)
              continue;

            board[valueRow][valueCol].value = value;

            possibilitiesArray[valueRow][valueCol].clear();

            for (int i = 0; i < 9; ++i) {
              possibilitiesRows[valueRow][i]->erase(value);
              possibilitiesCols[valueCol][i]->erase(value);
            }

            int startRow = valueRow - valueRow % 3;
            int startCol = valueCol - valueCol % 3;

            for (int i = startRow; i < startRow + 3; ++i) {
              for (int j = startCol; j < startCol + 3; ++j) {
                possibilitiesArray[i][j].erase(value);
              }
            }

            count--;
          }
        }

        // Narrows down cells where this value should occur at

        for (int i = 0; i < 9; ++i) {
          int iStart = i - i % 3;

          for (int jStart = 0; jStart < 9; jStart += 3) {
            int rowCount =
                std::count_if(possibilitiesRows[i].begin() + jStart, possibilitiesRows[i].begin() + jStart + 3,
                              [value](Possibilities *possibilities) { return possibilities->contains(value); });
            int colCount =
                std::count_if(possibilitiesCols[i].begin() + jStart, possibilitiesCols[i].begin() + jStart + 3,
                              [value](Possibilities *possibilities) { return possibilities->contains(value); });

            int gridRowCount = 0;
            int gridColCount = 0;

            for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
              for (int colOffset = 0; colOffset < 3; ++colOffset) {
                if (possibilitiesArray[iStart + rowOffset][jStart + colOffset].contains(value)) {
                  gridRowCount += 1;
                }

                if (possibilitiesArray[jStart + rowOffset][iStart + colOffset].contains(value)) {
                  gridColCount += 1;
                }
              }
            }

            if (gridRowCount && gridRowCount == rowCount) {
              for (int col = 0; col < 9; ++col) {
                if (col == jStart || col == jStart + 1 || col == jStart + 2)
                  continue;

                possibilitiesArray[i][col].erase(value);
              }
            }

            if (gridColCount && gridColCount == colCount) {
              for (int row = 0; row < 9; ++row) {
                if (row == jStart || row == jStart + 1 || row == jStart + 2)
                  continue;

                possibilitiesArray[row][i].erase(value);
              }
            }

            int totalRowCount = std::ranges::count_if(
                possibilitiesRows[i], [value](Possibilities *possibilities) { return possibilities->contains(value); });
            int totalColumnCount = std::ranges::count_if(
                possibilitiesCols[i], [value](Possibilities *possibilities) { return possibilities->contains(value); });

            if (totalRowCount && totalRowCount == rowCount) {
              for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
                if (iStart + rowOffset == i)
                  continue;

                for (int colOffset = 0; colOffset < 3; ++colOffset) {
                  possibilitiesArray[iStart + rowOffset][jStart + colOffset].erase(value);
                }
              }
            }

            if (totalColumnCount && totalColumnCount == colCount) {
              for (int colOffset = 0; colOffset < 3; ++colOffset) {
                if (iStart + colOffset == i)
                  continue;

                for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
                  possibilitiesArray[jStart + rowOffset][iStart + colOffset].erase(value);
                }
              }
            }
          }
        }
      }

      // Checks if theres any cell where only one number is possible

      for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
          if (std::ranges::count_if(possibilitiesArray[row][col], [](int const x) { return x == 0; }) != 8)
            continue;

          int const val = *possibilitiesArray[row][col].begin();

          board[row][col].value = val + 1;

          possibilitiesArray[row][col].clear();

          for (int i = 0; i < 9; ++i) {
            possibilitiesRows[row][i]->erase(val);
            possibilitiesCols[col][i]->erase(val);
          }

          int startRow = row - row % 3;
          int startCol = col - col % 3;

          for (int i = startRow; i < startRow + 3; ++i) {
            for (int j = startCol; j < startCol + 3; ++j) {
              possibilitiesArray[i][j].erase(val);
            }
          }

          count--;
        }
      }

      // Checks if all usefull moves have been used

      if (count == countCopy) {
        for (int row = 0; row < 9; ++row) {
          for (int col = 0; col < 9; ++col) {

            for (int value : possibilitiesArray[row][col]) {
              if (value == 0) {
                continue;
              }

              auto copy = Board(*this);

              copy[row][col].value = value;

              copy.possibilitiesArray[row][col].clear();

              for (int i = 0; i < 9; ++i) {
                copy.possibilitiesArray[row][i].erase(value);
                copy.possibilitiesArray[i][col].erase(value);
              }

              int startRow = row - row % 3;
              int startCol = col - col % 3;

              for (int i = startRow; i < startRow + 3; ++i) {
                for (int j = startCol; j < startCol + 3; ++j) {
                  copy.possibilitiesArray[i][j].erase(value);
                }
              }

              if (!copy.solve())
                continue;

              for (int x = 0; x < 9; ++x) {
                for (int y = 0; y < 9; ++y) {
                  if (copy[x][y].value != board[x][y].value) {
                    board[x][y].value = copy[x][y].value;
                  }
                }
              }

              return true;
            }
          }
        }

        return false;
      }

      countCopy = count;
    }

    return true;
  }

  Row &operator[](size_t row) { return board[row]; }

  friend std::ostream &operator<<(std::ostream &stream, Board board) {
    stream << '\n';

    for (int _ = 0; _ < 37; ++_) {
      stream << '-';
    }

    stream << '\n';

    for (auto &row : board.board) {
      stream << row;
    }

    stream << '\n';

    return stream;
  }
};

int main() {
  std::array<std::array<int, 9>, 9> values = {};

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
