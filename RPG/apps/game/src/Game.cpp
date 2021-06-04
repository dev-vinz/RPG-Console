#include "..\include\Game.hpp"

namespace HE_Arc::RPG
{
    /**
     * @brief Constructor per default
     * @brief All is chosen randomly
     */
    Game::Game()
    {
        system("CLS");
        srand(time(nullptr));

        this->currentMap = Map();
        this->player = nullptr;

        // We fix 4% from the map' size as opponents
        int rndOpp = trunc(0.04 * this->currentMap.getWidth() * this->currentMap.getHeight());
        this->nbOpponents = rndOpp;
        this->nbPotions = rndOpp * 2;
    }

    /**
     * @brief Constructor with initialization
     * @param _width The map's width
     * @param _height The map's height
     * @param _nbOpponents The number of opponents
     */
    Game::Game(int _width, int _height, int _nbOpponents) : nbOpponents(_nbOpponents), nbPotions(_nbOpponents * 2)
    {
        system("CLS");
        srand(time(nullptr));

        this->currentMap = Map(_width, _height);
        this->player = nullptr;
    }

    /**
     * @brief Destructor, delete the pointer on current Hero
     */
    Game::~Game()
    {
        if (this->player != nullptr)
        {
            delete this->player;
            this->player = nullptr;
        }
    }

    /**
     * @brief Get the map
     * @returns The map
     */
    Map Game::getMap() const
    {
        return this->currentMap;
    }

    /**
     * @brief Get the list of opponents
     * @returns The opponents
     */
    vector<Hero *> Game::getOpponents() const
    {
        return this->listOpponents;
    }

