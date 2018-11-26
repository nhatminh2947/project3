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
    state_hint(const Board &board) : state(const_cast<Board &>(board)) {}

    char type() const { return state.info() ? state.info() + '0' : 'x'; }

    operator cell_t () const { return state.info(); }

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
        after_state_lut.resize((unsigned long) (pow(BASE, 6)),
                               std::vector<std::vector<tuple3>>(4, std::vector<tuple3>(5, std::make_tuple(
                                       INT32_MAX, 0.0f, INT32_MIN))));
        before_state_lut.resize((unsigned long) (pow(BASE, 6)),
                                std::vector<tuple3>(4, std::make_tuple(INT32_MAX, 0.0f, INT32_MIN)));

        Expectimax(0, Board(), 0, 7, 0);
    }

    tuple3 GetLookUpValue(int state, Board board, int hint, int last) {
        if (state == 0) {
            return after_state_lut[board.GetId()][hint][last];
        }
        return before_state_lut[board.GetId()][hint];
    }

    void SetLookUpValue(int state, Board board, int hint, int last, std::tuple<float, float, float> value) {
        if (state == 0) {
            after_state_lut[board.GetId()][hint][last] = value;
        } else before_state_lut[board.GetId()][hint] = value;
    }

    tuple3 Expectimax(int state, Board board, int player_move, int bag, int hint) {
        if(board.IsValid() == false) {
            std::cout << board << std::endl;
        }

        if (board.GetId() != 0 && std::get<0>(GetLookUpValue(state, board, hint, player_move)) != INT32_MAX) {
            return GetLookUpValue(state, board, hint, player_move);
        }

        if (hint != 0 && IsTerminal(board)) {
            tuple3 answer = std::make_tuple(0, 0, 0);
            SetLookUpValue(state, board, hint, player_move, answer);

            return answer;
        }

        if (state == 1) { // Max node - before state
            tuple3 answer = std::make_tuple(INT32_MAX, 0, INT32_MIN);
            float max_reward = -1;
            for (int d = 0; d < 4; ++d) { //direction
                Board child = Board(board);
                child.Slide(d);
                if (child == board) continue;
                float score = child.GetScore() - board.GetScore();

                tuple3 expect = Expectimax(1 - state, child, d + 1, bag, hint);

                if (score + std::get<1>(expect) > max_reward) {
                    max_reward = score + std::get<1>(expect);
                    answer = std::make_tuple(score + std::get<0>(expect),
                                             score + std::get<1>(expect),
                                             score + std::get<2>(expect));
                }
            }

            SetLookUpValue(state, board, hint, player_move, answer);

            return answer;
        }

        //Average node - after state
        std::vector<int> positions = getPlacingPosition(player_move);
        tuple3 answer = std::make_tuple(INT32_MAX, 0, INT32_MIN);

        int count_child_node = 0;
        for (unsigned int position : positions) {
            if (board(position) != 0) continue;
            if (hint != 0) {
                Board child = Board(board);
                child.Place(position, hint);
                float score = child.GetScore() - board.GetScore();

                int child_bag = bag ^(1 << (hint - 1));
                if (child_bag == 0) {
                    child_bag = 7;
                }

                for (int h = 1; h <= 3; h++) {
                    if (((1 << (h - 1)) & child_bag) != 0) {
                        tuple3 expect = Expectimax(1 - state, child, 0, child_bag, h);

                        answer = std::make_tuple(std::min(std::get<0>(answer), score + std::get<0>(expect)),
                                                 std::get<1>(answer) + score + std::get<1>(expect),
                                                 std::max(std::get<2>(answer), score + std::get<2>(expect)));

                        count_child_node++;
                    }
                }
            } else {
                for (int tile = 1; tile <= 3; ++tile) {
                    if (((1 << (tile - 1)) & bag) != 0) {
                        Board child = Board(board);
                        child.Place(position, tile);
                        float score = child.GetScore() - board.GetScore();

                        int child_bag = bag ^(1 << (tile - 1));
                        if (child_bag == 0) {
                            child_bag = 7;
                        }

                        for (int h = 1; h <= 3; h++) {
                            if (((1 << (h - 1)) & child_bag) != 0) {
                                tuple3 expect = Expectimax(1 - state, child, 0, child_bag, h);

                                answer = std::make_tuple(std::min(std::get<0>(answer), score + std::get<0>(expect)),
                                                         std::get<1>(answer) + score + std::get<1>(expect),
                                                         std::max(std::get<2>(answer), score + std::get<2>(expect)));

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

    std::vector<int> getPlacingPosition(int player_move) {
        if (player_move == 1) {
            return std::vector<int>{3, 4, 5};
        }

        if (player_move == 2) {
            return std::vector<int>{0, 3};
        }

        if (player_move == 3) {
            return std::vector<int>{0, 1, 2};
        }

        if (player_move == 4) {
            return std::vector<int>{2, 5};
        }

        return std::vector<int>{0, 1, 2, 3, 4, 5};
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

    tuple3 solve(const Board &state, state_type type = state_type::before) {
        cell_t hint = state_hint(state);

        if (!state.IsValid() || hint > 3 || hint < 1) {
            return std::make_tuple(INT32_MAX, 0, INT32_MIN);
        }

        if (type.is_after()) {

            for (int last = 1; last < 4; ++last) {
                tuple3 after_state_result = after_state_lut[state.GetId()][hint][last];

                if (std::get<0>(after_state_result) != INT32_MAX) {
                    return after_state_result;
                }
            }
            return std::make_tuple(INT32_MAX, 0, INT32_MIN);
        } else if (type.is_before()) {
            return before_state_lut[state.GetId()][hint];
        }
    }

private:
    std::vector<std::vector<std::vector<tuple3>>> after_state_lut;
    std::vector<std::vector<tuple3>> before_state_lut;
};
