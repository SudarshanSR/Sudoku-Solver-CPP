#include "solver.hpp"

#include <algorithm>
#include <ostream>
#include <ranges>

namespace {
std::optional<std::uint16_t>
check_count(std::array<Possibilities *, 9> const &array, Number const number) {
    std::optional<std::uint16_t> result;

    for (auto const &[index, possibilities] : array | std::views::enumerate) {
        if (!possibilities->contains(number))
            continue;

        if (result)
            return {};

        result = index;
    }

    return result;
}

std::optional<Cell> check_count(
    std::array<std::array<Possibilities, 9>, 9> const &board,
    Number const number, Row const row_start, Col const col_start
) {
    Count zero_count = 9;

    std::optional<Cell> cell;

    for (std::uint16_t i = 0; i < 3; ++i)
        for (std::uint16_t j = 0; j < 3; ++j) {
            if (!board[row_start + i][col_start + j].contains(number))
                continue;

            zero_count -= 1;

            if (zero_count == 7)
                return {};

            cell = {row_start + i, col_start + j};
        }

    return cell;
}
} // namespace

Board::Board(std::array<std::array<Number, 9>, 9> const &values) {
    for (Row row = 0; row < 9; ++row)
        for (Col col = 0; col < 9; ++col) {
            this->data_.possibilities_board_[row][col] = {1, 2, 3, 4, 5,
                                                          6, 7, 8, 9};
            this->data_.possibilities_rows_[row][col] =
                &this->data_.possibilities_board_[row][col];
            this->data_.possibilities_cols_[col][row] =
                &this->data_.possibilities_board_[row][col];

            this->data_.remaining_cells_.insert({row, col});
        }

    for (Row row = 0; row < 9; ++row)
        for (Col col = 0; col < 9; ++col)
            if (values[row][col])
                this->set_cell(row, col, values[row][col]);
}

