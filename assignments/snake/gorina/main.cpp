#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <mutex>

enum Direction { LEFT = 1, UP, RIGHT, DOWN };
const int MAX_TAIL_SIZE = 1000;
const int START_TAIL_SIZE = 3;
const int MAX_FOOD_SIZE = 20;
const int FOOD_EXPIRE_SECONDS = 10;

enum GameResult { NONE, SNAKE1_WON, SNAKE2_WON, DRAW };

struct Tail { int x, y; };

struct Food {//структура для еды
    int x, y;
    time_t put_time;
    char symbol;
    bool enabled;

    Food() : x(0), y(0), put_time(0), symbol('$'), enabled(false) {}

    //Случайное размещение еды
    void placeRandomly(int maxX, int maxY, const std::vector<Tail>& snake1_tail, const std::vector<Tail>& snake2_tail) {
        bool conflict;
        do {
            conflict = false;
            x = rand() % (maxX - 2) + 1;
            y = rand() % (maxY - 6) + 4;
            for (const auto& t : snake1_tail) { //размещение не на змейках
                if (x == t.x && y == t.y) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) {
                for (const auto& t : snake2_tail) {
                    if (x == t.x && y == t.y) {
                        conflict = true;
                        break;
                    }
                }
            }
        } while (conflict);
        put_time = time(nullptr);
        enabled = true;
    }

    //Еда съедена?
    bool isEatenBy(int snake_x, int snake_y) {
        if (enabled && x == snake_x && y == snake_y) {
            enabled = false;
            return true;
        }
        return false;
    }

    //Отображение еды на экране
    void render() const {
        if (enabled) {
            mvprintw(y, x, "%c", symbol);
        }
    }

    //Время еды истекло?
    bool isExpired() const {
        return enabled && (time(nullptr) - put_time) > FOOD_EXPIRE_SECONDS;
    }
};

class Obstacle { //препятствие
public:
    Obstacle() : x(0), y(0), enabled(false) {}

    //случайно размещаем препятствие
    void placeRandomly(int maxX, int maxY) {
        x = rand() % (maxX - 2) + 1;
        y = rand() % (maxY - 6) + 4;
        enabled = true;
    }

    //отключение препятствия
    void disable() {
        enabled = false;
    }

    //отображение препятствия на экране
    void render() const {
        if (enabled) {
            attron(COLOR_PAIR(1));
            mvprintw(y, x, "#");
            attroff(COLOR_PAIR(1));
        }
    }

    //змейка столкнулась с препятствием?
    bool isHitBy(int snake_x, int snake_y) const {
        return enabled && x == snake_x && y == snake_y;
    }

    //препятствие активно?
    bool isEnabled() const {
        return enabled;
    }

    //получение координат препятствия
    int getX() const { return x; } 
    int getY() const { return y; }

private:
    int x, y;
    bool enabled;
};

class Bonus { //бонус - увеличивает длину на 2
public:
    Bonus() : x(0), y(0), enabled(false) {}

    //случайно размещаем бонус
    void placeRandomly(int maxX, int maxY, const std::vector<Tail>& snake1_tail, const std::vector<Tail>& snake2_tail) {
        bool conflict;
        do {
            conflict = false;
            x = rand() % (maxX - 2) + 1;
            y = rand() % (maxY - 6) + 4;
            for (const auto& t : snake1_tail) { //размещение не на змейках
                if (x == t.x && y == t.y) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) {
                for (const auto& t : snake2_tail) {
                    if (x == t.x && y == t.y) {
                        conflict = true;
                        break;
                    }
                }
            }
        } while (conflict);
        enabled = true;
    }

    //бонус съеден?
    bool isEatenBy(int snake_x, int snake_y) {
        if (enabled && x == snake_x && y == snake_y) {
            enabled = false;
            return true;
        }
        return false;
    }

    //отображение бонуса
    void render() const {
        if (enabled) {
            attron(COLOR_PAIR(2));
            mvprintw(y, x, "A"); // 'A' for Apple
            attroff(COLOR_PAIR(2));
        }
    }

private:
    int x, y;
    bool enabled;
};

class Snake { //змейка
public:
    Snake(int startX, int startY, Direction dir, char headCh, char tailCh)
        : x(startX), y(startY), direction(dir), tsize(START_TAIL_SIZE), headChar(headCh), tailChar(tailCh) {
        tail.resize(MAX_TAIL_SIZE);
        for (size_t i = 0; i < tsize; ++i) {
            tail[i].x = x;
            tail[i].y = y;
        }
    }

