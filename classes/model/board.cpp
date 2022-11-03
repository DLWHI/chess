#include "board.h"
#include <functional>
#include <iostream>
using namespace chess;
//----------SQUARE-STRUCT----------
Square::Square(int8_t x, int8_t y)
{
    this->x = x;
    this->y = y;
}

void Square::operator+=(const Square& other)
{
    x += other.x;
    y += other.y;
}
//----------constructors----------
Board::Board(const Board& other)
{
    for (uint8_t i = 0; i < x_size; i++)
        for (uint8_t j = 0; j < y_size; j++)
            pieces[i][j] = other.pieces[i][j];
    bking_pos = other.bking_pos;
    wking_pos = other.wking_pos;
    king_checked = other.king_checked;
    castles = other.castles;
}
//----------functionality----------
void Board::move_piece(const Square& from, const Square& to)
{
    halfmove_clock++;
    if (pieces[to.x][to.y])
    {
        dead_pieces.push(pieces[to.x][from.y]);
        last_takes = true;
        halfmove_clock = 0;
    }
    else
        last_takes = false;
    pieces[to.x][to.y] = pieces[from.x][from.y];
    pieces[from.x][from.y] = C_SQUARE;
    en_passant_target.x = -1;
    if (pieces[to.x][to.y] & C_KING)
        move_king(from, to);
    else if (pieces[to.x][to.y] & C_PAWN)
        move_pawn(from, to);
}

void Board::move_king(const Square& from, const Square& to)
{
    uint8_t color = pieces[to.x][to.y] & C_WHITE;
    Square castle_target = Square(0, (color) ? 7 : 0);
    Square& king = (color) ? wking_pos : bking_pos;
    if(from.x == 4 && from.y == castle_target.y && to.x == 6)
        castle(color, 0);
    else if (from.x == 4 && from.y == castle_target.y && to.x == 2)
        castle(color, 1);
    king = to;
    if (castles.find('k' - color/C_WHITE*32) + 1)
        castles.erase(castles.find('k' - color/C_WHITE*32), 1);
    if (castles.find('q' - color/C_WHITE*32) + 1)
        castles.erase(castles.find('q' - color/C_WHITE*32), 1);
}
void Board::move_pawn(const Square& from, const Square& to)
{
    if (abs(from.y - to.y) == 2)
        en_passant_target = Square(to.x, to.y + (from.y - to.y)/2);
    else if (to.y == 0 || to.y == 7)
    {
        pieces[from.x][from.y] = pieces[to.x][to.y];
        pieces[to.x][to.y] = C_SQUARE;
        promotion_target = from;
    }
    else if (to.x == en_passant_target.x && to.y == en_passant_target.y)
    {
        dead_pieces.push(pieces[to.x][from.y]);
        pieces[to.x][from.y] = C_SQUARE;
        last_takes = true;
    }
    halfmove_clock = 0;
}

void Board::promote(uint8_t target)
{
    if (target & C_PIECE_MASK)
    {
        pieces[promotion_target.x][promotion_target.y] = C_SQUARE;
        promotion_target.y = (target & C_WHITE) ? 0 : 7; 
        pieces[promotion_target.x][promotion_target.y] = target;
    }
    promotion_target.x = -1;
}
void Board::castle(uint8_t color, uint8_t side)
{
    Square rook_pos = Square((side) ? 0 : 7, (color) ? 7 : 0);
    Square rook_end = Square((side) ? 3 : 5, (color) ? 7 : 0);
    move_piece(rook_pos, rook_end);
    castles.erase(castles.find('k' - color/C_WHITE*32), 1);
    castles.erase(castles.find('q' - color/C_WHITE*32), 1);
}
//----------checks----------
bool Board::is_opposite_color(int8_t piece1, int8_t piece2) const
{
    if (piece1 && piece2)
        return (piece1 & C_WHITE) ^ (piece2 & C_WHITE);
    return true;
}

