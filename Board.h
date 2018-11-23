#pragma once

#include <array>
#include <iostream>
#include <iomanip>
#include <cmath>

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
    typedef uint32_t cell;
    typedef std::array<cell, 3> row;
    typedef std::array<row, 2> grid;
    typedef uint64_t data;
    typedef int reward;

public:
    Board() : tile(), attr(0) {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 3; ++j) {
                operator()(i * 3 + j) = 0;
            }
        }
    }

    Board(const grid &b, data v = 0) : tile(b), attr(v) {}

    Board(const Board &b) = default;

    Board &operator=(const Board &b) = default;

    operator grid &() { return tile; }

    operator const grid &() const { return tile; }

    row &operator[](unsigned i) { return tile[i]; }

    const row &operator[](unsigned i) const { return tile[i]; }

    cell &operator()(unsigned i) { return tile[i / 4][i % 4]; }

    const cell &operator()(unsigned i) const { return tile[i / 4][i % 4]; }

    data info() const { return attr; }

    data info(data dat) {
        data old = attr;
        attr = dat;
        return old;
    }

public:
    bool operator==(const Board &b) const { return tile == b.tile; }

    bool operator<(const Board &b) const { return tile < b.tile; }

    bool operator!=(const Board &b) const { return !(*this == b); }

    bool operator>(const Board &b) const { return b < *this; }

    bool operator<=(const Board &b) const { return !(b < *this); }

    bool operator>=(const Board &b) const { return !(*this < b); }

public:

    /**
     * place a tile (index value) to the specific position (1-d form index)
     * return 0 if the action is valid, or -1 if not
     */
    reward Place(unsigned pos, cell tile) {
        if (pos >= 16) return -1;
        if (tile != 1 && tile != 2 && tile != 3) return -1;
        operator()(pos) = tile;
        return 0;
    }

    /**
     * apply an action to the board
     * return the reward of the action, or -1 if the action is illegal
     */
    reward Slide(unsigned opcode) {
        switch (opcode & 0b11) {
            case 0:
                return slide_up();
            case 1:
                return slide_right();
            case 2:
                return slide_down();
            case 3:
                return slide_left();
            default:
                return -1;
        }
    }

    reward slide_left() {
        Board prev = *this;
        reward score = 0;
        for (int r = 0; r < 2; r++) {
            auto &row = tile[r];
            int top = 0, hold = 0;

            for(int c = 0; c < 2; c++) {
                if(row[c] == 0) {
                    for(int k = c; k < 2; k++) {
                        row[k] = row[k+1];
                    }
                    row[2] = 0;
                    break;
                }
                else {
                    if(row[c] == row[c+1] && row[c] >= 3) {
                        row[c]++;

                        for(int k = c+1; k < 2; k++) {
                            row[k] = row[k+1];
                        }
                        row[2] = 0;

                        break;
                    }
                }
            }
        }

        return (*this != prev) ? score : -1;
    }

    reward slide_right() {
        Board prev = *this;
        reward score = 0;
        for (int r = 0; r < 2; r++) {
            auto &row = tile[r];
            int top = 0, hold = 0;

            for(int c = 2; c > 0; c--) {
                if(row[c] == 0) {
                    for(int k = c; k > 0; k--) {
                        row[k] = row[k-1];
                    }
                    row[0] = 0;
                    break;
                }
                else {
                    if(row[c] == row[c-1] && row[c] >= 3) {
                        row[c]++;

                        for(int k = c-1; k > 0; k--) {
                            row[k] = row[k-1];
                        }
                        row[0] = 0;

                        break;
                    }
                }
            }
        }

        return (*this != prev) ? score : -1;
    }

    reward slide_up() {
        Board prev = *this;
        auto &row0 = tile[0];
        auto &row1 = tile[1];

        reward score = 0;

        for (int c = 0; c < 3; ++c) {
            if(row0[c] == 0) {
                row0[c] = row1[c];
                row1[c] = 0;
            }
            else if(row0[c] == row1[c] && row0[c] >= 3) {
                row0[c]++;
                row1[c] = 0;
            }
        }

        return (*this != prev) ? score : -1;
    }

    reward slide_down() {
        Board prev = *this;
        auto &row0 = tile[0];
        auto &row1 = tile[1];

        reward score = 0;

        for (int c = 0; c < 3; ++c) {
            if(row1[c] == 0) {
                row1[c] = row0[c];
                row0[c] = 0;
            }
            else if(row1[c] == row0[c] && row1[c] >= 3) {
                row1[c]++;
                row0[c] = 0;
            }
        }

        return (*this != prev) ? score : -1;
    }

    void transpose() {
        for (int r = 0; r < 4; r++) {
            for (int c = r + 1; c < 4; c++) {
                std::swap(tile[r][c], tile[c][r]);
            }
        }
    }

    void reflect_horizontal() {
        for (int r = 0; r < 4; r++) {
            std::swap(tile[r][0], tile[r][3]);
            std::swap(tile[r][1], tile[r][2]);
        }
    }

    void reflect_vertical() {
        for (int c = 0; c < 4; c++) {
            std::swap(tile[0][c], tile[3][c]);
            std::swap(tile[1][c], tile[2][c]);
        }
    }

    /**
     * rotate the board clockwise by given times
     */
    void rotate(int r = 1) {
        switch (((r % 4) + 4) % 4) {
            default:
            case 0:
                break;
            case 1:
                rotate_right();
                break;
            case 2:
                reverse();
                break;
            case 3:
                rotate_left();
                break;
        }
    }

    void rotate_right() {
        transpose();
        reflect_horizontal();
    } // clockwise
    void rotate_left() {
        transpose();
        reflect_vertical();
    } // counterclockwise
    void reverse() {
        reflect_horizontal();
        reflect_vertical();
    }

public:
    friend std::ostream &operator<<(std::ostream &out, const Board &b) {
        for (int i = 0; i < 6; i++) {
            int value = b(i);
            if (value >= 3) {
                value = pow(3, b(i) - 2);
            }

            out << std::setw(std::min(i, 1)) << "" << value;
        }
        return out;
    }

    friend std::istream &operator>>(std::istream &in, Board &b) {
        for (int i = 0; i < 6; i++) {
            while (!std::isdigit(in.peek()) && in.good()) in.ignore(1);
            in >> b(i);
            b(i) = std::log2(b(i));
        }
        return in;
    }

private:
    grid tile;
    data attr;
};
