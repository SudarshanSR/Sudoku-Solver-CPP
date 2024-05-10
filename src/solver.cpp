#include "solver.hpp"

#include <algorithm>
#include <ostream>

Board::Board(std::array<std::array<Number, 9>, 9> const &values) {
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

bool Board::solve() {
  short countCopy = this->remainingCells.size();

  while (!this->remainingCells.empty()) {
    for (auto const [value, _] : this->remainingValues) {
      for (unsigned short row = 0; row < 9; row += 3) {
        for (unsigned short col = 0; col < 9; col += 3) {
          this->checkGrid(row, col, value);
        }
      }

      for (unsigned short i = 0; i < 9; ++i) {
        // Checks if there are any unique cells in a row and col where only this number can occur

        this->checkRow(value, i);

        this->checkCol(value, i);

        // Narrows down cells where this value should occur at

        unsigned short iStart = i - i % 3;

        this->narrowRows(value, i, iStart);

        this->narrowCols(value, i, iStart);
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

    Board copy(this->board);

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

std::ostream &operator<<(std::ostream &stream, Board const &board) {
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

void Board::setCell(unsigned short const row, unsigned short const col, Number const value) {
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

  this->remainingCells.erase({row, col});
}

void Board::narrowRows(Number const value, unsigned short const row, unsigned short const rowStart) {
  std::array<Possibilities *, 9> const &possibilitiesArray = this->possibilitiesRows[row];

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

void Board::narrowCols(Number const value, unsigned short const col, unsigned short const colStart) {
  std::array<Possibilities *, 9> const &possibilitiesArray = this->possibilitiesCols[col];

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

void Board::checkRow(Number const value, unsigned short const row) {
  std::array<Possibilities *, 9> const &possibilitiesArray = this->possibilitiesRows[row];

  if (std::ranges::count_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; }) != 1)
    return;

  this->setCell(
      row,
      std::ranges::find_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; }) -
          possibilitiesArray.begin(),
      value
  );
}

void Board::checkCol(Number const value, unsigned short const col) {
  std::array<Possibilities *, 9> const &possibilitiesArray = this->possibilitiesCols[col];

  if (std::ranges::count_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; }) != 1)
    return;

  this->setCell(
      std::ranges::find_if(possibilitiesArray, [value](Possibilities *p) { return (*p)[value - 1]; }) -
          possibilitiesArray.begin(),
      col, value
  );
}

void Board::checkGrid(unsigned short const row, unsigned short const col, Number const value) {
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
    return;

  this->setCell(valueRow, valueCol, value);
}