    //перемещение змейки
    void move() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(y, x, " ");

        switch (direction) {
            case LEFT: x = (x <= 1) ? max_x - 2 : x - 1; break;
            case RIGHT: x = (x >= max_x - 2) ? 1 : x + 1; break;
            case UP: y = (y <= 1) ? max_y - 2 : y - 1; break;
            case DOWN: y = (y >= max_y - 2) ? 1 : y + 1; break;
        }
    }

    //перемещение хвоста
    void moveTail() {
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " ");
        for (size_t i = tsize - 1; i > 0; --i) {
            tail[i] = tail[i - 1];
        }
        tail[0].x = x;
        tail[0].y = y;
    }

    //изменение направления движения
    void changeDirection(Direction newDir) {
        if ((direction == LEFT && newDir != RIGHT) ||
            (direction == RIGHT && newDir != LEFT) ||
            (direction == UP && newDir != DOWN) ||
            (direction == DOWN && newDir != UP)) {
            direction = newDir;
        }
    }

    //увеличение длины 
    void grow() {
        if (tsize < MAX_TAIL_SIZE) {
            tsize++;
        }
    }

    //отображение змейки
    void render() const {
        mvprintw(y, x, "%c", headChar);
        for (size_t i = 1; i < tsize; ++i) {
            mvprintw(tail[i].y, tail[i].x, "%c", tailChar);
        }
    }

    //проверка: столкнулась ли змейка сама с собой
    bool checkSelfCollision() const {
        for (size_t i = 1; i < tsize; ++i) {
            if (x == tail[i].x && y == tail[i].y) {
                return true;
            }
        }
        return false;
    }

    //проверка: столкнулась ли с препятствием
    bool checkCollisionWithObstacle(const std::vector<Obstacle>& obstacles) const {
        for (const auto& obstacle : obstacles) {
            if (obstacle.isEnabled() && x == obstacle.getX() && y == obstacle.getY()) {
                return true; 
            }
        }
        return false;
    }    

    int getX() const { return x; }
    int getY() const { return y; }
    size_t getSize() const { return tsize; }
    std::vector<Tail> getTail() const { return std::vector<Tail>(tail.begin(), tail.begin() + tsize); }

private:
    int x, y;
    Direction direction;
    size_t tsize;
    std::vector<Tail> tail;
    char headChar;
    char tailChar;
};

class Game { //представление игры
public:
    Game() : snake1(nullptr), snake2(nullptr), gameOver(false), result(NONE) {
        foods.resize(MAX_FOOD_SIZE);
        obstacles.resize(5); 
        bonuses.resize(3);
    }

    ~Game() {
        delete snake1;
        delete snake2;
    }

    //инициализация
    void init() {
        srand(time(nullptr));
        initscr();
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK); 
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        keypad(stdscr, TRUE);
        raw();
        noecho();
        curs_set(FALSE);
        nodelay(stdscr, TRUE);
        getmaxyx(stdscr, max_y, max_x);

        // Увеличиваем ширину и высоту
        max_x = 80;
        max_y = 30;

        if (max_x < 10 || max_y < 10) {
            endwin();
            std::cerr << "Window size is too small!" << std::endl;
            exit(1);
        }

        snake1 = new Snake(max_x / 4, max_y / 2, RIGHT, '@', '*');
        snake2 = new Snake(3 * max_x / 4, max_y / 2, LEFT, '#', '*');

        for (auto &food : foods) {
            placeFood(food);
        }

        createWalls(); // Создаем стены
    }

    //запуск игры
    void run() {
        std::thread obstacleThread(&Game::updateObstacles, this);
        std::thread bonusThread(&Game::updateBonuses, this);
    
        while (!gameOver) {
            handleInput();
            moveSnakes();
            checkFoodConsumption();
            checkBonusConsumption();
            refreshFood();
            render();
            checkCollisions();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        }
        obstacleThread.detach();
        bonusThread.detach();

    }

    //завершение игры
    void end() {
        gameOver = true;
        clear();
        mvprintw(max_y / 2 - 1, (max_x / 2), "Game Over!");
        mvprintw(max_y / 2 + 1, (max_x / 2) - 15, "Snake '@' ate %ld foods, Snake '#' ate %ld foods", snake1->getSize(), snake2->getSize());

        if (result == SNAKE1_WON) {
            mvprintw(max_y / 2 + 2, (max_x / 2) - 2, "Snake '@' wins!");
        } else if (result == SNAKE2_WON) {
            mvprintw(max_y / 2 + 2, (max_x / 2) - 2, "Snake '#' wins!");
        } else if (result == DRAW) {
            mvprintw(max_y / 2 + 2, (max_x / 2) - 2, "It's a draw!");
        }

        mvprintw(max_y / 2 + 4, (max_x / 2) - 5, "Press any key to exit.");
        nodelay(stdscr, FALSE);
        getch();
        endwin();
    }