bool Board::check_board(const Square& king, uint8_t color) const
{
    std::function<std::list<Square>(uint8_t, uint8_t)> finders[] = 
    {
        std::bind(find_knight_by_moves, this, std::placeholders::_1, std::placeholders::_2, color),
        std::bind(find_bishop_by_moves, this, std::placeholders::_1, std::placeholders::_2, C_BISHOP | color),
        std::bind(find_rook_by_moves, this, std::placeholders::_1, std::placeholders::_2, C_ROOK | color),
        std::bind(find_queen_by_moves, this, std::placeholders::_1, std::placeholders::_2, color),
        std::bind(find_king_by_moves, this, std::placeholders::_1, std::placeholders::_2, color)
    };
    uint8_t types[] = 
    {
        C_KNIGHT,
        C_BISHOP,
        C_ROOK,
        C_QUEEN,
        C_KING
    };
    std::list<Square> moves;
    for (uint8_t i = 0; i < 5; i++)
    {
        moves = finders[i](king.x, king.y);
        for (auto j = moves.begin(); j != moves.end(); j++)
            if ((pieces[j->x][j->y] & C_WHITE) == color && (pieces[j->x][j->y] & types[i]))
                return true;
    }
    
    // if (color)
    //     return true;
    // else if (!color)
    //     return true;

    return false;
}
bool Board::check_castles(uint8_t color, uint8_t side) const
{
    if ((side && castles.find('q' - color/C_WHITE*32) == std::string::npos)
        || (castles.find('k' - color/C_WHITE*32) == std::string::npos) || king_checked)
        return true;
    Square rook_pos = Square((side) ? 0 : 7, (color) ? 7 : 0);
    Square king_pos = (color) ? wking_pos : bking_pos;
    Square king_end = Square((side) ? 2 : 6, (color) ? 7 : 0);
    Square rook_end = Square((side) ? 3 : 5, (color) ? 7 : 0);
    side = (side) ? -1 : 1;
    while (king_pos.x != king_end.x)
    {
        king_pos += Square(side, 0);
        if (pieces[king_pos.x][king_pos.y] || check_board(king_pos, color ^ C_WHITE))
            return true;
    }
    while (rook_pos.x != rook_end.x)
    {
        rook_pos += Square(-side, 0);
        if ((pieces[rook_pos.x][rook_pos.y] & color == color) && (pieces[rook_pos.x][rook_pos.y] & C_KING))
            continue;
        else if (pieces[rook_pos.x][rook_pos.y])
            return true;
    }
    return false;
}
void Board::check_king(uint8_t color)
{
    Square king = (color) ? wking_pos : bking_pos;
    if (check_board(king, color ^ C_WHITE))
        king_checked = (color) ? KingState::WhiteChecked : KingState::BlackChecked;
    else
        king_checked = KingState::NoCheck;
    if (king_checked)
        check_mate(color);
}
void Board::check_mate(uint8_t color)
{
    for (uint8_t i = 0; i < x_size; i++)
        for (uint8_t j = 0; j < y_size; j++)
            if ((pieces[j][i] & C_WHITE) == color)
            {
                std::list<Square> moves = gen_moves(j, i);
                filter_legal(moves, Square(j, i));
                if (moves.size())
                    return;
            }
    king_checked = (color) ? KingState::WhiteMated : KingState::BlackMated;
}
//----------move-gen----------
std::list<Square> Board::gen_moves(uint8_t x, uint8_t y) const
{
    if (0 > x || x > x_size || 0 > y || y > y_size)
        return std::list<Square>();
    if (pieces[x][y] & C_PAWN)
        return gen_pawn_moves(x, y);
    else if (pieces[x][y] & C_KNIGHT)
        return gen_knight_moves(x, y);
    else if (pieces[x][y] & C_BISHOP)
        return gen_bishop_moves(x, y);
    else if (pieces[x][y] & C_ROOK)
        return gen_rook_moves(x, y);
    else if (pieces[x][y] & C_QUEEN)
        return gen_queen_moves(x, y);
    else if (pieces[x][y] & C_KING)
        return gen_king_moves(x, y, true);
    return std::list<Square>();
}
std::list<Square> Board::find_piece_by_moves(uint8_t piece, uint8_t x, uint8_t y) const
{
    if (0 > x || x > Board::x_size || 0 > y || y > Board::y_size)
        return std::list<Square>();
    if (piece & C_PAWN)
        return find_pawn_by_moves(x, y, piece & C_WHITE);
    else if (piece & C_KNIGHT)
        return find_knight_by_moves(x, y, piece & C_WHITE);
    else if (piece & C_BISHOP)
        return find_bishop_by_moves(x, y, piece);
    else if (piece & C_ROOK)
        return find_rook_by_moves(x, y, piece);
    else if (piece & C_QUEEN)
        return find_queen_by_moves(x, y, piece & C_WHITE);
    else if (piece & C_KING)
        return find_king_by_moves(x, y, piece & C_WHITE);
    return std::list<Square>();
}

