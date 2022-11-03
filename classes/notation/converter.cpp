#include "converter.h"

using namespace chess;
Converter::Converter(const std::string& data)
{
    this->data = data;
    std::string fen_short = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
    uint8_t x = 0, y = 0;
    auto it = fen_short.begin();
    for (;*it; it++)
    {
        if ('0' < *it && *it < '9')
            for (char i = 0; i + 48 < *it; i++)
                board[x++%8][y] = C_SQUARE;
        else if (*it == '/')
            y = (++y)%Board::y_size;
        else if (*it)
            board[x++][y] = char_to_piece(*it);
        x = x%Board::x_size;
    }
}

char Converter::char_to_piece(char c) const
{
    char piece = 0;
    if ('A' <= c && c <= 'Z')
    {
        piece = C_WHITE;
        c += 32;
    }
    if (c == 'p')
        piece |= C_PAWN;
    else if (c == 'n')
        piece |= C_KNIGHT;
    else if (c == 'b')
        piece |= C_BISHOP;
    else if (c == 'r')
        piece |= C_ROOK;
    else if (c == 'q')
        piece |= C_QUEEN;
    else if (c == 'k')
        piece |= C_KING;
    return piece;
}

std::string Converter::get_input() const
{
    return data;
}
uint8_t Converter::get_move() const
{
    return move;
}
uint16_t Converter::get_move_no() const
{
    return move_no;
}
std::string Converter::get_castles() const
{
    return castles;
}
Square Converter::get_en_passant() const
{
    return en_passant;
}
