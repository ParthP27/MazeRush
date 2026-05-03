#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <random>
#include <algorithm>
#include <iostream>

using namespace std;

const int ROWS = 21;
const int COLS = 31;
const int TILE = 24;

enum Cell {
    WALL,
    PATH
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

bool inside(int r, int c) {
    return r > 0 && r < ROWS - 1 && c > 0 && c < COLS - 1;
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

    vector<Point> path;
    Point cur = goal;

    while (!(cur == start) && cur.r != -1) {
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

int main() {
    generateMaze(1, 1);
    maze[ROWS - 2][COLS - 2] = PATH;

    sf::RenderWindow window(
        sf::VideoMode({COLS * TILE, ROWS * TILE}),
        "Maze Rush Arena - AI BFS"
    );

    Point player{1, 1};
    Point enemy{ROWS - 2, COLS - 2};
    Point goal{ROWS - 2, COLS - 2};

    sf::Clock enemyClock;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
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

        if (enemyClock.getElapsedTime().asMilliseconds() > 300) {
            vector<Point> path = bfs(enemy, player);
            if (!path.empty()) {
                enemy = path[0];
            }
            enemyClock.restart();
        }

        if (player == enemy) {
            cout << "Game Over! Enemy caught you.\n";
            window.close();
        }

        if (player == goal) {
            cout << "You Win!\n";
            window.close();
        }

        window.clear();

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                sf::RectangleShape tile({TILE - 1.f, TILE - 1.f});
                tile.setPosition({float(c * TILE), float(r * TILE)});

                if (maze[r][c] == WALL)
                    tile.setFillColor(sf::Color(40, 40, 40));
                else
                    tile.setFillColor(sf::Color(220, 220, 220));

                window.draw(tile);
            }
        }

        sf::RectangleShape playerShape({TILE - 4.f, TILE - 4.f});
        playerShape.setPosition({float(player.c * TILE + 2), float(player.r * TILE + 2)});
        playerShape.setFillColor(sf::Color::Blue);
        window.draw(playerShape);

        sf::RectangleShape enemyShape({TILE - 4.f, TILE - 4.f});
        enemyShape.setPosition({float(enemy.c * TILE + 2), float(enemy.r * TILE + 2)});
        enemyShape.setFillColor(sf::Color::Red);
        window.draw(enemyShape);

        sf::RectangleShape goalShape({TILE - 4.f, TILE - 4.f});
        goalShape.setPosition({float(goal.c * TILE + 2), float(goal.r * TILE + 2)});
        goalShape.setFillColor(sf::Color::Green);
        window.draw(goalShape);

        window.display();
    }

    return 0;
}
