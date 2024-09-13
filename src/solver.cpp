#include "solver.hpp"

#include <algorithm>
#include <ostream>
#include <ranges>

Board::Board(std::array<std::array<Number, 9>, 9> const &values) {
    for (Row row = 0; row < 9; ++row)
        for (Col col = 0; col < 9; ++col) {
            this->possibilities_board_[row][col] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
            this->possibilities_rows_[row][col] =
                &this->possibilities_board_[row][col];
            this->possibilities_cols_[col][row] =
                &this->possibilities_board_[row][col];

            this->remaining_cells_.insert({row, col});
        }

    for (Row row = 0; row < 9; ++row)
        for (Col col = 0; col < 9; ++col)
            if (values[row][col])
                this->set_cell(row, col, values[row][col]);
}

bool Board::solve() {
    Count count_copy = this->remaining_cells_.size();

    while (!this->remaining_cells_.empty()) {
        for (auto const &[number, count] : this->remaining_numbers_) {
            if (!count)
                continue;

            for (Row row = 0; row < 9; row += 3)
                for (Col col = 0; col < 9; col += 3)
                    this->check_grid(number, row, col);

            for (std::uint16_t i = 0; i < 9; ++i) {
                // Checks if there are any unique cells in a row and col where
                // only this number can occur

                this->check_row(number, i);

                this->check_col(number, i);

                // Narrows down cells where this value should occur at

                std::uint16_t const i_start = i - i % 3;

                this->narrow_rows(number, i, i_start);

                this->narrow_cols(number, i, i_start);
            }
        }

        // Checks if there's any cell where only one number is possible

        for (auto const &[row, col] : this->remaining_cells_) {
            if (this->possibilities_board_[row][col].size() != 8)
                continue;

            this->set_cell(
                row, col, *this->possibilities_board_[row][col].cbegin()
            );
        }

        // Checks if all useful moves have been used

        if (Count const count = this->remaining_cells_.size();
            count != count_copy) {
            count_copy = count;

            continue;
        }

        // Starts guessing values

        auto const &[row, col] = *this->remaining_cells_.cbegin();

        if (this->possibilities_board_[row][col].empty())
            return false;

        Number const number = *this->possibilities_board_[row][col].cbegin();

        Board copy(this->board_);

        copy.set_cell(row, col, number);

        if (copy.solve()) {
            this->board_ = copy.board_;

            return true;
        }

        this->possibilities_board_[row][col].erase(number);

        if (this->possibilities_board_[row][col].empty())
            return false;
    }

    return true;
}

std::ostream &operator<<(std::ostream &stream, Board const &board) {
    stream << '\n';

    for (int _ = 0; _ < 37; ++_)
        stream << '-';

    stream << '\n';

    for (std::array<Number, 9> const &row : board.board_) {
        stream << "|";

        for (Number const &cell : row) {
            cell ? stream << ' ' << cell << ' ' : stream << "   ";

            stream << '|';
        }

        stream << '\n';

        for (int _ = 0; _ < 37; ++_)
            stream << '-';

        stream << '\n';
    }

    return stream;
}

void Board::set_cell(Row const row, Col const col, Number const number) {
    this->board_[row][col] = number;

    this->possibilities_board_[row][col].clear();

    for (Possibilities *p : this->possibilities_rows_[row])
        p->erase(number);

    for (Possibilities *p : this->possibilities_cols_[col])
        p->erase(number);

    Row const start_row = row - row % 3;
    Col const start_col = col - col % 3;

    for (Row i = start_row; i < start_row + 3; ++i)
        for (Col j = start_col; j < start_col + 3; ++j)
            this->possibilities_board_[i][j].erase(number);

    --this->remaining_numbers_[number];

    this->remaining_cells_.erase({row, col});
}

