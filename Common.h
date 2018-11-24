//
// Created by nhatminh2947 on 10/6/18.
//
#pragma once

#include <cstdint>
#include <tuple>

typedef int reward_t;
typedef uint16_t row_t;
typedef uint64_t board_t;
typedef uint8_t cell_t;

static const board_t ROW_MASK = 0xFFF;
static const cell_t CELL_MASK = 0xF;
static const board_t COL_MASK = 0x00F00F;

static board_t row_left_table[4096];
static board_t row_right_table[4096];
static board_t col_up_table[256];
static board_t col_down_table[256];
static int score_table[4096];

typedef std::tuple<float, float, float> tuple3;