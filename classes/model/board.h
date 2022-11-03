#ifndef BOARD_H_
#define BOARD_H_
#include "../../chess.h"
#include <list>
#include <stack>
#include <string>

namespace chess 
{
    enum KingState
    {
        NoCheck = 0,
        WhiteChecked = 1,
        WhiteMated = 2,
        BlackChecked = -1,
        BlackMated = -2
    };
    struct Square
    {
        int8_t x;
        int8_t y;

        Square() = default;
        Square(int8_t x, int8_t y);

        void operator+=(const Square& other);
    };

    typedef uint8_t board_type[8][8];

    class Board 
    {
        public:
            Board() = default;
            Board(const Board& other);
            ~Board() = default;

            static const uint8_t x_size = 8;
            static const uint8_t y_size = 8;
            
            void move_piece(const Square& from, const Square& to); // moves piece
            std::list<Square> gen_moves(uint8_t x, uint8_t y) const; // generates all legal moves for piece on given square

            bool check_board(const Square& king, uint8_t color) const; // checks king for checks, returns true if king checked
            bool check_castles(uint8_t color, uint8_t side) const; // checks for castling avaliability, returns false if one can castle
            void check_king(uint8_t color); // updates king_checked
            void check_mate(uint8_t color); // updates king_checked for mate
        protected:
            friend class Game;

            board_type pieces;
            KingState king_checked = KingState::NoCheck; // whether any of the kings is checked
            Square bking_pos; // kings' position, required in order not to look for them when needed
            Square wking_pos;
            Square en_passant_target = Square(-1, -1); // square which can be en passanted, if none exists then ept.x == -1
            Square promotion_target = Square(-1, -1);
            std::string castles; // castling avaliability as in fen notations

            bool last_takes = false; // whether last move was takes or not
            uint16_t halfmove_clock = 0; // moves without taking or pawn advance
            std::stack<uint8_t> dead_pieces; // pieces taken;

            void move_king(const Square& from, const Square& to); // all king moving encapsulation(updating pos and castling if needed)
            void move_pawn(const Square& from, const Square& to); // pawn move encapsulation(promotion + en passant)
            void promote(uint8_t target);
            void castle(uint8_t color, uint8_t side); // side = 1 for queenside, 0 for king

            void find_kings(); // look for king, required in setting up game from position
            bool is_opposite_color(int8_t piece1, int8_t piece2) const; // checks if piceces on square is different color, true if one of the arguments is 0(empty square)

            // move generators
            std::list<Square> gen_pawn_moves(uint8_t x, uint8_t y) const;
            std::list<Square> gen_knight_moves(uint8_t x, uint8_t y) const;
            std::list<Square> gen_bishop_moves(uint8_t x, uint8_t y) const;
            std::list<Square> gen_rook_moves(uint8_t x, uint8_t y) const;
            std::list<Square> gen_queen_moves(uint8_t x, uint8_t y) const;
            std::list<Square> gen_king_moves(uint8_t x, uint8_t y, bool gen_castles) const;
            std::list<Square> gen_king_castles(uint8_t x, uint8_t y) const;

            // move filtering
            void filter_legal(std::list<Square>& moves, const Square& pos);
            void filter_castles(std::list<Square>& moves, const Square& pos) const;

            // acn converters
            std::string to_acn(const Square& from, const Square& to); // acn = algebraic chess notation
            Square from_acn_end(const std::string& acn) const;
            Square from_acn_start(const std::string& acn, const Square& end, uint8_t color) const;

            // piece finders, required for acn converters
            // like move generators but more restrcitive
            std::list<Square> find_piece_by_moves(uint8_t piece, uint8_t x, uint8_t y) const;
            std::list<Square> find_pawn_by_moves(uint8_t x, uint8_t y, uint8_t color) const;
            std::list<Square> find_knight_by_moves(uint8_t x, uint8_t y, uint8_t color) const;
            std::list<Square> find_bishop_by_moves(uint8_t x, uint8_t y, uint8_t color) const;
            std::list<Square> find_rook_by_moves(uint8_t x, uint8_t y, uint8_t color) const;
            std::list<Square> find_queen_by_moves(uint8_t x, uint8_t y, uint8_t color) const;
            std::list<Square> find_king_by_moves(uint8_t x, uint8_t y, uint8_t color) const;

            // char <-> code conversion
            uint8_t acn_to_piece(char c) const;
            uint8_t fen_to_piece(char c) const;

            char piece_to_acn(uint8_t piece) const;
            char piece_to_fen(uint8_t piece) const;
    };
}
#endif
