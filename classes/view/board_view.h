#ifndef BV_H_
#define BV_H_
#include <base_element.h>
#include <clickable.h>
#include "../model/game.h"
#include "game_view.h"
namespace chess
{
    class BoardViewer : public view::BaseElement, public view::Clickable
    {
        public:
            BoardViewer(Game& game, GameViewer& game_info);
            BoardViewer(const BoardViewer& other);
            ~BoardViewer();

            void render_to_terminal() const;
            void render_to_log() const;

            void draw(SDL_Renderer* ren);
            view::BaseElement* clone(const std::string& id) const;

            void recalc_board_size(uint32_t win_w, uint32_t win_h);
        private:
            Game& bound_game;
            GameViewer& viewer;

            Square selected_piece = {-1, -1};
            std::list<Square> current_moves;
            bool piece_grabbed = false;

            SDL_Texture* texture_atlas = nullptr;
            SDL_Rect piece_offset[Board::x_size][Board::y_size];

            SDL_Rect sq = {0, 0, 0, 0};
            SDL_Rect board = {0, 0, 0, 0};

            SDL_Color bg_color = {100, 100, 100, 255};

            void grab_piece(view::Clickable& sender, SDL_MouseButtonEvent data);
            void release_piece(view::Clickable& sender, SDL_MouseButtonEvent data);
            
            void call_promotion(SDL_MouseButtonEvent data);
            void keyboard_controller(SDL_KeyboardEvent data);
            void update_board();

            void build_texture_cache(SDL_Renderer* ren);
            void draw_square(SDL_Renderer* ren, SDL_Rect* square, SDL_Color color);
            void draw_board(SDL_Renderer* ren);
            void draw_pieces(SDL_Renderer* ren);
            void draw_promotion(SDL_Renderer* ren);

            bool is_move(uint8_t x, uint8_t y);
    };
}
#endif