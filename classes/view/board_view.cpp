#include "board_view.h"
#include <iostream>
#include <fstream>
#include <SDL2/SDL_image.h>
using namespace chess;

const SDL_Color black = {0, 0, 0, 255};
const SDL_Color white = {255, 255, 255, 255};
const SDL_Color green = {0, 255, 0, 255};
const SDL_Color red = {255, 0, 0, 255};

const SDL_Rect board_rect = {0, 0, 65, 100};

const std::string atlas_file = "resources/img/pieces_textures.png";

//----------constructors----------
BoardViewer::BoardViewer(Game& game, GameViewer& game_info) : bound_game(game), viewer(game_info)
{
    on_mb_down += std::bind(grab_piece, this, std::placeholders::_1, std::placeholders::_2);
    on_mb_up += std::bind(release_piece, this, std::placeholders::_1, std::placeholders::_2);
    update_board();
}
BoardViewer::BoardViewer(const BoardViewer& other) : BaseElement(other), bound_game(other.bound_game), viewer(other.viewer)
{
    sq = other.sq;
    board = other.board;
    texture_atlas = other.texture_atlas;
    
    on_mb_down += std::bind(grab_piece, this, std::placeholders::_1, std::placeholders::_2);
    on_mb_up += std::bind(release_piece, this, std::placeholders::_1, std::placeholders::_2);

    update_board();
}
BoardViewer::~BoardViewer()
{
    SDL_DestroyTexture(texture_atlas);
}

view::BaseElement* BoardViewer::clone(const std::string& id) const
{
    BoardViewer* cl = new BoardViewer(*this);
    cl->name = id;
    return cl;
}
//----------event-functions----------
void BoardViewer::grab_piece(view::Clickable& sender, SDL_MouseButtonEvent data)
{
    if (bound_game.get_promotion_pawn().x != -1)
        return call_promotion(data);
    uint8_t x = (data.x - board.x)/sq.w;
    uint8_t y = (data.y - board.y)/sq.h;
    uint8_t move = bound_game.get_move_order();
    if (0 <= y && y < Board::x_size && 0 <= x && x < Board::x_size)
    {
        piece_grabbed = true;
        if (selected_piece.x != x || selected_piece.y != y)
        {
            if (piece_offset[x][y].y >= 0 && piece_offset[x][y].y/2 == move)
            {
                selected_piece.x = x;
                selected_piece.y = y;
                current_moves = bound_game.get_possible_moves(Square(x, y));
                return;
            }
            for (auto i = current_moves.begin(); i != current_moves.end(); i++)
                if (i->x == x && i->y == y)
                {
                    bound_game.move(selected_piece, Square(x, y), 0);
                    viewer.update_data();
                    update_board();
                    break;
                }
            selected_piece.x = -1;
            current_moves.clear();
        }
    }
}
void BoardViewer::release_piece(view::Clickable& sender, SDL_MouseButtonEvent data)
{
    uint8_t x = (data.x - board.x)/sq.w;
    uint8_t y = (data.y - board.y)/sq.h;
    if (0 <= y && y < Board::x_size && 0 <= x && x < Board::x_size)
    {
        if (piece_offset[x][y].y < 0 || piece_offset[x][y].y/2 != bound_game.get_move_order())
        {
            for (auto i = current_moves.begin(); i != current_moves.end(); i++)
                if (i->x == x && i->y == y)
                {
                    bound_game.move(selected_piece, Square(x, y), 0);
                    viewer.update_data();
                    update_board();
                    break;
                }
            selected_piece.x = -1;
            current_moves.clear();
        }
    }
    piece_grabbed = false;
}

void BoardViewer::call_promotion(SDL_MouseButtonEvent data)
{
    Square pressed = {(data.x - board.x)/sq.w, (data.y - board.y)/sq.h};
    Square pp = bound_game.get_promotion_pawn();
    uint8_t promotion = -1;
    if ((pressed.y == 0 || pressed.y == 7) && pressed.x == pp.x)
        promotion = C_QUEEN;
    else if ((pressed.y == 1 || pressed.y == 6) && pressed.x == pp.x)
        promotion = C_ROOK;
    else if ((pressed.y == 2 || pressed.y == 5) && pressed.x == pp.x)
        promotion = C_BISHOP;
    else if ((pressed.y == 3 || pressed.y == 4) && pressed.x == pp.x)
        promotion = C_KNIGHT;
    bound_game.move(selected_piece, selected_piece, promotion);
    viewer.update_data();
    update_board();
}

void BoardViewer::keyboard_controller(SDL_KeyboardEvent data)
{
    if (data.type == SDL_KEYUP)
    {
        if (data.keysym.scancode == SDL_SCANCODE_LEFT)
            bound_game.move_back();
        else if (data.keysym.scancode == SDL_SCANCODE_RIGHT)
            bound_game.move_forward();
    }
}
void BoardViewer::recalc_board_size(uint32_t win_w, uint32_t win_h)
{
    dim.x = win_w*board_rect.x/100;
    dim.y = win_h*board_rect.y/100;
    dim.w = win_w*board_rect.w/100;
    dim.h = win_h*board_rect.h/100;

    sq.w = dim.w/10;
    sq.h = dim.h/10;

    sq.w = std::min(sq.w, sq.h);
    sq.h = sq.w;

    board.x = dim.w/2 - sq.w*Board::x_size/2;
    board.y = dim.h/2 - sq.h*Board::y_size/2;
    board.w = sq.w*Board::x_size;
    board.h = sq.h*Board::y_size;

    on_resize_event(*this, dim);
}

