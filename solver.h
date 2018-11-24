#pragma once

#include <iostream>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <tuple>

#include "Board.h"

class state_type {
public:
    enum type : char {
        before = 'b',
        after = 'a',
        illegal = 'i'
    };

public:
    state_type() : t(illegal) {}

    state_type(const state_type &st) = default;

    state_type(state_type::type code) : t(code) {}

    friend std::istream &operator>>(std::istream &in, state_type &type) {
        std::string s;
        if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
        return in;
    }

    friend std::ostream &operator<<(std::ostream &out, const state_type &type) {
        return out << char(type.t);
    }

    bool is_before() const { return t == before; }

    bool is_after() const { return t == after; }

    bool is_illegal() const { return t == illegal; }

private:
    type t;
};

class state_hint {
public:
    state_hint(const Board& state) : state(const_cast<Board&>(state)) {}

    char type() const { return state.info() ? state.info() + '0' : 'x'; }

//    operator Board::cell() const { return state.info(); }

public:
    friend std::istream &operator>>(std::istream &in, state_hint &hint) {
        while (in.peek() != '+' && in.good()) in.ignore(1);
        char v;
        in.ignore(1) >> v;
        hint.state.info(v != 'x' ? v - '0' : 0);
        return in;
    }

    friend std::ostream &operator<<(std::ostream &out, const state_hint &hint) {
        return out << "+" << hint.type();
    }

private:
    Board &state;
};


class solver {
public:
    typedef float value_t;

public:
    class answer {
    public:
        answer(value_t min = 0.0 / 0.0, value_t avg = 0.0 / 0.0, value_t max = 0.0 / 0.0) : min(min), avg(avg),
                                                                                            max(max) {}

        friend std::ostream &operator<<(std::ostream &out, const answer &ans) {
            return !std::isnan(ans.avg) ? (out << ans.min << " " << ans.avg << " " << ans.max) : (out << "-1")
                    << std::endl;
        }

    public:
        const value_t min, avg, max;
    };

public:
    solver(const std::string &args) {
        // TODO: explore the tree and save the result
        LUT.resize(2, std::vector<std::vector<std::vector<tuple3>>>(1000000,
                std::vector<std::vector<tuple3>>(3,
                        std::vector<tuple3>(5, std::make_tuple(INT32_MAX, 0.0f, INT32_MIN)))));

//        for (int i = 0; i < 2; ++i) {
//            for (int j = 0; j < 1000000; ++j) {
//                for (int k = 0; k < 3; ++k) {
//                    for (int l = 0; l < 5; ++l) {
//                        LUT[i][j][k][l] = std::make_tuple(123456, 0.0f, -123456);
//                    }
//                }
//            }
//        }

        Expectimax(0, Board(), 0, 7, 0);
        std::cout << "feel free to display some messages..." << std::endl;
    }

    std::tuple<float, float, float> GetLookUpValue(int state, Board board, int hint, int last) {
        return LUT[state][board.GetId()][hint - 1][last];
    }

    void SetLookUpValue(int state, Board board, int hint, int last, std::tuple<float, float, float> value) {
        LUT[state][board.GetId()][hint - 1][last] = value;
    }

