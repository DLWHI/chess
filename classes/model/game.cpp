#include "game.h"
#include <functional>
#include <regex>
#include <iostream>
using namespace chess;

const std::string Game::startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

//----------constructors----------
Game::Game()
{
    from_fen(startFEN);
    FEN.push_back(std::string(startFEN.begin(), startFEN.begin() + startFEN.find(' ')));
    current_pos = FEN.begin();
}
Game::Game(const std::string& data, bool pgn)
{
    if (pgn)
    {
        from_pgn(data);
        write_fen();
    }
    else
    {
        from_fen(data);
        FEN.push_back(data);
    }
    current_pos = FEN.begin();
    end();
}
// Game::Game(Converter& cnv)
// {
//     cnv.from(board.pieces);
//     move_order = cnv.get_move();
//     move_no = cnv.get_move_no();
//     board.castles = cnv.get_castles();
//     board.en_passant_target = cnv.get_en_passant();
//     board.find_kings();
// }
Game::Game(const Game& other) : board(other.board)
{
    move_order = other.move_order;
}

void Game::from_pgn(const std::string& pgn)
{
    from_fen(startFEN);
    PGN = pgn;
    if (PGN.back() != ' ')
        PGN += ' ';
    std::regex move_reg("[NBRQK]?[a-h1-8]?x?(?:[a-h][1-8]|O-O-O|O-O)[\\+#]?", std::regex::ECMAScript);
    Square start, end;
    for(auto i = std::sregex_iterator(PGN.begin(), PGN.end(), move_reg);
                            i != std::sregex_iterator();
                            i++, move_order ^= C_WHITE, move_no++)
    {
        if (i->str() == "O-O-O")
        {
            end = {2, (move_order) ? 7 : 0};
            start = {4, (move_order) ? 7 : 0};
        }
        else if (i->str() == "O-O")
        {
            end = {6, (move_order) ? 7 : 0};
            start = {4, (move_order) ? 7 : 0};
        }
        else
        {
            end = board.from_acn_end(i->str());
            start = board.from_acn_start(i->str(), end, move_order);
        }
        board.move_piece(start, end);
        if (i->str().find('+') + 1)
            board.king_checked = (move_order) ? KingState::BlackChecked : KingState::WhiteChecked;
        else if (i->str().find('#') + 1)
            board.king_checked = (move_order) ?  KingState::BlackMated : KingState::WhiteMated;
    }
    move_no /= 2;
}
void Game::from_fen(const std::string& fen)
{
    uint8_t x = 0, y = 0;
    size_t pos = 0;
    for (; fen[pos] != ' '; pos++)
    {
        if ('0' < fen[pos] && fen[pos] < '9')
            for (char i = 0; i + 48 < fen[pos]; i++)
                board.pieces[x++%8][y] = C_SQUARE;
        else if (fen[pos] == '/')
            y = (++y)%Board::y_size;
        else if (fen[pos])
            board.pieces[x++][y] = board.fen_to_piece(fen[pos]);
        else
            throw std::runtime_error("Invalid FEN");
        x = x%Board::x_size;
    }
    pos++;
    if (fen[pos] == 'w')
        move_order = C_WHITE;
    else if (fen[pos] == 'b')
        move_order = C_BLACK;
    else
        throw std::runtime_error("Invalid FEN");
    pos += 2;
    for (; fen[pos] != ' '; pos++)
        if (fen[pos] != '-')
            board.castles += fen[pos];
    pos++;
    if (fen[pos] != '-')
    {
        board.en_passant_target.y = 56 - fen[pos++]; // 8 - *i - 48(char '0') + 1
        board.en_passant_target.x = fen[pos] - 'a';
    }
    pos += 2;
    board.halfmove_clock = std::stoi(&fen[pos]);
    pos = fen.find(' ', pos);
    move_no = std::stoi(&fen[++pos]);
    board.find_kings();
    board.check_king(move_order);
}
void Game::from_short_fen(const std::string& short_fen)
{
    uint8_t x = 0, y = 0;
    size_t pos = 0;
    for (; short_fen[pos]; pos++)
    {
        if ('0' < short_fen[pos] && short_fen[pos] < '9')
            for (char i = 0; i + 48 < short_fen[pos]; i++)
                board.pieces[x++%8][y] = C_SQUARE;
        else if (short_fen[pos] == '/')
            y = (++y)%Board::y_size;
        else if (short_fen[pos])
            board.pieces[x++][y] = board.fen_to_piece(short_fen[pos]);
        x = x%Board::x_size;
    }
    board.find_kings();
}
//----------getters----------
const board_type& Game::get_board() const
{
    return board.pieces;
}
const Square& Game::get_promotion_pawn() const
{
    return board.promotion_target;
}
uint8_t Game::get_state() const
{
    return state;
}
uint8_t Game::get_move_order() const
{
    return move_order;
}
uint32_t Game::get_move_no() const
{
    return move_no;
}
Square Game::is_king_checked() const
{
    if (board.king_checked)
        if (board.king_checked == 1 || board.king_checked == 2)
            return board.wking_pos;
        else if (board.king_checked == -1 || board.king_checked == -2)
            return board.bking_pos;
    return Square(-1, -1);
}