void Board::solve() {
    Count count_copy = this->data_.remaining_cells_.size();

    while (!this->data_.remaining_cells_.empty()) {
        for (auto const &[number, count] : this->data_.remaining_numbers_) {
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

        for (auto const &[row, col] : this->data_.remaining_cells_) {
            if (this->data_.possibilities_board_[row][col].size() != 8)
                continue;

            this->set_cell(
                row, col, *this->data_.possibilities_board_[row][col].cbegin()
            );
        }

        // Checks if all useful moves have been used

        if (Count const count = this->data_.remaining_cells_.size();
            count != count_copy) {
            count_copy = count;

            continue;
        }

        // Starts guessing values

        auto [row, col] = *this->data_.remaining_cells_.cbegin();

        if (this->data_.possibilities_board_[row][col].empty()) {
            while (this->data_.possibilities_board_[row][col].empty()) {
                ChangedState &state = states_.top();

                row = state.row;
                col = state.col;

                this->data_ = std::move(state.data);

                this->data_.possibilities_board_[row][col].erase(state.number);

                states_.pop();
            }

            continue;
        }

        Number const number =
            *this->data_.possibilities_board_[row][col].cbegin();

        states_.emplace(row, col, number, this->data_);

        this->set_cell(row, col, number);
    }
}

std::ostream &operator<<(std::ostream &stream, Board const &board) {
    stream << '\n';

    for (int _ = 0; _ < 37; ++_)
        stream << '-';

    stream << '\n';

    for (std::array<Number, 9> const &row : board.data_.board_) {
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
    this->data_.board_[row][col] = number;

    this->data_.possibilities_board_[row][col].clear();

    for (Possibilities *p : this->data_.possibilities_rows_[row])
        p->erase(number);

    for (Possibilities *p : this->data_.possibilities_cols_[col])
        p->erase(number);

    Row const start_row = row - row % 3;
    Col const start_col = col - col % 3;

    for (std::array<Possibilities, 9> &array :
         this->data_.possibilities_board_ | std::views::drop(start_row) |
             std::views::take(3))
        for (Possibilities &possibilities :
             array | std::views::drop(start_col) | std::views::take(3))
            possibilities.erase(number);

    --this->data_.remaining_numbers_[number];

    this->data_.remaining_cells_.erase({row, col});
}

void Board::narrow_rows(
    Number const number, Row const row, Row const row_start
) {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->data_.possibilities_rows_[row];

    for (Col col_start = 0; col_start < 9; col_start += 3) {
        Count const row_count = std::ranges::count_if(
            possibilities_array | std::views::drop(col_start) |
                std::views::take(3),
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (!row_count)
            continue;

        Count grid_count = 0;

        for (std::uint16_t offset = 0; offset < 3; ++offset)
            grid_count += std::ranges::count_if(
                this->data_.possibilities_rows_[row_start + offset] |
                    std::views::drop(col_start) | std::views::take(3),
                [number](Possibilities const *const p) -> bool {
                    return p->contains(number);
                }
            );

        if (grid_count && grid_count == row_count) {
            for (Possibilities *const possibilities :
                 possibilities_array | std::views::take(col_start))
                possibilities->erase(number);

            for (Possibilities *const possibilities :
                 possibilities_array | std::views::drop(col_start + 3))
                possibilities->erase(number);
        }

        Count const total_count = std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (!total_count || total_count != row_count)
            continue;

        for (Row row_offset = 0; row_offset < 3; ++row_offset) {
            if (row_start + row_offset == row)
                continue;

            for (Possibilities *const possibilities :
                 this->data_.possibilities_rows_[row_start + row_offset] |
                     std::views::drop(col_start) | std::views::take(3))
                possibilities->erase(number);
        }
    }
}

void Board::narrow_cols(
    Number const number, Col const col, Col const col_start
) {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->data_.possibilities_cols_[col];

    for (Row row_start = 0; row_start < 9; row_start += 3) {
        Count const col_count = std::ranges::count_if(
            possibilities_array | std::views::drop(row_start) |
                std::views::take(3),
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (!col_count)
            continue;

        Count grid_count = 0;

        for (std::uint16_t offset = 0; offset < 3; ++offset)
            grid_count += std::ranges::count_if(
                this->data_.possibilities_cols_[col_start + offset] |
                    std::views::drop(row_start) | std::views::take(3),
                [number](Possibilities const *const p) -> bool {
                    return p->contains(number);
                }
            );

        if (grid_count && grid_count == col_count) {
            for (Possibilities *const possibilities :
                 possibilities_array | std::views::take(row_start))
                possibilities->erase(number);

            for (Possibilities *const possibilities :
                 possibilities_array | std::views::drop(row_start + 3))
                possibilities->erase(number);
        }

        Count const total_count = std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (!total_count || total_count != col_count)
            continue;

        for (Col col_offset = 0; col_offset < 3; ++col_offset) {
            if (col_start + col_offset == col)
                continue;

            for (Possibilities *const possibilities :
                 this->data_.possibilities_cols_[col_start + col_offset] |
                     std::views::drop(row_start) | std::views::take(3))
                possibilities->erase(number);
        }
    }
}

void Board::check_row(Number const number, Row const row) {
    std::optional<Col> const &col =
        check_count(this->data_.possibilities_rows_[row], number);

    if (!col)
        return;

    this->set_cell(row, *col, number);
}

void Board::check_col(Number const number, Col const col) {
    std::optional<Row> const &row =
        check_count(this->data_.possibilities_cols_[col], number);

    if (!row)
        return;

    this->set_cell(*row, col, number);
}

void Board::check_grid(
    Number const number, Row const row_start, Col const col_start
) {
    std::optional<Cell> const &cell = check_count(
        this->data_.possibilities_board_, number, row_start, col_start
    );

    if (!cell)
        return;

    this->set_cell(cell->first, cell->second, number);
}