std::list<Square> Board::gen_pawn_moves(uint8_t x, uint8_t y) const
{
    std::list<Square> generated;
    int8_t dir = 1;
    if (pieces[x][y] & C_WHITE)
        dir = -1;
    if (x + 1 < x_size && pieces[x + 1][y + dir] && is_opposite_color(pieces[x + 1][y + dir], pieces[x][y]))
        generated.push_back(Square(x + 1, y + dir));
    if (x - 1 >= 0 && pieces[x - 1][y + dir] && is_opposite_color(pieces[x - 1][y + dir], pieces[x][y]))
        generated.push_back(Square(x - 1, y + dir));
    if (!pieces[x][y + dir])
        generated.push_back(Square(x, y + dir));
    if (((y == 1 && dir == 1) || (y == 6 && dir == -1)) && !pieces[x][y + 2*dir] && !pieces[x][y + dir])
        generated.push_back(Square(x, y + 2*dir));
    if (en_passant_target.x != -1 && abs(x - en_passant_target.x) == 1 && abs(y - en_passant_target.y) == 1)
        generated.push_back(en_passant_target);
    return generated;
}
std::list<Square> Board::gen_knight_moves(uint8_t x, uint8_t y) const
{
    std::list<Square> generated;
    Square Nmove = {1, 2};
    int8_t dir = 1;
    for (uint8_t i = 0; i < 8; i++, dir *= -1)
    {
        if (0 <= Nmove.x + x && 0 <= Nmove.y*dir + y
            && Nmove.x + x < x_size && Nmove.y*dir + y < y_size
            && is_opposite_color(pieces[x][y], pieces[Nmove.x + x][Nmove.y*dir + y]))
            generated.push_back(Square(Nmove.x + x, Nmove.y*dir + y));
        if (i%2)
            Nmove = {Nmove.y, -Nmove.x};
    }
    return generated;
}
std::list<Square> Board::gen_bishop_moves(uint8_t x, uint8_t y) const
{
    std::list<Square> generated;int8_t l = x - 1, r = x + 1, ud = y - 1, dir = -1;
    for (int8_t i = 0; i < 2; i++, ud = y + 1, dir = 1, l = x - 1, r = x + 1)
        for (; 0 <= ud && ud < Board::y_size && (0 <= l || r < 8); ud += dir)
        {
            if (l >= 0)
            {
                if (is_opposite_color(pieces[x][y], pieces[l][ud]))
                    generated.push_back(Square(l, ud));
                if (pieces[l--][ud])
                    l = -1;
            }
            if (r < 8)
            {
                if (is_opposite_color(pieces[x][y], pieces[r][ud]))
                    generated.push_back(Square(r, ud));
                if (pieces[r++][ud])
                    r = 9;
            }
        }
    return generated;
}
std::list<Square> Board::gen_rook_moves(uint8_t x, uint8_t y) const
{
    std::list<Square> generated;
    int8_t l = x - 1, r = x + 1, u = y - 1, d = y + 1;
    for (; 0 <= u || 0 <= l || r < 8 || d < 8; l--, u--, r++, d++)
    {
        if (l >= 0)
        {
            if (is_opposite_color(pieces[x][y], pieces[l][y]))
                generated.push_back(Square(l, y));
            if (pieces[l][y])
                l = -1;
        }
        if (r < 8)
        {
            if (is_opposite_color(pieces[x][y], pieces[r][y]))
                generated.push_back(Square(r, y));
            if (pieces[r][y])
                r = 9;
        }
        if (u >= 0)
        {
            if (is_opposite_color(pieces[x][y], pieces[x][u]))
                generated.push_back(Square(x, u));
            if (pieces[x][u])
                u = -1;
        }
        if (d < 8)
        {
            if (is_opposite_color(pieces[x][y], pieces[x][d]))
                generated.push_back(Square(x, d));
            if (pieces[x][d])
                d = 9;
        }
    }
    return generated;
}
std::list<Square> Board::gen_queen_moves(uint8_t x, uint8_t y) const
{
    std::list<Square> qbishop = gen_bishop_moves(x, y);
    std::list<Square> qrook = gen_rook_moves(x, y);
    qbishop.splice(qbishop.end(), qrook);
    return qbishop;
}
std::list<Square> Board::gen_king_moves(uint8_t x, uint8_t y, bool gen_castles) const
{
    std::list<Square> generated;
    Square Kmove = {0, 1};
    for (uint8_t j = 0; j < 2; j++, Kmove = {1, 1})
        for (uint8_t i = 0; i < 4; i++)
        {
            if (0 <= Kmove.x + x && 0 <= Kmove.y + y
                && Kmove.x + x < x_size && Kmove.y + y < y_size
                && is_opposite_color(pieces[x][y], pieces[x + Kmove.x][y + Kmove.y]))
                generated.push_back(Square(x + Kmove.x, y + Kmove.y));
            Kmove = {Kmove.y, -Kmove.x};
        }
    if (gen_castles)
        generated.splice(generated.end(), gen_king_castles(x, y));
    return generated;
}
std::list<Square> Board::gen_king_castles(uint8_t x, uint8_t y) const
{
    std::list<Square> generated;
    uint8_t color = pieces[x][y] & C_WHITE;
    if (castles.find('k' - color/C_WHITE*32) + 1)
        generated.push_back(Square(6, (color) ? 7 : 0));
    if (castles.find('q' - color/C_WHITE*32) + 1)
        generated.push_back(Square(2, (color) ? 7 : 0));
    return generated;
}

