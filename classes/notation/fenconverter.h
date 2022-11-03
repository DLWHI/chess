#ifndef FEN_H_
#define FEN_H_
#include "../../chess.h"

namespace chess
{
    class FENConverter
    {
        public:
            FENConverter(const std::string& data = "");

            uint8_t from(uint8_t board[Board::x_size][Board::y_size], std::string fen = "");
            std::string to(const Game& game);
            std::string to(const std::string& board);

            std::string get_input() const;
            uint8_t get_move() const;
            uint16_t get_move_no() const;
            std::string get_castles() const;
            Square get_en_passant() const;
        private:
            std::string data;

            uint8_t move;
            uint16_t move_no;
            std::string castles;
            Square en_passant;

            uint8_t board[Board::x_size][Board::y_size];

            virtual char acn_to_piece(char c) const;
    };
}
#endif
