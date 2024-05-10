#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <array>
#include <map>
#include <ostream>
#include <set>

using Number = unsigned short;
using Possibilities = std::array<bool, 9>;
using Cell = std::pair<unsigned short, unsigned short>;

class Board {
public:
  explicit Board(std::array<std::array<Number, 9>, 9> const &values);

  bool solve();

  friend std::ostream &operator<<(std::ostream &stream, Board const &board);

private:
  std::array<std::array<Number, 9>, 9> board = {};
  std::array<std::array<Possibilities, 9>, 9> possibilitiesBoard = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesRows = {};
  std::array<std::array<Possibilities *, 9>, 9> possibilitiesCols = {};
  std::map<Number, unsigned short> remainingValues = {
      {1, 9}, {2, 9}, {3, 9}, {4, 9}, {5, 9}, {6, 9}, {7, 9}, {8, 9}, {9, 9},
  };
  std::set<Cell> remainingCells;

  void setCell(unsigned short const row, unsigned short const col, Number const value);

  void narrowRows(Number const value, unsigned short const row, unsigned short const rowStart);

  void narrowCols(Number const value, unsigned short const col, unsigned short const colStart);

  void checkRow(Number const value, unsigned short const row);

  void checkCol(Number const value, unsigned short const col);

  void checkGrid(unsigned short const row, unsigned short const col, Number const value);
};

#endif