std::list<Square> Board::find_pawn_by_moves(uint8_t x, uint8_t y, uint8_t color) const
{
    std::list<Square> generated;
    int8_t dir = -1;
    if (color)
        dir = 1;
    if ((pieces[x][y + dir] & (C_PAWN | C_WHITE)) == (C_PAWN | color))
        generated.push_back(Square(x, y + dir));
    else if (((y == 4 && dir == 1) || (y == 3 && dir == -1)) 
        && (pieces[x][y + 2*dir] & (C_PAWN | C_WHITE)) == (C_PAWN | color) && !pieces[x][y + dir])
        generated.push_back(Square(x, y + 2*dir));
    else if ((pieces[x][y] & C_WHITE) == (color ^ C_WHITE) || (en_passant_target.x == x && en_passant_target.y))
    {
        if (x + 1 < Board::x_size && (pieces[x + 1][y + dir] & (C_PAWN | C_WHITE)) == (C_PAWN | color))
            generated.push_back(Square(x + 1, y + dir));
        if (x - 1 >= 0 && (pieces[x - 1][y + dir] & (C_PAWN | C_WHITE)) == (C_PAWN | color))
            generated.push_back(Square(x - 1, y + dir));
    }
    return generated;
}
std::list<Square> Board::find_knight_by_moves(uint8_t x, uint8_t y, uint8_t color) const
{
    std::list<Square> generated;
    Square Nmove = {1, 2};
    int8_t dir = 1;
    for (uint8_t i = 0; i < 8; i++, dir *= -1)
    {
        if (0 <= Nmove.x + x && 0 <= Nmove.y*dir + y
            && Nmove.x + x < Board::x_size && Nmove.y*dir + y < Board::y_size
            && (pieces[Nmove.x + x][Nmove.y*dir + y] & (C_KNIGHT | C_WHITE)) == (C_KNIGHT | color))
            generated.push_back(Square(Nmove.x + x, Nmove.y*dir + y));
        if (i%2)
            Nmove = {Nmove.y, -Nmove.x};
    }
    return generated;
}
std::list<Square> Board::find_bishop_by_moves(uint8_t x, uint8_t y, uint8_t color) const
{
    std::list<Square> generated;
    int8_t l = x - 1, r = x + 1, ud = y - 1, dir = -1;
    for (int8_t i = 0; i < 2; i++, ud = y + 1, dir = 1, l = x - 1, r = x + 1)
        for (; 0 <= ud && ud < Board::y_size && (0 <= l || r < 8); ud += dir)
        {
            if (l >= 0)
            {
                if ((pieces[l][ud] & (C_BISHOP | C_QUEEN | C_WHITE)) == color)
                    generated.push_back(Square(l, ud));
                if (pieces[l--][ud])
                    l = -1;
            }
            if (r < 8)
            {
                if ((pieces[r][ud] & (C_BISHOP | C_QUEEN | C_WHITE)) == color)
                    generated.push_back(Square(r, ud));
                if (pieces[r++][ud])
                    r = 9;
            }
        }
    return generated;
}
std::list<Square> Board::find_rook_by_moves(uint8_t x, uint8_t y, uint8_t color) const
{
    std::list<Square> generated;
    int8_t l = x - 1, r = x + 1, u = y - 1, d = y + 1;
    for (; 0 <= u || 0 <= l || r < 8 || d < 8; l--, u--, r++, d++)
    {
        if (l >= 0)
        {
            if ((pieces[l][y] & (C_ROOK | C_QUEEN | C_WHITE)) == color)
                generated.push_back(Square(l, y));
            if (pieces[l][y])
                l = -1;
        }
        if (r < 8)
        {
            if ((pieces[r][y] & (C_ROOK | C_QUEEN | C_WHITE)) == color)
                generated.push_back(Square(r, y));
            if (pieces[r][y])
                r = 9;
        }
        if (u >= 0)
        {
            if ((pieces[x][u] & (C_ROOK | C_QUEEN | C_WHITE)) == color)
                generated.push_back(Square(x, u));
            if (pieces[x][u])
                u = -1;
        }
        if (d < 8)
        {
            if ((pieces[x][d] & (C_ROOK | C_QUEEN | C_WHITE)) == color)
                generated.push_back(Square(x, d));
            if (pieces[x][d])
                d = 9;
        }
    }
    return generated;
}
std::list<Square> Board::find_queen_by_moves(uint8_t x, uint8_t y, uint8_t color) const
{
    std::list<Square> qbishop = find_bishop_by_moves(x, y, color | C_QUEEN);
    std::list<Square> qrook = find_rook_by_moves(x, y, color | C_QUEEN);
    qbishop.splice(qbishop.end(), qrook);
    return qbishop;
}
std::list<Square> Board::find_king_by_moves(uint8_t x, uint8_t y, uint8_t color) const
{
    std::list<Square> generated;
    Square Kmove = {0, 1};
    for (uint8_t j = 0; j < 2; j++, Kmove = {1, 1})
        for (uint8_t i = 0; i < 4; i++)
        {
            if (0 <= Kmove.x + x && 0 <= Kmove.y + y
                && Kmove.x + x < Board::x_size && Kmove.y + y < Board::y_size
                && (pieces[Kmove.x + x][Kmove.y + y] & (C_KING | C_WHITE)) == (C_KING | color))
                generated.push_back(Square(x + Kmove.x, y + Kmove.y));
            Kmove = {Kmove.y, -Kmove.x};
        }
    return generated;
}

