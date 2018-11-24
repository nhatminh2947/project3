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

static row_t reverse_row(row_t row) {
    return (row >> 8) | (row & 0x0F0) | ((row << 8) & 0xF00);
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

        for(i=0; i<3; i++) {
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

        for(int j=i+1; j<3; j++)
            cell[j] = cell[j+1];
        cell[3] = 0;

        row_t result = row_t((cell[0] <<  0) |
                (cell[1] <<  4) |
                (cell[2] <<  8));

        row_t rev_result = reverse_row(result);
        unsigned rev_row = reverse_row(row);

        row_left_table [    row] =                row  ^                result;
        row_right_table[rev_row] =            rev_row  ^            rev_result;
    }
}