void BoardViewer::update_board()
{
    const board_type& pieces = bound_game.get_board();
    for (uint8_t i = 0; i < Board::y_size; sq.y += sq.h, sq.x = board.x, i++)
        for (uint8_t j = 0; j < Board::x_size; sq.x += sq.w, j++)
            if (pieces[j][i])
            {
                uint8_t offset = 0;
                while (!((pieces[j][i] >> offset) & 1))
                    offset++;
                piece_offset[j][i].x = offset*256;
                piece_offset[j][i].y = (pieces[j][i] & C_WHITE) ? 256 : 0;
                piece_offset[j][i].w = piece_offset[j][i].h = 256;
            }
            else
                piece_offset[j][i].y = -1;
}
//----------rendering----------
void BoardViewer::draw(SDL_Renderer* ren)
{
    draw_square(ren, &dim, bg_color);
    draw_board(ren);
    draw_pieces(ren);
    if (bound_game.get_promotion_pawn().x != -1)
        draw_promotion(ren);
}

void BoardViewer::build_texture_cache(SDL_Renderer* ren)
{
    SDL_DestroyTexture(texture_atlas);
    SDL_Surface* surf = IMG_Load(atlas_file.c_str());
    texture_atlas = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
}

void BoardViewer::draw_square(SDL_Renderer* ren, SDL_Rect* square, SDL_Color color)
{
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(ren, square);
}
void BoardViewer::draw_board(SDL_Renderer* ren)
{
    sq.x = board.x;
    sq.y = board.y;
    Square checked_king = bound_game.is_king_checked();
    for (uint8_t i = 0; i < Board::y_size; sq.y += sq.h, sq.x = board.x, i++)
        for (uint8_t j = 0; j < Board::x_size; sq.x += sq.w, j++)
            if (checked_king.x == j && checked_king.y == i)
                draw_square(ren, &sq, red);
            else if (is_move(j, i))
                draw_square(ren, &sq, green);
            else if ((i + j)%2)
                draw_square(ren, &sq, black);
            else
                draw_square(ren, &sq, white);
}
void BoardViewer::draw_pieces(SDL_Renderer* ren)
{
    sq.x = board.x;
    sq.y = board.y;
    
    for (uint8_t i = 0; i < Board::y_size; sq.y += sq.h, sq.x = board.x, i++)
        for (uint8_t j = 0; j < Board::x_size; sq.x += sq.w, j++)
        {
            if (piece_offset[j][i].y < 0 || (j == selected_piece.x && i == selected_piece.y && piece_grabbed))
                continue;  
            SDL_RenderCopy(ren, texture_atlas, &piece_offset[j][i], &sq);
        }
    if (piece_grabbed && selected_piece.x != -1)
    {
        SDL_Rect pos = sq;
        SDL_GetMouseState(&pos.x, &pos.y);
        pos.x -= sq.w/2;
        pos.y -= sq.h/2;
        SDL_RenderCopy(ren, texture_atlas, &piece_offset[selected_piece.x][selected_piece.y], &pos);
    }
}
void BoardViewer::draw_promotion(SDL_Renderer* ren)
{
    SDL_Color pelena = {0, 0, 0, 128};
    draw_square(ren, &board, pelena);
    Square pp = bound_game.get_promotion_pawn();
    uint8_t color = bound_game.get_move_order() ;
    sq.x = board.x + pp.x*sq.w;
    sq.y = (color) ? board.y : board.y + (Board::y_size - 1)*sq.h;
    SDL_Rect stencil = {0, 0, 256, 256};
    stencil.y = (color) ? 256 : 0;
    for (uint8_t offset = 4; offset >= 1; offset--)
    {
        stencil.x = offset*stencil.w;
        SDL_RenderCopy(ren, texture_atlas, &stencil, &sq);
        sq.y = (color) ? sq.y + sq.h : sq.y - sq.h;
    }
}

bool BoardViewer::is_move(uint8_t x, uint8_t y)
{
    for (auto i = current_moves.begin(); i != current_moves.end(); i++)
        if (i->x == x && i->y == y)
            return true;
    return false;
}
//----------debug-rendering----------
void BoardViewer::render_to_terminal() const
{
    const board_type& pieces = bound_game.get_board();
    for (uint8_t i = 0; i < Board::y_size; i++)
    {
        for (uint8_t j = 0; j < Board::x_size; j++)
        {
            if (pieces[j][i])
                std::cout << pieces[j][i];
            else
                std::cout << '.';
        }
        std::cout << std::endl;
    }
}
void BoardViewer::render_to_log() const
{
    
}
