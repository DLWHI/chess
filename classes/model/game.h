#ifndef GAME_H_
#define GAME_H_

#include "board.h"

#define C_RESULT_MASK 0xE0
#define C_TERMINATION_MASK 0xF

#define C_WHITE_WIN 0x80
#define C_BLACK_WIN 0x40
#define C_DRAW 0x20

#define C_AGREEMENT 0x8
#define C_ON_TIME 0x4
#define C_MATE 0x2
#define C_RESIGNATION 1

namespace chess
{
    class Game 
    {
        public:
            Game();
            Game(const std::string& data, bool pgn = false); // input from fen or pgn notations
            // Game(Converter& cnv); deprecated
            Game(const Game& other);
            ~Game() = default;

            static const std::string startFEN; // default piece position

            // set up game from position
            void from_pgn(const std::string& pgn);
            void from_fen(const std::string& fen);

            // getters
            const board_type& get_board() const;
            const Square& get_promotion_pawn() const;
            uint8_t get_state() const;
            uint8_t get_move_order() const;
            uint32_t get_move_no() const;
            Square is_king_checked() const;

            std::string pgn() const;
            std::string fen() const;

            std::string pop_last_move();

            void move(const Square& from, const Square& to, uint8_t promotion); // tries to move piece
            const std::list<Square> get_possible_moves(const Square& pos);
            void move_back(); // moves back from position
            void move_forward(); // moves forward from position
        private:
            Board board;
            uint8_t state = 0; // state of the game, left 3 bits represent result, 4 right bits reprsent termination

            uint8_t move_order; // who moves now
            uint32_t move_no; // move number

            std::string player_white; // white player name
            std::string player_black; // black player name
 
            uint32_t time_white; // time left by white
            uint32_t time_black; // time left by black

            std::list<std::string>::iterator current_pos; // iterator pointing at current position in FEN list
            std::list<std::string> FEN; // list of all positions that was reached during the game
            std::string PGN; // pgn of the current game
            std::string last_move; // last made move in acn that will be popped by game viewer

            void from_short_fen(const std::string& short_fen); // translates fen to position(without extra game info like castles etc.)
            void write_fen(); // writes short fen of the position(no extra info like castles, move number and etc.)
            void write_pgn(const Square& from, const Square& to); // writes pgn(and due to technical nuances makes a move);

            void end(); // tries to end the game
    };
}
#endif