    /**
     * @brief Choose the player between three
     * @param _name The player's name
     */
    void Game::choosePlayer(string _name)
    {
        char choice;

        while (choice != '1' && choice != '2' && choice != '3')
        {
            cin.clear();
            cout << " Please choose your type of hero with 1, 2 or 3 :" << endl
                 << " [1] Warrior" << endl
                 << " [2] Wizard" << endl
                 << " [3] Necromancer" << endl
                 << " >>> ";

            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        switch (choice)
        {
        case '1':
            this->player = new Warrior(_name, 50, 20, 80, 100, new Sword(15));
            break;
        case '2':
            this->player = new Wizard(_name, 55, 90, 5, 0, 100, new MagicWand(30));
            break;
        case '3':
            this->player = new Necromancer(_name, 70, 40, 40, 0, 100, new Scepter(30));
            break;
        default:
            cout << "[ERROR] Houston we have a problem, choice can be only 1 2 or 3 (choice = " << choice << ")" << endl;
            exit(-1);
        }

        if (not Game::VJ_DEBUG_LOG)
            system("CLS");

        cout << " Here's your Hero : " << endl;
        this->player->show();

        cout << endl;

        if (not Game::VJ_DEBUG_LOG)
        {
            ConsoleController::displayLoading("Loading game", 5);
        }
    }

    /**
     * @brief Announce who has won
     */
    void Game::end() const
    {
        if (!Game::VJ_DEBUG_LOG)
            system("CLS");

        cout << " ============================================" << endl;

        switch (this->gWinner)
        {
        case Winner::WNone:
            cout << " " << ConsoleController::getCenter("Game Over", 45, '=') << endl;
            break;
        case Winner::WOpponent:
            cout << " " << ConsoleController::getCenter("Game Over", 45, '=') << endl;
            break;
        case Winner::WPlayer:
            cout << " " << ConsoleController::getCenter("Congratulations", 45, '=') << endl;
            break;
        case Winner::WQuit:
            cout << " " << ConsoleController::getCenter("Quitted", 45, '=') << endl;
            break;
        case Winner::WNull:
            cout << "[ERROR : Game::end()] : Winner is NULL" << endl;
            exit(-1);
        default:
            cout << "[ERROR : Game::end()] : Undefined state of winner" << endl;
            exit(-1);
        }

        cout << " ============================================" << endl;
    }

    /**
     * @brief Initialize the game, the opponents, the potions
     */
    void Game::initialize()
    {
        RandomGenerator random;

        for (int k = 0; k < this->nbOpponents; k++)
        {
            int opponent = random.getRandomNumber(3);

            switch (opponent)
            {
            case 0:
                this->listOpponents.push_back(new Inferi(random.getRandomName(), 10, 60, 50, 75));
                break;
            case 1:
                this->listOpponents.push_back(new Vampire(random.getRandomName(), 35, 50, 35, 75));
                break;
            case 2:
                this->listOpponents.push_back(new Medusa(random.getRandomName(), 20, 50, 50, 75));
                break;
            default:
                cout << "[ERROR : Game::initialize] Problem with random.getRandomNumber() = " << opponent;
                exit(-1);
            }
        }

        int start = random.getRandomNumber(3);
        for (int k = 0; k < this->nbPotions; k++)
        {
            Type _type = (Type)((start + k) % 3);
            this->listPotions.push_back(new Potion(random.getRandomNumber(7, 13), _type));
        }

        this->setPositions();
    }

    /**
     * @brief Play the game
     */
    void Game::play()
    {
        this->isPlaying = true;
        string _endLine;

        while (this->isPlaying)
        {
            this->display();

            if (this->currentMap.isOpponentNear(this->player->getPosX(), this->player->getPosY()))
            {
                Hero *opponent = this->currentMap.getOpponentNear(this->player->getPosX(), this->player->getPosY());

                cout << " There's an opponent nearly !" << endl
                     << " Do you want to fight " << opponent->getName() << " ?\a" << endl;

                char fight;
                do
                {
                    cout << " [Y] Yes, sure !" << endl
                         << " [N] No thank's" << endl
                         << " >>> ";

                    cin >> fight;
                    getline(cin, _endLine);

                    fight = tolower(fight, locale());
                } while (!this->checkFight(fight));

                // isPlaying can only switch to false here
                this->applyFight(fight, opponent);

                if (this->isPlaying)
                {
                    this->display();
                }
            }

            if (this->isPlaying)
            {
                char movement;
                do
                {
                    cout << " Choose your movement (w, a, s, d, n, q) : ";

                    cin >> movement;
                    getline(cin, _endLine);

                    movement = tolower(movement, locale());
                } while (not this->checkMovement(movement, this->player));

                this->applyMovements(movement);
            }
        }
    }

    // =================================================
    // Private Methods
    // =================================================

    /**
     * @brief Checks if the player wants to fight or not
     * @param _fight Yes or no
     * @returns True if the check is correct
     */
    bool Game::checkFight(char _fight) const
    {
        char tabFight[] = {'y', 'n'};
        if (!ConsoleController::checkInput(tabFight, _fight))
            return false;

        return true;
    }

    /**
     * @brief Checks the next hero's movement
     * @param _movement The movement
     * @param _hero The hero
     * @returns True if the check is correct
     */
    bool Game::checkMovement(char _movement, Hero *_hero) const
    {
        char tabMovements[] = {'w', 'a', 's', 'd', 'n', 'q'};

        if (!ConsoleController::checkInput(tabMovements, _movement))
            return false;

        int posX = _hero->getPosX();
        int posY = _hero->getPosY();

        switch (_movement)
        {
        case 'w':
            posY--;
            break;
        case 'a':
            posX--;
            break;
        case 's':
            posY++;
            break;
        case 'd':
            posX++;
            break;
        case 'n':
        case 'q':
            return true;
        default:
            cout << "[ERROR : CheckMovement] Houston we have a problem, there's another movement detected (_movement = " << _movement << ")" << endl;
            exit(-1);
        }

        if (not this->currentMap.isCaseEmpty(posX, posY) && this->currentMap.whatIs(posX, posY) == _Hero)
        {
            if (_hero->getIsPlayer())
            {
                cout << " The case (" << posX << "; " << posY << ") is busy by " << this->currentMap.whoIs(posX, posY)->getName() << endl;
            }

            return false;
        }

        if (posY == this->currentMap.getHeight() || posY < 0 || posX == this->currentMap.getWidth() || posX < 0)
        {
            if (_hero->getIsPlayer())
            {
                cout << " You're not in the map anymore with this movement" << endl;
            }

            return false;
        }

        if (this->currentMap.whatIs(posX, posY) == _Potion && not _hero->getIsPlayer())
        {
            return false;
        }

        return true;
    }

    /**
     * @brief Apply the player's choice about fight
     * @param _fight Yes (y) or no (n)
     * @param _opponent The opponent
     */
    void Game::applyFight(char _fight, Hero *_opponent)
    {
        if (_fight == 'y')
        {
            Battle anyBattle(this->player, _opponent);

            if (!Game::VJ_DEBUG_LOG)
                ConsoleController::displayLoading("Loading battle", 2);

            Hero *winner = anyBattle.getWinner();

            if (!Game::VJ_DEBUG_LOG)
                system("CLS");

            if (winner == this->player)
            {
                if (Game::VJ_DEBUG_LOG)
                {
                    cout << "PLAYER HAS WON" << endl;
                    this->isPlaying = false;
                    this->gWinner = Winner::WPlayer;
                }

                vector<Hero *>::iterator iterator = find(this->listOpponents.begin(), this->listOpponents.end(), _opponent);
                int index = -1;

                if (iterator != this->listOpponents.end())
                {
                    index = iterator - this->listOpponents.begin();
                }
                else
                {
                    cout << "[ERROR : Game::applyFight()] Opponent isn't in list anymore";
                    exit(-1);
                }

                this->listOpponents.erase(this->listOpponents.begin() + index);
                this->currentMap.update(this->listOpponents);

                if (this->listOpponents.empty())
                {
                    this->isPlaying = false;
                    this->gWinner = Winner::WPlayer;
                }

                cout << " You win against " << _opponent->getName() << " !" << endl;
            }
            else if (winner == _opponent)
            {
                if (Game::VJ_DEBUG_LOG)
                {
                    cout << "OPPONENT HAS WON" << endl
                         << "GAME OVER" << endl;
                }

                if (this->player->isDead())
                {
                    this->isPlaying = false;
                    this->gWinner = Winner::WOpponent;
                }
            }
            else if (winner == nullptr)
            {
                if (Game::VJ_DEBUG_LOG)
                {
                    cout << "NOBODY HAS WON" << endl
                         << "GAME OVER" << endl;
                }

                this->isPlaying = false;
                this->gWinner = Winner::WNone;
            }
            else
            {
                cout << " [ERROR : Game::applyFight()] : Unknown response from Battle::getWinner()" << endl;
                exit(-1);
            }

            if (!Game::VJ_DEBUG_LOG)
                ConsoleController::displayLoading("Closing battle", 2);
        }
    }

    /**
     * @brief Apply the player and opponents movements
     * @param _movement The player's movement
     */
    void Game::applyMovements(char _movement)
    {
        vector<Hero *> listHeroes;
        vector<Potion *> listPotions;

        int posX = this->player->getPosX();
        int posY = this->player->getPosY();

        switch (_movement)
        {
        case 'w':
            posY--;
            break;
        case 'a':
            posX--;
            break;
        case 's':
            posY++;
            break;
        case 'd':
            posX++;
            break;
        case 'n':
            break;
        case 'q':
            this->isPlaying = false;
            this->gWinner = Winner::WQuit;
            return;
        default:
            cout << "[ERROR : Game::applyMovements] Houston we have a problem, there's another movement detected (_movement = " << _movement << ")" << endl;
            exit(-1);
        }

        this->player->setPosXY(posX, posY);
        listHeroes.push_back(this->player);

        if (this->currentMap.whatIs(posX, posY) == _Potion)
            this->catchPotion(posX, posY);

        RandomGenerator random;

        for (Hero *opp : this->listOpponents)
        {
            int count = 0;
            bool isMovementCorrect = false;
            int rndNb;

            this->currentMap.update(listHeroes);

            do
            {
                do
                {
                    rndNb = random.getRandomNumber(5);

                    posX = opp->getPosX();
                    posY = opp->getPosY();

                    count++;

                    switch (rndNb)
                    {
                    case 0:
                        isMovementCorrect = this->checkMovement('w', opp);
                        break;
                    case 1:
                        isMovementCorrect = this->checkMovement('a', opp);
                        break;
                    case 2:
                        isMovementCorrect = this->checkMovement('s', opp);
                        break;
                    case 3:
                        isMovementCorrect = this->checkMovement('d', opp);
                        break;
                    case 4:
                        isMovementCorrect = this->checkMovement('n', opp);
                        break;
                    default:
                        cout << "[ERROR : Game::applyMovements] Problem with getRandomNumber(int _max) = " << rndNb << endl;
                        exit(-1);
                    }
                } while (!isMovementCorrect);

                switch (rndNb)
                {
                case 0:
                    posY--;
                    break;
                case 1:
                    posX--;
                    break;
                case 2:
                    posY++;
                    break;
                case 3:
                    posX++;
                    break;
                case 4:
                    break;
                default:
                    cout << "[ERROR : Game::applyMovements] Forgot a case in second switch (rndNb = " << rndNb << ")" << endl;
                    exit(-1);
                }

                opp->setPosXY(posX, posY);
            } while (!this->currentMap.isCaseEmpty(posX, posY));

            listHeroes.push_back(opp);
        }

        this->currentMap.update(listHeroes);
        this->currentMap.update(this->listPotions);
    }

    /**
     * @brief Catch a potion and add it to the backpack
     * @param _x The position X
     * @param _y The position Y
     */
    void Game::catchPotion(int _x, int _y)
    {
        Potion *anyPotion = this->currentMap.whichIs(_x, _y);

        if (anyPotion == nullptr)
        {
            cout << "[ERROR : Game::catchPotion] Potion = nullptr" << endl;
            exit(-1);
        }

        vector<Potion *>::iterator iterator = find(this->listPotions.begin(), this->listPotions.end(), anyPotion);
        int index = -1;

        if (iterator != this->listPotions.end())
        {
            index = iterator - this->listPotions.begin();
        }
        else
        {
            cout << "[ERROR : catchPotion] Potion isn't in list anymore";
            exit(-1);
        }

        if (Game::VJ_DEBUG_LOG)
        {
            cout << "BackPack Before :" << endl;
            this->player->backPack.show(Game::VJ_DEBUG_LOG);

            cout << endl
                 << "List of Potion Before :" << endl;

            for (auto &item : this->listPotions)
            {
                cout << item->getName() << endl;
            }

            cout << endl
                 << "Index : " << index << endl;
        }

        this->listPotions.erase(this->listPotions.begin() + index);
        this->player->backPack.pack(anyPotion);

        if (Game::VJ_DEBUG_LOG)
        {
            cout << endl
                 << "List of Potion After :" << endl;

            for (auto &item : this->listPotions)
            {
                cout << item->getName() << endl;
            }

            cout << endl
                 << "BackPack After :" << endl;
            this->player->backPack.show(Game::VJ_DEBUG_LOG);
        }
    }

    /**
     * @brief Display the game
     */
    void Game::display() const
    {
        if (not Game::VJ_DEBUG_LOG)
            system("CLS");

        this->currentMap.display(*this->player, this->listOpponents, this->listPotions);

        cout << " ============================================" << endl
             << " " << ConsoleController::getCenter("INSTRUCTIONS", 45, '=') << endl
             << " ============================================" << endl;

        cout << endl
             << "           w (up)" << endl
             << " a (left)  s (down)  d (right)" << endl
             << endl
             << " Not moving : n" << endl
             << " Quit the game : q" << endl;

        cout << " ============================================" << endl;
    }

    /**
     * @brief Set the positions
     */
    void Game::setPositions()
    {
        RandomGenerator random;

        int rndX = random.getRandomNumber(this->currentMap.getWidth());
        int rndY = random.getRandomNumber(this->currentMap.getHeight());

        vector<Hero *> listHeroes;
        vector<Potion *> listPotions;

        this->player->setPosXY(rndX, rndY);
        listHeroes.push_back(this->player);

        for (Hero *opp : this->listOpponents)
        {
            this->currentMap.update(listHeroes);

            do
            {
                rndX = random.getRandomNumber(this->currentMap.getWidth());
                rndY = random.getRandomNumber(this->currentMap.getHeight());

                opp->setPosXY(rndX, rndY);
            } while (!this->currentMap.isCaseEmpty(rndX, rndY));

            listHeroes.push_back(opp);
        }

        this->currentMap.update(listHeroes);

        for (Potion *potion : this->listPotions)
        {
            this->currentMap.update(listPotions);

            do
            {
                rndX = random.getRandomNumber(this->currentMap.getWidth());
                rndY = random.getRandomNumber(this->currentMap.getHeight());

                potion->setPosXY(rndX, rndY);
            } while (!this->currentMap.isCaseEmpty(rndX, rndY));

            listPotions.push_back(potion);
        }
        this->currentMap.update(listPotions);
    }
}