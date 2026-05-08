#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <random>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

const int ROWS = 21;
const int COLS = 31;
const int TILE = 24;

enum Cell {
    WALL,
    PATH
};

enum GameState {
    PLAYING,
    WON,
    LOST
};

struct Point {
    int r, c;

    bool operator==(const Point& other) const {
        return r == other.r && c == other.c;
    }
};

vector<vector<int>> maze(ROWS, vector<int>(COLS, WALL));

vector<Point> directions = {
    {-2, 0}, {2, 0}, {0, -2}, {0, 2}
};

bool showHint = false;

bool inside(int r, int c) {
    return r > 0 && r < ROWS - 1 && c > 0 && c < COLS - 1;
}

void resetMaze() {
    maze.assign(ROWS, vector<int>(COLS, WALL));
}

void generateMaze(int r, int c) {
    maze[r][c] = PATH;

    random_device rd;
    mt19937 g(rd());
    shuffle(directions.begin(), directions.end(), g);

    for (auto d : directions) {
        int nr = r + d.r;
        int nc = c + d.c;

        if (inside(nr, nc) && maze[nr][nc] == WALL) {
            maze[r + d.r / 2][c + d.c / 2] = PATH;
            generateMaze(nr, nc);
        }
    }
}

void addExtraOpenings(int count) {
    int opened = 0;

    while (opened < count) {
        int r = 1 + rand() % (ROWS - 2);
        int c = 1 + rand() % (COLS - 2);

        if (maze[r][c] == WALL) {
            maze[r][c] = PATH;
            opened++;
        }
    }
}

vector<Point> bfs(Point start, Point goal) {
    vector<vector<bool>> visited(ROWS, vector<bool>(COLS, false));
    vector<vector<Point>> parent(ROWS, vector<Point>(COLS, {-1, -1}));

    queue<Point> q;
    q.push(start);
    visited[start.r][start.c] = true;

    vector<Point> moves = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    while (!q.empty()) {
        Point current = q.front();
        q.pop();

        if (current == goal) break;

        for (auto m : moves) {
            int nr = current.r + m.r;
            int nc = current.c + m.c;

            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
                !visited[nr][nc] && maze[nr][nc] == PATH) {
                visited[nr][nc] = true;
                parent[nr][nc] = current;
                q.push({nr, nc});
            }
        }
    }

    if (!visited[goal.r][goal.c]) {
        return {};
    }

    vector<Point> path;
    Point cur = goal;

    while (!(cur == start)) {
        path.push_back(cur);
        cur = parent[cur.r][cur.c];
    }

    reverse(path.begin(), path.end());
    return path;
}

bool canMove(Point p) {
    return p.r >= 0 && p.r < ROWS &&
           p.c >= 0 && p.c < COLS &&
           maze[p.r][p.c] == PATH;
}

void startLevel(Point& player, Point& enemy, Point& goal, GameState& state, int level) {
    resetMaze();
    generateMaze(1, 1);

    player = {1, 1};
    enemy = {ROWS - 2, 1};
    goal = {ROWS - 2, COLS - 2};

    maze[player.r][player.c] = PATH;
    maze[enemy.r][enemy.c] = PATH;
    maze[goal.r][goal.c] = PATH;

    int openings = 35 + level * 8;
    addExtraOpenings(openings);

    showHint = false;
    state = PLAYING;
}

int main() {
    srand(time(nullptr));

    sf::RenderWindow window(
        sf::VideoMode({COLS * TILE, ROWS * TILE}),
        "Maze Rush Arena - BFS AI"
    );

    Point player;
    Point enemy;
    Point goal;

    int level = 1;
    int score = 0;
    GameState state = PLAYING;

    startLevel(player, enemy, goal, state, level);

    sf::Clock enemyClock;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::H && state == PLAYING) {
                    showHint = !showHint;
                }

                if (key->scancode == sf::Keyboard::Scancode::R && state != PLAYING) {
                    if (state == WON) {
                        level++;
                        score += 100;
                    }

                    startLevel(player, enemy, goal, state, level);
                    enemyClock.restart();
                }

                if (state == PLAYING) {
                    Point next = player;

                    if (key->scancode == sf::Keyboard::Scancode::W) next.r--;
                    if (key->scancode == sf::Keyboard::Scancode::S) next.r++;
                    if (key->scancode == sf::Keyboard::Scancode::A) next.c--;
                    if (key->scancode == sf::Keyboard::Scancode::D) next.c++;

                    if (canMove(next)) {
                        player = next;
                    }
                }
            }
        }

        if (state == PLAYING) {
            int enemyDelay = max(190, 800 - level * 55);

            if (enemyClock.getElapsedTime().asMilliseconds() > enemyDelay) {
                vector<Point> enemyPath = bfs(enemy, player);

                if (!enemyPath.empty() && rand() % 5 != 0) {
                    enemy = enemyPath[0];
                }

                enemyClock.restart();
            }

            if (player == enemy) {
                cout << "Game Over! Press R to restart.\n";
                state = LOST;
            }

            if (player == goal) {
                cout << "You Win! Press R for next level.\n";
                state = WON;
            }
        }

        window.clear();

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                sf::RectangleShape tile({TILE - 1.f, TILE - 1.f});
                tile.setPosition({float(c * TILE), float(r * TILE)});

                if (maze[r][c] == WALL)
                    tile.setFillColor(sf::Color(35, 35, 35));
                else
                    tile.setFillColor(sf::Color(220, 220, 220));

                window.draw(tile);
            }
        }

        if (showHint && state == PLAYING) {
            vector<Point> hintPath = bfs(player, goal);

            for (Point p : hintPath) {
                sf::RectangleShape hintTile({TILE - 10.f, TILE - 10.f});
                hintTile.setPosition({float(p.c * TILE + 5), float(p.r * TILE + 5)});
                hintTile.setFillColor(sf::Color(255, 220, 0));
                window.draw(hintTile);
            }
        }

        sf::RectangleShape goalShape({TILE - 4.f, TILE - 4.f});
        goalShape.setPosition({float(goal.c * TILE + 2), float(goal.r * TILE + 2)});
        goalShape.setFillColor(sf::Color::Green);
        window.draw(goalShape);

        sf::RectangleShape playerShape({TILE - 4.f, TILE - 4.f});
        playerShape.setPosition({float(player.c * TILE + 2), float(player.r * TILE + 2)});
        playerShape.setFillColor(sf::Color::Blue);
        window.draw(playerShape);

        sf::RectangleShape enemyShape({TILE - 4.f, TILE - 4.f});
        enemyShape.setPosition({float(enemy.c * TILE + 2), float(enemy.r * TILE + 2)});
        enemyShape.setFillColor(sf::Color::Red);
        window.draw(enemyShape);

        if (state == WON || state == LOST) {
            sf::RectangleShape overlay({float(COLS * TILE), float(ROWS * TILE)});
            overlay.setFillColor(sf::Color(0, 0, 0, 130));
            window.draw(overlay);
        }

        window.display();
    }

    return 0;
}