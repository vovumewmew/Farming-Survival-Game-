#include "game.h"

int main()
{
    Game game;
    if(game.init("Game Nong Trai Sinh Ton", 800, 600))
    {
        while(game.running())
        {
            game.handleEvents();
            game.update();
            game.render();
        }
    }
    game.clean();
    return 0;
}
