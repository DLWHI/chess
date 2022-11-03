#include "fenconverter.h"

using namespace chess;

uint8_t FENConverter::from(uint8_t board[Board::x_size][Board::y_size], std::string fen)
{
    if (fen == "")
        fen = data;
    uint8_t x = 0, y = 0;
    auto it = fen.begin();
    for (;*it != ' '; it++)
    {
        if ('0' < *it && *it < '9')
            for (char i = 0; i + 48 < *it; i++)
                board[x++%8][y] = C_SQUARE;
        else if (*it == '/')
            y = (++y)%Board::y_size;
        else if (*it)
            board[x++][y] = char_to_piece(*it);
        else
            return -1;
        x = x%Board::x_size;
    }
    it++;
    if (*it == 'w')
        move = C_WHITE;
    else if (*it == 'b')
        move = C_BLACK;
    data = fen;
    return 0;
}
std::string FENConverter::to(const Game& game)
{
    std::string fen;
    return fen;
}
std::string to(const std::string& board)
{
    return "";
}