void Board::filter_legal(std::list<Square>& moves, const Square& pos) 
{
    uint8_t color = pieces[pos.x][pos.y] & C_WHITE;
    uint8_t piece_buffer;
    bool del;
    if (pieces[pos.x][pos.y] & C_KING)
        filter_castles(moves, pos);
    for (auto i = moves.begin(); i != moves.end(); i++)
    {
        Square king = (color) ? wking_pos : bking_pos;
        if (pieces[pos.x][pos.y] & C_KING)
            king = *i;
        piece_buffer = pieces[i->x][i->y];
        pieces[i->x][i->y] = pieces[pos.x][pos.y];
        pieces[pos.x][pos.y] = C_SQUARE;
        del = check_board(king, C_WHITE ^ color);
        pieces[pos.x][pos.y] = pieces[i->x][i->y];
        pieces[i->x][i->y] = piece_buffer;
        if (del)
            moves.erase(i--);
    }
}
void Board::filter_castles(std::list<Square>& moves, const Square& pos) const
{
    for (auto i = moves.begin(); i != moves.end() && pos.x == 4; i++)
    {
        if ((i->x == 6 && i->y == 0 && check_castles(C_BLACK, 0)) ||
            (i->x == 2 && i->y == 0 && check_castles(C_BLACK, 1)) ||
            (i->x == 6 && i->y == 7 && check_castles(C_WHITE, 0)) ||
            (i->x == 2 && i->y == 7 && check_castles(C_WHITE, 1)))
            moves.erase(i--);
    }
}
//----------translators----------
void Board::find_kings()
{
    for (uint8_t i = 0; i < Board::y_size; i++)
        for (uint8_t j = 0; j < Board::x_size; j++)
            if (pieces[j][i] == C_KING)
                bking_pos = {j, i};
            else if (pieces[j][i] == (C_KING | C_WHITE))
                wking_pos = {j, i};
}

