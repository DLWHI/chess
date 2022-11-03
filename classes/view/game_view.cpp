#include "game_view.h"
#include <regex>

using namespace chess;

const SDL_Rect game_info_rect = {65, 0, 35, 100};
//----------constructors----------
GameViewer::GameViewer(Game& game) : view::TextRenderer("resources/FantasqueSansMono-Regular.ttf", 28), bound_game(game)
{
    border.set_width(3);
    border.set_color({0, 0, 0, 255});
    border.set_dim(dim);
    moves.push_back("");
}
GameViewer::GameViewer(const GameViewer& other) : view::TextRenderer(other), border(other.border), bound_game(other.bound_game)
{
    moves = other.moves;
}
view::BaseElement* GameViewer::clone(const std::string& id) const
{
    GameViewer* cl = new GameViewer(*this);
    cl->name = id;
    return cl;
}
//----------
void GameViewer::update_data()
{
    std::string move = bound_game.pop_last_move();
    if (!move.empty())
    {
        if (moves.back().empty())
            moves.back() += std::to_string(bound_game.get_move_no()) + ". " + move;
        else
        {
            moves.back() += " " + move;
            moves.push_back("");
        }
    }

    uint8_t state = bound_game.get_state();
    if ((state & C_RESULT_MASK) == C_WHITE_WIN)
        result = " White won";
    else if ((state & C_RESULT_MASK) == C_BLACK_WIN)
        result = " Black won";
    else if ((state & C_RESULT_MASK) == C_DRAW)
        result = " Draw";
    else
        result = "";

    if ((state & C_TERMINATION_MASK) == C_AGREEMENT)
        result += " by an agreement.";
    else if ((state & C_TERMINATION_MASK) == C_ON_TIME)
        result += " on time.";
    else if ((state & C_TERMINATION_MASK) == C_MATE)
        result += " by checkmate.";
    else if ((state & C_TERMINATION_MASK) == C_RESIGNATION)
        result += " by resignation.";
    else
        result += "";
}

void GameViewer::recalc_size(uint32_t win_w, uint32_t win_h)
{
    dim.x = win_w*game_info_rect.x/100;
    dim.y = win_h*game_info_rect.y/100;
    dim.w = win_w*game_info_rect.w/100;
    dim.h = win_h*game_info_rect.h/100;
}
//----------
void GameViewer::draw(SDL_Renderer* ren)
{
    SDL_SetRenderDrawColor(ren, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(ren, &dim);

    SDL_Rect border_carriage = dim;
    border_carriage.h = dim.h*history_area_ratio/100;
    border.set_dim(border_carriage);
    border.draw(ren);

    border_carriage.y += border_carriage.h;
    border_carriage.h = dim.h*result_area_ratio/100;
    border.set_dim(border_carriage);
    border.draw(ren);

    carriage.x = dim.x;
    carriage.y = dim.y;

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_Rect frame = {dim.x, dim.y, dim.w, carriage.h};
    for (auto i = moves.begin(); i != moves.end() && !i->empty(); i++)
    {
        for (auto j = i->begin(); j != i->end(); j++)
            draw_glyph(*j, ren, dim);
        SDL_RenderDrawRect(ren, &frame);
        frame.y += frame.h;
        carriage.y += carriage.h;
        carriage.x = dim.x;
    }

    if (!result.empty())
    {
        carriage.x = dim.x;
        carriage.y = dim.y + dim.h*history_area_ratio/100;

        for (auto i = result.begin(); i != result.end(); i++)
            draw_glyph(*i, ren, dim);
    }
}
void GameViewer::build_texture_cache(SDL_Renderer* ren)
{
    TextRenderer::build_texture_cache(ren);
    carriage.w = 15;
    carriage.h = 29;
}
