#pragma once

#include <array>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "Common.h"

/**
 * array-based board for 2048
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */

class Board {
public:

public:
    Board() : board_() {}

    Board(const board_t &board) : board_(board), attr(0) {}

    Board(const Board &board) = default;

    Board &operator=(const Board &b) = default;

    explicit operator board_t &() { return board_; }

    explicit operator const board_t &() const { return board_; }

    row_t operator[](unsigned i) {
        return row_t((board_ >> (i * 12)) & ROW_MASK);
    }

    const row_t operator[](unsigned i) const {
        return row_t((board_ >> (i * 12)) & ROW_MASK);
    }

    cell_t operator()(unsigned i) {
        return cell_t((board_ >> (i * 4)) & CELL_MASK);
    }

    const cell_t operator()(unsigned i) const {
        return cell_t((board_ >> (i * 4)) & CELL_MASK);
    }

    board_t GetId() {
        int p = 1;
        board_t id = 0;
        for (int i = 0; i < 6; ++i) {
            id += this->operator()(i) * p;
            p *= BASE;
        }
        return id;
    }

    const board_t GetId() const {
        int p = 1;
        board_t id = 0;
        for (int i = 0; i < 6; ++i) {
            id += this->operator()(i) * p;
            p *= BASE;
        }
        return id;
    }

    const bool IsValid() const {
        for (int i = 0; i < 6; ++i) {
            if(this->operator()(i) > BASE) {
                return false;
            }
        }
        return true;
    }

    bool IsValid() {
        for (int i = 0; i < 6; ++i) {
            if(this->operator()(i) > BASE) {
                return false;
            }
        }
        return true;
    }

    uint64_t info() const { return attr; }

    uint64_t info(uint64_t dat) {
        uint64_t old = attr;
        attr = dat;
        return old;
    }

public:
    bool operator==(const Board &b) const { return board_ == b.board_; }

    bool operator<(const Board &b) const { return board_ < b.board_; }

    bool operator!=(const Board &b) const { return !(*this == b); }

    bool operator>(const Board &b) const { return b < *this; }

    bool operator<=(const Board &b) const { return !(b < *this); }

    bool operator>=(const Board &b) const { return !(*this < b); }

public:

    /**
     * place a tile (index value) to the specific position (1-d form index)
     * return 0 if the action is valid, or -1 if not
     */
    reward_t Place(unsigned int position, cell_t tile) {
        if (position >= 16) return -1;
        if (tile != 1 && tile != 2 && tile != 3) return -1;

        board_ = board_ | (tile << (position * 4));

        return 0;
    }

    void Assign(unsigned int position, cell_t tile) {
        if (tile < 0 || position >= 6) return;

        board_ = board_ ^ (operator()(position) << (position * 4));
        board_ = board_ | (tile << (position * 4));

        return;
    }

    /**
     * apply an action to the board
     * return the reward of the action, or -1 if the action is illegal
     */
    reward_t Slide(unsigned opcode) {
        switch (opcode & 0b11) {
            case 0:
                return SlideUp();
            case 1:
                return SlideRight();
            case 2:
                return SlideDown();
            case 3:
                return SlideLeft();
            default:
                return -1;
        }
    }

    reward_t SlideLeft() {
        board_t ret = board_;

        ret ^= board_t(row_left_table[(board_ >> 0) & ROW_MASK]) << 0;
        ret ^= board_t(row_left_table[(board_ >> 12) & ROW_MASK]) << 12;

        this->board_ = ret;

        return 0;
    }

    reward_t SlideRight() {
        board_t ret = board_;

        ret ^= board_t(row_right_table[(board_ >> 0) & ROW_MASK]) << 0;
        ret ^= board_t(row_right_table[(board_ >> 12) & ROW_MASK]) << 12;

        this->board_ = ret;

        return 0;
    }

    reward_t SlideUp() {
        board_t ret = board_;

        ret ^= col_up_table[this->operator()(0) | (this->operator()(3) << 4)] << 0;
        ret ^= col_up_table[this->operator()(1) | (this->operator()(4) << 4)] << 4;
        ret ^= col_up_table[this->operator()(2) | (this->operator()(5) << 4)] << 8;

        this->board_ = ret;

        return 0;
    }

    reward_t SlideDown() {
        board_t ret = board_;

        ret ^= col_down_table[this->operator()(0) | (this->operator()(3) << 4)] << 0;
        ret ^= col_down_table[this->operator()(1) | (this->operator()(4) << 4)] << 4;
        ret ^= col_down_table[this->operator()(2) | (this->operator()(5) << 4)] << 8;

        this->board_ = ret;

        return 0;
    }

    reward_t GetScore() {
        return score_table[board_ & ROW_MASK]
               + score_table[(board_ >> 12) & ROW_MASK];
    }

public:
    friend std::ostream &operator<<(std::ostream &out, const Board &b) {
        for (int i = 0; i < 6; i++) {
            int value = b(i);
            if (value > 3) {
                value = 3 * pow(2, b(i) - 3);
            }

            out << std::setw(std::min(i, 1)) << "" << value;
        }
        return out;
    }

    friend std::istream &operator>>(std::istream &in, Board &b) {
        for (int i = 0; i < 6; i++) {
            while (!std::isdigit(in.peek()) && in.good()) in.ignore(1);
            int value;
            in >> value;
            if (value > 3) {
                value = std::log2(value / 3) + 3;
            }

            b.Assign(i, value);
        }
        return in;
    }

private:
    board_t board_;
    uint64_t attr;
};
