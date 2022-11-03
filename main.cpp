#include "classes/model/game.h"
#include "classes/view/board_view.h"
#include "classes/view/game_view.h"
#include <view_lib.h>
#include <iostream>
class initializer
{
    public:
        void set_all(view::Window& win, chess::Game& game)
        {
            chess::GameViewer gviewer(game);
            win.canvas().push_element(gviewer, "game");
            chess::GameViewer& gviewer_element = dynamic_cast<chess::GameViewer&>(win.canvas().get_element("game"));
            win.on_window_resize() += std::bind(chess::GameViewer::recalc_size, &gviewer_element, 
                                                std::placeholders::_1, std::placeholders::_2);
            gviewer_element.recalc_size(win.get_dimensions().w, win.get_dimensions().h);

            chess::BoardViewer bviewer(game, gviewer_element);
            win.canvas().push_element(bviewer, "board");
            chess::BoardViewer& bviewer_element = dynamic_cast<chess::BoardViewer&>(win.canvas().get_element("board"));
            win.on_window_resize() += std::bind(chess::BoardViewer::recalc_board_size, &bviewer_element, 
                                                std::placeholders::_1, std::placeholders::_2);
            bviewer_element.recalc_board_size(win.get_dimensions().w, win.get_dimensions().h);
        }

        int status = 0;
};

int main()
{
    initializer i;
    if (i.status)
        return -1;
    view::Window window("Poor chess", 1280, 720);
    //chess::Game game("8/4P3/8/8/8/k7/4p3/K7 w - - 0 1");
    chess::Game game;

    i.set_all(window, game);

    while (window.get_status() == view::WindowState::Open)
        window.update();
    return 0;
}
