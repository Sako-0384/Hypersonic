#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

const int dx[4] = {1,0,-1,0};
const int dy[4] = {0,1,0,-1};

class Vector2i {
public:
    int x, y;

    Vector2i(int ix = 0, int iy = 0)
            : x(ix), y(iy) {}
};

class Entity {
public:
    int owner;
    Vector2i pos;

    Entity(int o, Vector2i p)
            : owner(o), pos(p) {}
};

class Player : public Entity {
public:
    Player(int o, Vector2i p)
            : Entity(o, p) {}
};

class Bomb : public Entity {
public:
    Bomb(int o, Vector2i p)
            : Entity(o, p) {}
};

class Item : public Entity {
public:
    Item(Vector2i p)
            : Entity(0, p) {}
};

class State {
public:

};

int main() {
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId; cin.ignore();

    // game loop
    while (1) {
        for (int i = 0; i < height; i++) {
            string row;
            cin >> row; cin.ignore();
        }
        int entities;
        cin >> entities; cin.ignore();
        for (int i = 0; i < entities; i++) {
            int entityType;
            int owner;
            int x;
            int y;
            int param1;
            int param2;
            cin >> entityType >> owner >> x >> y >> param1 >> param2; cin.ignore();
        }

        cout << "BOMB 6 5" << endl;
    }
}