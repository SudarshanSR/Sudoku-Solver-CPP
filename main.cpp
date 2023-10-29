#include <iostream>

class Possibilities {
 public:
  int values[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  int &operator[](size_t position) {
    return values[position];
  }
};

class Cell {
 public:
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

class Row {
 public:
  Cell cells[9] = {};

  Cell &operator[](size_t cell) {
    return cells[cell];
  }

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

class Board {
 public:
  Row rows[9] = {};
  Possibilities possibilitiesArray[9][9] = {};

  explicit Board(int (*values)[9]) {
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
        rows[i][j].value = values[i][j];
      }
    }

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        possibilitiesArray[row][col] = possibilities(row, col);
      }
    }
  }

  Board(Board &board) {
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
        rows[i][j].value = board[i][j].value;
      }
    }

    for (int row = 0; row < 9; ++row) {
      for (int col = 0; col < 9; ++col) {
        possibilitiesArray[row][col] = possibilities(row, col);
      }
    }
  };

  Possibilities possibilities(int row, int col) {
    Possibilities possibilities;

    if (rows[row][col].value != 0) {
      for (int i = 0; i < 9; ++i) {
        possibilities[i] = 0;
      }

      return possibilities;
    }

    for (int i = 0; i < 9; ++i) {
      int rowVal = rows[row][i].value;

      if (rowVal != 0) {
        possibilities[rowVal - 1] = 0;
      }

      int colVal = rows[i][col].value;

      if (colVal != 0) {
        possibilities[colVal - 1] = 0;
      }
    }

    int startRow = row - row % 3;
    int startCol = col - col % 3;

    for (int i = startRow; i < startRow + 3; ++i) {
      for (int j = startCol; j < startCol + 3; ++j) {
        int val = rows[i][j].value;

        if (val != 0) {
          possibilities[val - 1] = 0;
        }
      }
    }

    return possibilities;
  };

  bool solve() {
    int count = 0;

    for (auto &row : rows) {
      for (auto &cell : row.cells) {
        if (!cell.value) {
          count++;
        }
      }
    }

    int countCopy = count;

    while (count > 0) {
      for (int value = 1; value <= 9; ++value) {
        for (int i = 0; i < 9; ++i) {
          int zeroRowCount = 0;
          int zeroColCount = 0;
          int valueCol = 0;
          int valueRow = 0;

          for (int j = 0; j < 9; ++j) {
            if (possibilitiesArray[i][j][value - 1] != 0) {
              valueCol = j;
            } else {
              zeroRowCount += 1;
            }

            if (possibilitiesArray[j][i][value - 1] != 0) {
              valueRow = j;
            } else {
              zeroColCount += 1;
            }
          }

          if (zeroRowCount == 8) {
            rows[i][valueCol].value = value;

            for (int j = 0; j < 9; ++j) {
              possibilitiesArray[i][valueCol][j] = 0;
              possibilitiesArray[i][j][value - 1] = 0;
              possibilitiesArray[j][valueCol][value - 1] = 0;
            }

            int startRow = i - i % 3;
            int startCol = valueCol - valueCol % 3;

            for (int j = startRow; j < startRow + 3; ++j) {
              for (int k = startCol; k < startCol + 3; ++k) {
                possibilitiesArray[j][k][value - 1] = 0;
              }
            }

            count--;
          }

          if (zeroColCount == 8) {
            rows[valueRow][i].value = value;

            for (int j = 0; j < 9; ++j) {
              possibilitiesArray[valueRow][i][j] = 0;
              possibilitiesArray[valueRow][j][value - 1] = 0;
              possibilitiesArray[j][i][value - 1] = 0;
            }

            int startRow = valueRow - valueRow % 3;
            int startCol = i - i % 3;

            for (int j = startRow; j < startRow + 3; ++j) {
              for (int k = startCol; k < startCol + 3; ++k) {
                possibilitiesArray[j][k][value - 1] = 0;
              }
            }

            count--;
          }
        }

        for (int row = 0; row < 9; row += 3) {
          for (int col = 0; col < 9; col += 3) {
            int zeroCount = 0;
            int valueRow = 0;
            int valueCol = 0;

            for (int i = 0; i < 3; ++i) {
              for (int j = 0; j < 3; ++j) {
                if (possibilitiesArray[row + i][col + j][value - 1] == 0) {
                  zeroCount += 1;
                } else {
                  valueRow = row + i;
                  valueCol = col + j;
                }
              }
            }

            if (zeroCount == 8) {
              rows[valueRow][valueCol].value = value;

              for (int i = 0; i < 9; ++i) {
                possibilitiesArray[valueRow][valueCol][i] = 0;
                possibilitiesArray[valueRow][i][value - 1] = 0;
                possibilitiesArray[i][valueCol][value - 1] = 0;
              }

              int startRow = valueRow - valueRow % 3;
              int startCol = valueCol - valueCol % 3;

              for (int i = startRow; i < startRow + 3; ++i) {
                for (int j = startCol; j < startCol + 3; ++j) {
                  possibilitiesArray[i][j][value - 1] = 0;
                }
              }

              count--;
            }
          }
        }

        for (int i = 0; i < 9; ++i) {
          int iStart = i - i % 3;

          for (int jStart = 0; jStart < 9; jStart += 3) {
            int rowCount = 0;
            int totalRowCount = 0;
            int gridRowCount = 0;

            int colCount = 0;
            int totalColumnCount = 0;
            int gridColCount = 0;

            for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
              for (int colOffset = 0; colOffset < 3; ++colOffset) {
                if (possibilitiesArray[iStart + rowOffset][jStart + colOffset][value - 1] == value) {
                  gridRowCount += 1;
                }

                if (possibilitiesArray[jStart + rowOffset][iStart + colOffset][value - 1] == value) {
                  gridColCount += 1;
                }
              }
            }

            for (int offset = 0; offset < 3; ++offset) {
              if (possibilitiesArray[i][jStart + offset][value - 1] == value) {
                rowCount += 1;
              }

              if (possibilitiesArray[jStart + offset][i][value - 1] == value) {
                colCount += 1;
              }
            }

            for (int j = 0; j < 9; ++j) {
              if (possibilitiesArray[i][j][value - 1] == value) {
                totalRowCount++;
              }

              if (possibilitiesArray[j][i][value - 1] == value) {
                totalColumnCount++;
              }
            }

            if (gridRowCount && gridRowCount == rowCount) {
              for (int col = 0; col < 9; ++col) {
                if (col == jStart || col == jStart + 1 || col == jStart + 2) {
                  continue;
                }

                possibilitiesArray[i][col][value - 1] = 0;
              }
            }

            if (totalRowCount && totalRowCount == rowCount) {
              for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
                for (int colOffset = 0; colOffset < 3; ++colOffset) {
                  if (iStart + rowOffset == i) {
                    continue;
                  }

                  possibilitiesArray[iStart + rowOffset][jStart + colOffset][value - 1] = 0;
                }
              }
            }

            if (gridColCount && gridColCount == colCount) {
              for (int row = 0; row < 9; ++row) {
                if (row == jStart || row == jStart + 1 || row == jStart + 2) {
                  continue;
                }

                possibilitiesArray[row][i][value - 1] = 0;

              }
            }

            if (totalColumnCount && totalColumnCount == colCount) {
              for (int rowOffset = 0; rowOffset < 3; ++rowOffset) {
                for (int colOffset = 0; colOffset < 3; ++colOffset) {
                  if (iStart + colOffset == i) {
                    continue;
                  }

                  possibilitiesArray[jStart + rowOffset][iStart + colOffset][value - 1] = 0;
                }
              }
            }
          }
        }
      }

      for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
          int zeroCount = 0;
          int val = 0;

          for (int value : possibilitiesArray[row][col].values) {
            if (!value) {
              zeroCount += 1;
            } else {
              val = value;
            }
          }

          if (zeroCount == 8) {
            rows[row][col].value = val;

            for (int i = 0; i < 9; ++i) {
              possibilitiesArray[row][col][i] = 0;
              possibilitiesArray[row][i][val - 1] = 0;
              possibilitiesArray[i][col][val - 1] = 0;
            }

            int startRow = row - row % 3;
            int startCol = col - col % 3;

            for (int i = startRow; i < startRow + 3; ++i) {
              for (int j = startCol; j < startCol + 3; ++j) {
                possibilitiesArray[i][j][val - 1] = 0;
              }
            }

            count--;
          }
        }
      }

      if (count == countCopy) {
        for (int row = 0; row < 9; ++row) {
          for (int col = 0; col < 9; ++col) {

            for (int value : possibilitiesArray[row][col].values) {
              if (value == 0) {
                continue;
              }

              auto copy = Board(*this);

              copy[row][col].value = value;

              for (int i = 0; i < 9; ++i) {
                copy.possibilitiesArray[row][col][i] = 0;
                copy.possibilitiesArray[row][i][value - 1] = 0;
                copy.possibilitiesArray[i][col][value - 1] = 0;
              }

              int startRow = row - row % 3;
              int startCol = col - col % 3;

              for (int i = startRow; i < startRow + 3; ++i) {
                for (int j = startCol; j < startCol + 3; ++j) {
                  copy.possibilitiesArray[i][j][value - 1] = 0;
                }
              }

              if (copy.solve()) {
                for (int x = 0; x < 9; ++x) {
                  for (int y = 0; y < 9; ++y) {
                    if (copy[x][y].value != rows[x][y].value) {
                      rows[x][y].value = copy[x][y].value;
                    }
                  }
                }

                return true;
              }
            }
          }
        }

        return false;
      }

      countCopy = count;
    }

    return true;
  }

  Row &operator[](size_t row) {
    return rows[row];
  }

  friend std::ostream &operator<<(std::ostream &stream, Board board) {
    stream << '\n';

    for (int _ = 0; _ < 37; ++_) {
      stream << '-';
    }

    stream << '\n';

    for (auto &row : board.rows) {
      stream << row;
    }

    stream << '\n';

    return stream;
  }
};

int main() {
  int values[9][9] = {};

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
