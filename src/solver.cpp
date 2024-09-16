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
    std::map<std::pair<std::uint16_t, std::uint16_t>, Possibilities *> const
        &grid,
    Number const number
) {
    Count zero_count = 9;

    std::optional<Cell> cell;

    for (auto const &[pair, possibilities] : grid) {
        if (!possibilities->contains(number))
            continue;

        zero_count -= 1;

        if (zero_count == 7)
            return {};

        cell = pair;
    }

    return cell;
}
} // namespace

Board::Board(std::array<std::array<Number, 9>, 9> const &values) {
    for (Row row = 0; row < 9; ++row) {
        Row const row_mod = row % 3;
        Row const row_floor = row / 3;

        for (Col col = 0; col < 9; ++col) {
            this->data_.possibilities_board_[row][col] = {1, 2, 3, 4, 5,
                                                          6, 7, 8, 9};
            this->data_.possibilities_rows_[row][col] =
                &this->data_.possibilities_board_[row][col];
            this->data_.possibilities_cols_[col][row] =
                &this->data_.possibilities_board_[row][col];

            {
                std::pair<std::uint16_t, std::uint16_t> const &grid_cell{
                    row_mod, col % 3
                };

                Col const col_mod = col / 3;

                this->data_.possibilities_grids_row_wise_[row_floor][col_mod]
                                                         [grid_cell] =
                    &this->data_.possibilities_board_[row][col];
                this->data_.possibilities_grids_col_wise_[col_mod][row_floor]
                                                         [grid_cell] =
                    &this->data_.possibilities_board_[row][col];
            }

            this->data_.remaining_cells_.insert({row, col});
        }
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

            for (std::uint16_t x = 0; x < 3; ++x)
                for (std::uint16_t y = 0; y < 3; ++y)
                    this->check_grid(number, x, y);

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

    for (Possibilities *const p : this->data_.possibilities_rows_[row])
        p->erase(number);

    for (Possibilities *const p : this->data_.possibilities_cols_[col])
        p->erase(number);

    for (Possibilities *const possibilities :
         this->data_.possibilities_grids_row_wise_[row / 3][col / 3] |
             std::views::values)
        possibilities->erase(number);

    --this->data_.remaining_numbers_[number];

    this->data_.remaining_cells_.erase({row, col});
}

void Board::narrow_rows(Number const number, Row const row, Row const row_start)
    const {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->data_.possibilities_rows_[row];

    auto chunks = possibilities_array | std::views::chunk(3);
    auto grid = this->data_.possibilities_grids_row_wise_[row / 3];

    for (std::uint16_t i = 0; i < 3; ++i) {
        Count const row_count = std::ranges::count_if(
            chunks[i],
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (!row_count)
            continue;

        Count const grid_count = std::ranges::count_if(
            grid[i] | std::views::values,
            [number](Possibilities const *const p) -> bool {
                return p->contains(number);
            }
        );

        if (grid_count == row_count)
            for (auto const &[index, array] : chunks | std::views::enumerate) {
                if (index == i)
                    continue;

                for (Possibilities *const possibilities : array)
                    possibilities->erase(number);
            }

        Count const total_count = std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (total_count != row_count)
            continue;

        for (auto const &[pair, possibilities] : grid[i]) {
            if (row_start + pair.first == row)
                continue;

            possibilities->erase(number);
        }
    }
}

void Board::narrow_cols(Number const number, Col const col, Col const col_start)
    const {
    std::array<Possibilities *, 9> const &possibilities_array =
        this->data_.possibilities_cols_[col];

    auto chunks = possibilities_array | std::views::chunk(3);
    auto grid = this->data_.possibilities_grids_col_wise_[col / 3];

    for (std::uint16_t i = 0; i < 3; ++i) {
        Count const col_count = std::ranges::count_if(
            chunks[i],
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (!col_count)
            continue;

        Count const grid_count = std::ranges::count_if(
            grid[i] | std::views::values,
            [number](Possibilities const *const p) -> bool {
                return p->contains(number);
            }
        );

        if (grid_count == col_count)
            for (auto const &[index, array] : chunks | std::views::enumerate) {
                if (index == i)
                    continue;

                for (Possibilities *const possibilities : array)
                    possibilities->erase(number);
            }

        Count const total_count = std::ranges::count_if(
            possibilities_array,
            [number](Possibilities const *p) -> bool {
                return p->contains(number);
            }
        );

        if (total_count != col_count)
            continue;

        for (auto const &[pair, possibilities] : grid[i]) {
            if (col_start + pair.second == col)
                continue;

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

void Board::check_grid(Number const number, Row const x, Col const y) {
    std::optional<Cell> const &cell =
        check_count(this->data_.possibilities_grids_row_wise_[x][y], number);

    if (!cell)
        return;

    this->set_cell(3 * x + cell->first, 3 * y + cell->second, number);
}