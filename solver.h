#pragma once

#include <iostream>
#include <algorithm>
#include <cmath>
#include "Board.h"
#include <numeric>

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
    state_hint(const Board &state) : state(const_cast<Board &>(state)) {}

    char type() const { return state.info() ? state.info() + '0' : 'x'; }

    operator Board::cell() const { return state.info(); }

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
	LUT.resize(2, std::vector<std::vector<std::vector<float>>>(262144, std::vector<std::vector<float>>(16, std::vector<float>(5))));
        Expectimax(0, Board(), -1, 7, 0);
        std::cout << "feel free to display some messages..." << std::endl;
    }

    int ScoreBoard(Board board) {
        int score = 0;
        for (int i = 0; i < 6; ++i) {
            int cell = board(i);

            if (cell >= 3) {
                score += int(pow(3, cell - 2));
            }
        }

        return score;
    }

    float GetLookUpValue(int state, Board board, int hint, int last) {
	long long bitboard = 0;
	for(int i = 0; i < 6; i++) {
		int value = board(i);
		bitboard |= (value << i*4);
	}
        return LUT[state][bitboard][hint][last];
    }

    void SetLookUpValue(int state, Board board, int hint, int last, float value) {
	long long bitboard = 0;
        for(int i = 0; i < 6; i++) {
                int value = board(i);
                bitboard |= (value << i*4);
        }
        LUT[state][bitboard][hint][last] = value;
    }

    float Expectimax(int state, Board board, int player_move, int bag, int hint) {
        if (GetLookUpValue(state, board, hint, player_move) != 0
            || (hint != 0 && IsTerminal(board))) {
		std::cout << "State: " << state % 2 << std::endl;
	        std::cout << "Board: " << board << std::endl;
        	std::cout << "last: "<< player_move << std::endl;
        	std::cout << "bag: "<< bag << std::endl;
        	std::cout << "hint: "<< hint << std::endl;

            std::cout << "TERMINAL AT" << std::endl;

            return ScoreBoard(board);
        }

        float score;
        if (state % 2 == 1) { // Max node - before state
            score = INT64_MIN;

            for (int d = 0; d < 4; ++d) { //direction
                Board child = Board(board);
                child.Slide(d);
                if (child == board) continue;

                score = fmaxf(score, Expectimax(state + 1, child, d, bag, hint));
            }

            SetLookUpValue(state % 2, board, hint, player_move + 1, score);
            return score;
        }

        //Average node - after state
        score = 0.0f;
        std::array<unsigned int, 6> positions = getPlacingPosition(player_move);

        int count_child_node = 0;
        for (unsigned int position : positions) {
            if (board(position) != 0) continue;
	    if (hint == 0) {
                for (int tile = 1; tile <= 3; ++tile) {
                    if (((1 << (tile - 1)) & bag) != 0) {
                        Board child = Board(board);
                        child.Place(position, tile);

                        int child_bag = bag ^ (1 << (tile - 1));
                        count_child_node++;
		        if(child_bag == 0) {
			    child_bag = 7;
		        }
		    
		        for(int h = 1; h <= 3; h++) {
			    if(((1 << (h - 1)) & child_bag) != 0) {
	                    	score += Expectimax(state + 1, child, -1, child_bag, h);
			    }
		    	}
                    }
            	}
	    }
	    else {
		Board child = Board(board);
                child.Place(position, hint);

                int child_bag = bag ^ (1 << (hint - 1));
                if(child_bag == 0) {
                    child_bag = 7;
                }

                for(int h = 1; h <= 3; h++) {
                    if(((1 << (h - 1)) & child_bag) != 0) {
                	score += Expectimax(state + 1, child, -1, child_bag, h);
                    }
                }
	    }
        }

        score = score / count_child_node;
        SetLookUpValue(state % 2, board, hint, player_move + 1, score);

        return score;
    }

    std::array<unsigned int, 6> getPlacingPosition(int player_move) {
        if (player_move == 0) {
            return {3, 4, 5};
        }

        if (player_move == 1) {
            return {0, 3};
        }

        if (player_move == 2) {
            return {0, 1, 2};
        }

        if (player_move == 3) {
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

    answer solve(const Board &state, state_type type = state_type::before) {
        // TODO: find the answer in the lookup table and return it
        //       do NOT recalculate the tree at here

        // to fetch the hint (if type == state_type::after, hint will be 0)
//		board::cell hint = state_hint(state);

        // for a legal state, return its three values.
//		return { min, avg, max };
        // for an illegal state, simply return {}
        return {};
    }

private:
    // TODO: place your transposition table here
    std::vector<std::vector<std::vector<std::vector<float>>>> LUT;
};
