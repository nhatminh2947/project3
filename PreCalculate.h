//
// Created by cgilab on 11/24/18.
//

#pragma once

#include "Board.h"
#include "Common.h"

int ScoreRow(board_t row) {
    int score = 0;
    for (int i = 0; i < 3; ++i) {
        cell_t cell = (row >> (i * 3) & CELL_MASK);

        if (cell >= 3) {
            score += int(pow(3, cell - 2));
        }
    }

    return score;
}

static board_t unpack_col(row_t col) {
    board_t tmp = col;
    return (tmp | (tmp << 8ULL)) & COL_MASK;
}

static row_t reverse_row(row_t row) {
    return (row >> 8) | (row & 0x0F0) | ((row << 8) & 0xF00);
}

static row_t reverse_col(row_t col) {
    return (col >> 4) | ((col << 4) & 0xF0);
}

void InitLookUpTables() {
    for(unsigned row = 0; row < 4096; ++row) {
        unsigned cell[3] = {
                (row >>  0) & 0xf,
                (row >>  4) & 0xf,
                (row >>  8) & 0xf
        };

        // Score
        float score = 0.0f;
        for (int i = 0; i < 3; ++i) {
            int rank = cell[i];
            if (rank >= 3) {
                score += powf(3, rank-2);
            }
        }
        score_table[row] = score;

        int i;

        for(i=0; i<2; i++) {
            if(cell[i] == 0) {
                cell[i] = cell[i+1];
                break;
            } else if(cell[i] == 1 && cell[i+1] == 2) {
                cell[i] = 3;
                break;
            } else if(cell[i] == 2 && cell[i+1] == 1) {
                cell[i] = 3;
                break;
            } else if(cell[i] == cell[i+1] && cell[i] >= 3) {
                if(cell[i] != 14) {
                    cell[i]++;
                }
                break;
            }
        }

        if(i == 2) continue;

        for(int j=i+1; j<2; j++)
            cell[j] = cell[j+1];
        cell[2] = 0;

        row_t result = row_t((cell[0] <<  0) |
                (cell[1] <<  4) |
                (cell[2] <<  8));

        row_t rev_result = reverse_row(result);
        unsigned rev_row = reverse_row(row);
        if(rev_row == 1) {
            std::cout << std::endl;
        }
        row_left_table [    row] =  row     ^   result;
        row_right_table[rev_row] =  rev_row ^   rev_result;
    }

    for(unsigned col = 0; col < 256; ++col) {
        unsigned cell[2] = {
                (col >>  0) & 0xf,
                (col >>  4) & 0xf
        };

        int i;

        for(i=0; i<1; i++) {
            if(cell[i] == 0) {
                cell[i] = cell[i+1];
                break;
            } else if(cell[i] == 1 && cell[i+1] == 2) {
                cell[i] = 3;
                break;
            } else if(cell[i] == 2 && cell[i+1] == 1) {
                cell[i] = 3;
                break;
            } else if(cell[i] == cell[i+1] && cell[i] >= 3) {
                if(cell[i] != 14) {
                    cell[i]++;
                }
                break;
            }
        }

        if(i == 1) continue;

        cell[1] = 0;

        row_t result = row_t((cell[0] <<  0) |
                             (cell[1] <<  4));

        row_t rev_result = reverse_col(result);
        unsigned rev_col = reverse_col(col);

        col_up_table  [    col] = unpack_col(col)       ^ unpack_col(result);
        col_down_table[rev_col] = unpack_col(rev_col)   ^ unpack_col(rev_result);
    }
}