    std::tuple<float, float, float> Expectimax(int state, Board board, int player_move, int bag, int hint) {
//        std::cout << "State: " << state % 2 << std::endl;
//        std::cout << "Board: " << board << std::endl;
//        std::cout << "last: " << player_move - 1 << std::endl;
//        std::cout << "bag: " << bag << std::endl;
//        std::cout << "hint: " << hint << std::endl;
//        std::cout << std::endl;

        if (board.GetId() != 0 && std::get<0>(GetLookUpValue(state, board, hint, player_move)) != INT32_MAX) {
            return GetLookUpValue(state, board, hint, player_move);
        }

        if(hint != 0 && IsTerminal(board)) {
//            std::cout << "TERMINATED" << std::endl;
//            std::cout << "State: " << state % 2 << std::endl;
//            std::cout << "Board: " << board << std::endl;
//            std::cout << "last: " << player_move - 1 << std::endl;
//            std::cout << "bag: " << bag << std::endl;
//            std::cout << "hint: " << hint << std::endl;
//            std::cout << std::endl;

            std::tuple<float, float, float> answer = std::make_tuple(0,
                                                                     0,
                                                                     0);

            SetLookUpValue(state, board, hint, player_move, answer);

            return answer;
        }

        tuple3 score;
        if (state == 1) { // Max node - before state
            std::tuple<float, float, float> answer = std::make_tuple(INT64_MAX, 0, INT64_MIN);

            for (int d = 0; d < 4; ++d) { //direction
                Board child = Board(board);
                child.Slide(d);
                if (child == board) continue;
                float reward = child.GetScore() - board.GetScore();

                score = Expectimax(1 - state, child, d + 1, bag, hint);

                if (std::get<1>(score) + reward > std::get<1>(answer)) {
                    answer = score;
                }
            }

            SetLookUpValue(state, board, hint, player_move, answer);
            return answer;
        }

        //Average node - after state
        std::array<unsigned int, 6> positions = getPlacingPosition(player_move);
        tuple3 answer = std::make_tuple(INT64_MAX, 0, INT64_MIN);

        int count_child_node = 0;
        for (unsigned int position : positions) {
            if (board(position) != 0) continue;
            if (hint != 0) {
                Board child = Board(board);
                child.Place(position, hint);
                int reward = child.GetScore() - board.GetScore();

                int child_bag = bag ^(1 << (hint - 1));
                if (child_bag == 0) {
                    child_bag = 7;
                }

                for (int h = 1; h <= 3; h++) {
                    if (((1 << (h - 1)) & child_bag) != 0) {
                        score = Expectimax(1 - state, child, 0, child_bag, h);

                        answer = std::make_tuple(std::min(std::get<0>(answer), reward + std::get<0>(score)),
                                                 std::get<1>(answer) + reward + std::get<1>(score),
                                                 std::max(std::get<2>(answer), reward + std::get<2>(score)));

                        count_child_node++;
                    }
                }
            } else {
                for (int tile = 1; tile <= 3; ++tile) {
                    if (((1 << (tile - 1)) & bag) != 0) {
                        Board child = Board(board);
                        child.Place(position, tile);
                        int reward = child.GetScore() - board.GetScore();

                        int child_bag = bag ^(1 << (tile - 1));
                        if (child_bag == 0) {
                            child_bag = 7;
                        }

                        for (int h = 1; h <= 3; h++) {
                            if (((1 << (h - 1)) & child_bag) != 0) {
                                score = Expectimax(1 - state, child, 0, child_bag, h);

                                answer = std::make_tuple(std::min(std::get<0>(answer), reward + std::get<0>(score)),
                                                         std::get<1>(answer) + reward + std::get<1>(score),
                                                         std::max(std::get<2>(answer), reward + std::get<2>(score)));

                                count_child_node++;
                            }
                        }
                    }
                }
            }
        }

        std::get<1>(answer) = std::get<1>(answer) / count_child_node;
        SetLookUpValue(state, board, hint, player_move, answer);

        return answer;
    }

    std::array<unsigned int, 6> getPlacingPosition(int player_move) {
        if (player_move == 1) {
            return {3, 4, 5};
        }

        if (player_move == 2) {
            return {0, 3};
        }

        if (player_move == 3) {
            return {0, 1, 2};
        }

        if (player_move == 4) {
            return {2, 5};
        }

        return {0, 1, 2, 3, 4, 5};
    }

    bool IsTerminal(Board &board) {
        for (int direction = 0; direction < 4; ++direction) {
            Board temp_board = Board(board);
            temp_board.Slide(direction);
            if (temp_board != board)
                return false;
        }

        return true;
    }

    std::tuple<float, float, float> solve(const Board &state, int hint, state_type type = state_type::before) {
        // TODO: find the answer in the lookup table and return it
        //       do NOT recalculate the tree at here

        // to fetch the hint (if type == state_type::after, hint will be 0)
//		board::cell hint = state_hint(state);

        // for a legal state, return its three values.
//		return { min, avg, max };
        // for an illegal state, simply return {}
        if(type.is_after()) {
            return LUT[0][state.GetId()][hint-1][0];
        }
        else {
            return LUT[1][state.GetId()][hint-1][0];
        }
        return {};
    }

private:
    // TODO: place your transposition table here
    std::vector<std::vector<std::vector<std::vector<tuple3>>>> LUT;
};