std::string Board::to_acn(const Square& from, const Square& to)
{
    std::string ret;
    if (piece_to_acn(pieces[to.x][to.y]))
        ret += piece_to_acn(pieces[to.x][to.y]);
    std::list<Square> moves = find_piece_by_moves(pieces[to.x][to.y], to.x, to.y);
    if (moves.size())
    {
        if (from.x == moves.front().x && from.y != moves.front().y)
            ret += (56 - from.y);
        else
            ret += (from.x + 'a');
    }
    if (last_takes)
        ret += 'x';
    ret += (to.x + 'a');
    ret += (56 - to.y);
    return ret;
}
Square Board::from_acn_end(const std::string& acn) const
{
    Square end;
    auto i = acn.rbegin();
    while ('0' >= *i || *i >= '9')
        i++;
    end.y = 56 - *i++;
    end.x = *i - 'a';
    return end;
}
Square Board::from_acn_start(const std::string& acn, const Square& end, uint8_t color) const
{
    Square start = {-1, -1};
    auto i = acn.begin();
    uint8_t piece = acn_to_piece(*i);
    if (piece != C_PAWN)
        *i++;
    piece |= color;
    std::list<Square> moves = find_piece_by_moves(piece, end.x, end.y);
    if (moves.size() > 1)
    {
        if ('0' < *i && *i < '9')
            start.y = 56 - *i;
        else
            start.x = *i - 'a';
        for (auto j = moves.begin(); j != moves.end() && (start.x == -1 || start.y == -1); j++)
            if (j->x == start.x || j->y == start.y)
                start = {j->x, j->y};
    }
    else
    {
        start.x = moves.begin()->x;
        start.y = moves.begin()->y;
    }
    return start;
}

uint8_t Board::acn_to_piece(char c) const
{
    if (c == 'N')
        return C_KNIGHT;
    else if (c == 'B')
        return C_BISHOP;
    else if (c == 'R')
        return C_ROOK;
    else if (c == 'Q')
        return C_QUEEN;
    else if (c == 'K')
        return C_KING;
    return C_PAWN;
}
uint8_t Board::fen_to_piece(char c) const
{
    uint8_t piece = 0;
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
char Board::piece_to_acn(uint8_t piece) const
{
    if (piece & C_KNIGHT)
        return 'N';
    else if (piece & C_BISHOP)
        return 'B';
    else if (piece & C_ROOK)
        return 'R';
    else if (piece & C_QUEEN)
        return 'Q';
    else if (piece & C_KING)
        return 'K';
    else if (piece & C_PAWN)
        return 0;
    return -1;
}
char Board::piece_to_fen(uint8_t piece) const
{
    if (piece & C_KNIGHT)
        return 'n' - (piece & C_WHITE)/C_WHITE*32;
    else if (piece & C_BISHOP)
        return 'b' - (piece & C_WHITE)/C_WHITE*32;
    else if (piece & C_ROOK)
        return 'r' - (piece & C_WHITE)/C_WHITE*32;
    else if (piece & C_QUEEN)
        return 'q' - (piece & C_WHITE)/C_WHITE*32;
    else if (piece & C_KING)
        return 'k' - (piece & C_WHITE)/C_WHITE*32;
    else if (piece & C_PAWN)
        return 'p' - (piece & C_WHITE)/C_WHITE*32;
    return 0;
}
