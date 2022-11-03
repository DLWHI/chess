#ifndef CONVERTER_H_
#define CONVERTER_H_

#include <string>
#include "../model/game.h"

namespace chess
{
    class Converter
    {
        public:
            Converter(const std::string& data = "");
            virtual ~Converter() = default;
            
            virtual uint8_t from(uint8_t board[Board::x_size][Board::y_size], std::string notation = "") = 0;
            virtual std::string to(const Game& game) = 0;

            std::string get_input() const;
            uint8_t get_move() const;
            uint16_t get_move_no() const;
            std::string get_castles() const;
            Square get_en_passant() const;
        protected:
            std::string data;

            uint8_t move;
            uint16_t move_no;
            std::string castles;
            Square en_passant;

            uint8_t board[Board::x_size][Board::y_size];

            virtual char char_to_piece(char c) const;
    };
}
#endif