void Board::narrow_rows(
    Number const number, Row const row, Row const row_start
) {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->possibilities_rows_[row];

    for (Col col_start = 0; col_start < 9; col_start += 3) {
        Count grid_count = 0;

        for (std::uint16_t offset = 0; offset < 3; ++offset)
            grid_count += std::ranges::count_if(
                this->possibilities_board_[row_start + offset] |
                    std::views::drop(col_start) | std::views::take(3),
                [number](Possibilities const &p) { return p.contains(number); }
            );

        Count const row_count = std::ranges::count_if(
            possibilities_array | std::views::drop(col_start) |
                std::views::take(3),
            [number](Possibilities const *p) { return p->contains(number); }
        );

        if (grid_count && grid_count == row_count) {
            for (Col col = 0; col < col_start; ++col)
                possibilities_array[col]->erase(number);

            for (Col col = col_start + 3; col < 9; ++col)
                possibilities_array[col]->erase(number);
        }

        Count const total_count = std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) { return p->contains(number); }
        );

        if (total_count && total_count == row_count)
            for (Row rowOffset = 0; rowOffset < 3; ++rowOffset) {
                if (row_start + rowOffset == row)
                    continue;

                for (Col colOffset = 0; colOffset < 3; ++colOffset)
                    this->possibilities_board_[row_start + rowOffset]
                                              [col_start + colOffset]
                                                  .erase(number);
            }
    }
}

void Board::narrow_cols(
    Number const number, Col const col, Col const col_start
) {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->possibilities_cols_[col];

    for (Row row_start = 0; row_start < 9; row_start += 3) {
        Count grid_count = 0;

        for (std::uint16_t offset = 0; offset < 3; ++offset)
            grid_count += std::ranges::count_if(
                this->possibilities_board_[row_start + offset] |
                    std::views::drop(col_start) | std::views::take(3),
                [number](Possibilities const &p) { return p.contains(number); }
            );

        Count const col_count = std::ranges::count_if(
            possibilities_array | std::views::drop(row_start) |
                std::views::take(3),
            [number](Possibilities const *p) { return p->contains(number); }
        );

        if (grid_count && grid_count == col_count) {
            for (Row row = 0; row < row_start; ++row)
                possibilities_array[row]->erase(number);

            for (Row row = row_start + 3; row < 9; ++row)
                possibilities_array[row]->erase(number);
        }

        Count const total_count = std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) { return p->contains(number); }
        );

        if (total_count && total_count == col_count)
            for (Col colOffset = 0; colOffset < 3; ++colOffset) {
                if (col_start + colOffset == col)
                    continue;

                for (Row rowOffset = 0; rowOffset < 3; ++rowOffset)
                    this->possibilities_board_[row_start + rowOffset]
                                              [col_start + colOffset]
                                                  .erase(number);
            }
    }
}

void Board::check_row(Number const number, Row const row) {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->possibilities_rows_[row];

    if (std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) { return p->contains(number); }
        ) != 1)
        return;

    this->set_cell(
        row,
        std::ranges::find_if(
            possibilities_array,
            [number](Possibilities const *p) { return p->contains(number); }
        ) - possibilities_array.begin(),
        number
    );
}

void Board::check_col(Number const number, Col const col) {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->possibilities_cols_[col];

    if (std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) { return p->contains(number); }
        ) != 1)
        return;

    this->set_cell(
        std::ranges::find_if(
            possibilities_array,
            [number](Possibilities const *p) { return p->contains(number); }
        ) - possibilities_array.begin(),
        col, number
    );
}

void Board::check_grid(
    Number const number, Row const row_start, Col const col_start
) {
    Count zero_count = 0;
    Row row = 0;
    Col col = 0;

    for (std::uint16_t i = 0; i < 3; ++i)
        for (std::uint16_t j = 0; j < 3; ++j)
            if (!this->possibilities_board_[row_start + i][col_start + j]
                     .contains(number)) {
                zero_count += 1;
            } else {
                row = row_start + i;
                col = col_start + j;
            }

    if (zero_count != 8)
        return;

    this->set_cell(row, col, number);
}
