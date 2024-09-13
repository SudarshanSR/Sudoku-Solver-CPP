#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <array>
#include <cstdint>
#include <map>
#include <ostream>
#include <set>

using Number = std::uint16_t;
using Row = std::uint16_t;
using Col = std::uint16_t;
using Count = std::size_t;
using Possibilities = std::set<Number>;
using Cell = std::pair<Row, Col>;

class Board {
  public:
    explicit Board(std::array<std::array<Number, 9>, 9> const &values);

    bool solve();

    friend std::ostream &operator<<(std::ostream &stream, Board const &board);

  private:
    std::array<std::array<Number, 9>, 9> board_ = {};
    std::array<std::array<Possibilities, 9>, 9> possibilities_board_ = {};
    std::array<std::array<Possibilities *, 9>, 9> possibilities_rows_ = {};
    std::array<std::array<Possibilities *, 9>, 9> possibilities_cols_ = {};
    std::map<Number, Count> remaining_numbers_ = {
        {1, 9}, {2, 9}, {3, 9}, {4, 9}, {5, 9}, {6, 9}, {7, 9}, {8, 9}, {9, 9},
    };
    std::set<Cell> remaining_cells_;

    void set_cell(Row row, Col col, Number number);

    void narrow_rows(Number number, Row row, Row row_start);

    void narrow_cols(Number number, Col col, Col col_start);

    void check_row(Number number, Row row);

    void check_col(Number number, Col col);

    void check_grid(Number number, Row row_start, Col col_start);
};

#endif