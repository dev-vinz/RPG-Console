#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <stdlib.h>

#include "apps\game\include\Game.hpp"

using namespace std;
using namespace HE_Arc::RPG;

/*
    === IDEAS ===

    1) Choose between 3 heroes at the start of the game
    2) Commands to control the hero, like status, attack, dodge, block etc
    3) https://stackoverflow.com/questions/32203610/how-to-integrate-uml-diagrams-into-gitlab-or-github
*/

int main()
{
    Game myGame(10, 9, 3);

    string name;
    cout << " What's your name ? " << endl
         << " >>> ";

    getline(cin, name);

    myGame.choosePlayer(name);
    myGame.initialize();

    myGame.play();

    return 0;
}