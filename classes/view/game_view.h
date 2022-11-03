#ifndef GV_H_
#define GV_H_
#include <text_renderer.h>
#include <border.h>
#include "../model/game.h"

namespace chess
{
    class GameViewer : public view::TextRenderer
    {
        public:
            GameViewer(Game& game);
            GameViewer(const GameViewer& other);
            ~GameViewer() = default;

            view::BaseElement* clone(const std::string& id) const;
            void draw(SDL_Renderer* ren);
            void build_texture_cache(SDL_Renderer* ren);

            void update_data();

            void recalc_size(uint32_t win_w, uint32_t win_h);
        private:
            SDL_Color bg_color = {255, 255, 255, 255};

            Game& bound_game;

            view::Border border;
            std::list<std::string> moves;
            std::string result;

            uint8_t history_area_ratio = 75;
            uint8_t result_area_ratio = 25;
    };
}
#endif