private:
    Snake *snake1;
    Snake *snake2;
    std::vector<Food> foods;
    bool gameOver;
    GameResult result;
    int max_y, max_x;
    std::vector<std::pair<int, int>> walls; 
    std::vector<Obstacle> obstacles;
    std::vector<Bonus> bonuses;

    // обновление препятствий
    void updateObstacles() {
        while (true) {
            if (gameOver) 
                break; 
            
            for (auto& obstacle : obstacles) {
                if (!obstacle.isEnabled()) {
                    obstacle.placeRandomly(max_x, max_y);
                }
                if (!gameOver) {
                    std::this_thread::sleep_for(std::chrono::seconds(10)); // Пауза перед изменением
                    obstacle.disable(); // Убираем препятствие
                }
            }
        }
    }

    //обновление бонусов
    void updateBonuses() {
        while (true) {
            if (gameOver) 
                break; 

            for (auto& bonus : bonuses) {
                if (!bonus.isEatenBy(snake1->getX(), snake1->getY()) && !bonus.isEatenBy(snake2->getX(), snake2->getY())) {
                    bonus.placeRandomly(max_x, max_y, snake1->getTail(), snake2->getTail());
                }
                if (!gameOver)
                    std::this_thread::sleep_for(std::chrono::seconds(10)); // Пауза перед появлением нового бонуса
            }
        }
    }

    void checkWinner() {
        if (snake1->getSize() > snake2->getSize())
            result = SNAKE1_WON;
        else
            result = SNAKE2_WON;

    }
    //обработка ввода
    void handleInput() {
        int ch = getch();
        switch (ch) {
            case KEY_UP: snake1->changeDirection(UP); break;
            case KEY_DOWN: snake1->changeDirection(DOWN); break;
            case KEY_LEFT: snake1->changeDirection(LEFT); break;
            case KEY_RIGHT: snake1->changeDirection(RIGHT); break;
            case 'w': snake2->changeDirection(UP); break;
            case 's': snake2->changeDirection(DOWN); break;
            case 'a': snake2->changeDirection(LEFT); break;
            case 'd': snake2->changeDirection(RIGHT); break;
            case 'q': checkWinner(); gameOver = true; break;
        }
    }

    //перемещение змеек
    void moveSnakes() {
        snake1->move();
        snake1->moveTail();
        snake2->move();
        snake2->moveTail();
    }

    //проверка поедания еды
    void checkFoodConsumption() {
        for (auto &food : foods) {
            if (food.isEatenBy(snake1->getX(), snake1->getY())) {
                snake1->grow();
                placeFood(food);
            }
            if (food.isEatenBy(snake2->getX(), snake2->getY())) {
                snake2->grow();
                placeFood(food);
            }
        }
    }

    //проверка поедания бонусов
    void checkBonusConsumption() {
        for (auto &bonus : bonuses) {
            if (bonus.isEatenBy(snake1->getX(), snake1->getY())) {
                snake1->grow();
                snake1->grow(); 
                placeBonus(bonus);
            }
            if (bonus.isEatenBy(snake2->getX(), snake2->getY())) {
                snake2->grow();
                snake2->grow();
                placeBonus(bonus);
            }
        }
    }

    //размещение бонуса
    void placeBonus(Bonus &bonus) {
        bonus.placeRandomly(max_x, max_y, snake1->getTail(), snake2->getTail());
        bonus.render();
    }

    //размещение еды
    void placeFood(Food &food) {
        food.placeRandomly(max_x, max_y, snake1->getTail(), snake2->getTail());
        food.render();
    }

    //обновление еды
    void refreshFood() {
        for (auto &food : foods) {
            if (food.isExpired()) {
                placeFood(food);
            }
        }
    }

    //отображение игры
    void render() {
        mvprintw(1, 2, "Use arrows for control Snake '@'. Use WASD for control Snake '#'. Press 'q' for EXIT");
        mvprintw(2, 2, "Snake '@' Length: %zu, Snake '#' Length: %zu", snake1->getSize(), snake2->getSize());

        renderWalls();

        snake1->render();
        snake2->render();

        for (const auto &food : foods) {
            food.render();
        }

        for (const auto &bonus : bonuses) {
            bonus.render();
        }

        for (const auto &obstacle : obstacles) {
            obstacle.render();
        }

        refresh();
    }

    //проверка столкновений
    void checkCollisions() {
        bool snake1_hits_snake2_body = false;
        bool snake2_hits_snake1_body = false;
        bool heads_collide = false;

        std::vector<Tail> snake1_tail = snake1->getTail();
        std::vector<Tail> snake2_tail = snake2->getTail();

        // Проверка на столкновение головы змейки 1 с телом змейки 2
        for (size_t i = 1; i < snake2_tail.size(); ++i) {
            if (snake1->getX() == snake2_tail[i].x && snake1->getY() == snake2_tail[i].y) {
                snake1_hits_snake2_body = true;
                break;
            }
        }

        // Проверка на столкновение головы змейки 2 с телом змейки 1
        for (size_t i = 1; i < snake1_tail.size(); ++i) {
            if (snake2->getX() == snake1_tail[i].x && snake2->getY() == snake1_tail[i].y) {
                snake2_hits_snake1_body = true;
                break;
            }
        }

        // Проверка на столкновение голов змей
        if (snake1->getX() == snake2->getX() && snake1->getY() == snake2->getY()) {
            heads_collide = true;
        }

        // Определение результата игры на основе столкновений
        if (snake1_hits_snake2_body && snake2_hits_snake1_body) {
            result = DRAW;
            gameOver = true;
            return;
        } else if (snake2_hits_snake1_body) {
            result = SNAKE1_WON;
            gameOver = true;
            return;
        } else if (snake1_hits_snake2_body) {
            result = SNAKE2_WON;
            gameOver = true;
            return;
        } else if (heads_collide) {
            if (snake1->getSize() > snake2->getSize()) {
                result = SNAKE1_WON;
            } else if (snake2->getSize() > snake1->getSize()) {
                result = SNAKE2_WON;
            } else {
                result = DRAW;
            }
            gameOver = true;
            return;
        }

        //столкновение с собственным телом змейки 1
        if (snake1->checkSelfCollision()) {
            result = SNAKE2_WON;
            gameOver = true;
            return;
        }

        //столкновение с собственным телом змейки 2
        if (snake2->checkSelfCollision()) {
            result = SNAKE1_WON;
            gameOver = true;
            return;
        }

        //столкновение с препятствием змейки 1
        if (snake1->checkCollisionWithObstacle(obstacles)) {
            result = SNAKE2_WON; 
            gameOver = true;
            return;
        }

        //столкновение с препятствием змейки 2
        if (snake2->checkCollisionWithObstacle(obstacles)) {
            result = SNAKE1_WON;
            gameOver = true;
            return;
        }
        
        // Проверка на столкновение со стенами
        checkWallCollisions();
    }

    void checkWallCollisions() {
        if (snake1->getY() <= 2 || snake1->getY() >= max_y - 1 || snake1->getX() <= 1 || snake1->getX() >= max_x - 1) {
            result = SNAKE2_WON;
            gameOver = true;
            return;
        }
        if (snake2->getY() <= 2 || snake2->getY() >= max_y - 1 || snake2->getX() <= 1 || snake2->getX() >= max_x - 1) {
            result = SNAKE1_WON;
            gameOver = true;
            return;
        }

        for (auto& wall : walls) {
            if (snake1->getX() == wall.first && snake1->getY() == wall.second + 2) { 
                result = SNAKE2_WON;
                gameOver = true;
                return;
            }
            if (snake2->getX() == wall.first && snake2->getY() == wall.second + 2) { 
                result = SNAKE1_WON;
                gameOver = true;
                return;
            }
        }
    }

    //создание стен
    void createWalls() {
        for (int i = 1; i < max_x - 1; ++i) {
            walls.push_back({i, 1});
            walls.push_back({i, max_y - 2});
        }
        for (int i = 1; i < max_y - 1; ++i) {
            walls.push_back({1, i});
            walls.push_back({max_x - 2, i});
        }
    }

    //отображение стен
    void renderWalls() {
        for (const auto& wall : walls) {
            mvprintw(wall.second + 2, wall.first, "#");
        }
    }
};

int main() {
    Game game;
    game.init();
    game.run();
    game.end();
    return 0;
}