std::string Game::pgn() const
{
    return PGN;
}
std::string Game::fen() const
{
    return *current_pos;
}

std::string Game::pop_last_move()
{
    std::string pc = last_move;
    last_move = "";
    return pc;
}
//----------moves----------
void Game::move(const Square& from, const Square& to, uint8_t promotion)
{
    if (promotion)
        if (promotion != 255)
            board.promote(promotion | move_order);
        else
            return board.promote(0);
    else
        board.move_piece(from, to);
    if (board.promotion_target.x == -1)
    {
        board.check_king(move_order ^ C_WHITE);
        move_order ^= C_WHITE;
        if (move_order)
            move_no++;
        write_pgn(from, to);
        write_fen();
    }
}

const std::list<Square> Game::get_possible_moves(const Square& pos)
{
    std::list<Square> moves = board.gen_moves(pos.x, pos.y);
    board.filter_legal(moves, pos);
    return moves;
}
void Game::move_back()
{
    if (current_pos == FEN.begin())
        return;
    current_pos--;
    from_short_fen(*current_pos);
    if (std::distance(FEN.begin(), current_pos)%2)
        board.check_king(C_BLACK);
    else
        board.check_king(C_WHITE);
}
void Game::move_forward()
{
    if (current_pos == --FEN.end())
        return;
    current_pos++;
    from_short_fen(*current_pos);
    if (std::distance(FEN.begin(), current_pos)%2)
        board.check_king(C_BLACK);
    else
        board.check_king(C_WHITE);
}

void Game::write_fen()
{
    std::string current_fen;
    for (uint8_t i = 0; i < Board::y_size; i++)
    {
        uint8_t empty = 0;
        for (uint8_t j = 0; j < Board::x_size; j++)
        {
            if (board.pieces[j][i])
            {
                if (empty)
                {
                    current_fen += (empty + 48);
                    empty = 0;
                }
                current_fen += board.piece_to_fen(board.pieces[j][i]);
            }
            else
                empty++;
        }
        if (empty)
            current_fen += (empty + 48);
        current_fen += '/';
    }
    current_fen.erase(current_fen.size() - 1);
    FEN.push_back(current_fen);
    current_pos++;
}
void Game::write_pgn(const Square& from, const Square& to)
{
    if (move_order)
    {
        if (move_no > 1)
            PGN += " ";
        PGN += std::to_string(move_no) + ".";
    }
    last_move = board.to_acn(from, to);
    if (board.king_checked*board.king_checked == 1)
        last_move += "+";
    else if (board.king_checked*board.king_checked == 4)
        last_move += "#";
    PGN += " " + last_move;
    end();
}
//----------state----------
void Game::end()
{
    if (board.king_checked == -2)
    {
        PGN += " 1-0";
        state = C_WHITE_WIN | C_MATE;
    }
    else if (board.king_checked == 2)
    {
        PGN += " 0-1";
        state = C_BLACK_WIN | C_MATE;
    }
    else if (board.halfmove_clock == 150)
    {
        PGN += "1/2-1/2";
        state = C_DRAW | C_ON_TIME;
    }
}
