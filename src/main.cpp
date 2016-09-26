#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <list>
#include <map>

using namespace std;

const int dx[4] = {1, 0, -1, 0};
const int dy[4] = {0, 1, 0, -1};

class Vector2i {
public:
    int x, y;

    Vector2i(int ix = 0, int iy = 0)
            : x(ix), y(iy) {}

    Vector2i operator+(const Vector2i &v) {
        return Vector2i(x + v.x, y + v.y);
    }
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
    float numBombs, remainingBombs, xpRange;

    Player(int o, Vector2i p, float nBombs, float rBombs, float xpR)
            : Entity(o, p), numBombs(nBombs), remainingBombs(rBombs), xpRange(xpR) {}
};

class Bomb : public Entity {
public:
    int count, range;

    Bomb(int o, Vector2i p, int c, int r)
            : Entity(o, p), count(c), range(r) {}
};

class Item : public Entity {
public:
    int type;

    Item(Vector2i p, int t)
            : Entity(0, p), type(t) {}
};

class Field {
public:
    int height, width;
    int field[11];

    const static int Empty = 0;
    const static int Box = 1;
    const static int Wall = 2;

    Field() : height(11),
              width(13) {}

    void setType(const Vector2i &v, int s) {
        field[v.y] = field[v.y] | (3 << (2 * v.x)) ^ ((3 - s) << (2 * v.x));
    }

    int setType(const int x, const int y, const int s) {
        field[y] = field[y] | (3 << (2 * x)) ^ ((3 - s) << (2 * x));
    }

    int getType(const Vector2i &v) {
        return (field[v.y] >> (2 * v.x) % 4);
    }

    int getType(int x, int y) {
        return (field[y] >> (2 * x) % 4);
    }
};

class State {
public:
    int myId;
    Field f;
    map<pair<int, int>, Bomb> bombs;
    map<pair<int, int>, Item> items;
    Player players[2];
    int boxesBroke;
};

double evalState(const State &s) {
    return s.boxesBroke;
};

class StateComparator {
public:
    bool operator()(const pair<State *, int> &l, const pair<State *, int> &r) const {
        return evalState(*(l.first)) > evalState(*(r.first));
    }
};

class Action {
public:
    char dir;
    bool place;

    Action(char d, bool p)
            : dir(d), place(p) {}
};

const Action actions[10] = {
        Action(0, false), Action(1, false), Action(2, false), Action(3, false), Action(4, false),
        Action(0, true), Action(1, true), Action(2, true), Action(3, true), Action(4, true)
};

State *blow(const State &s) {
    State *res = new State(s);
    queue < pair<int, int> > blowQ;
    for (auto it = res->bombs.begin(); it != res->bombs.end();it++) {
        it->second.count--;
        if (it->second.count <= 0) {
            blowQ.push(it->first);
        }
    }

    while (!blowQ.empty()) {
        auto it = res->bombs.find(blowQ.front());
        blowQ.pop();
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j < it->second.range; j++) {
                if (it->first.first + dx[i] * j < 0 || it->first.first + dx[i] * j >= res->f.width
                    || it->first.second + dy[i] * j < 0 || it->first.second + dy[i] * j >= res->f.height)
                    break;
                else if (res->f.getType(it->first.first + dx[i] * j, it->first.second + dx[i] * j) == Field::Box) {
                    res->f.setType(it->first.first + dx[i] * j, it->first.second + dx[i] * j, Field::Empty);
                    res->items[make_pair(it->first.first + dx[i] * j, it->first.second + dx[i] * j)]
                            = Item(Vector2i(it->first.first + dx[i] * j, it->first.second + dx[i] * j),
                                   3);
                    break;
                } else if (res->bombs.find(make_pair(it->first.first + dx[i] * j, it->first.second + dx[i] * j)) !=
                           res->bombs.end()) {
                    blowQ.push(make_pair(it->first.first + dx[i] * j, it->first.second + dx[i] * j));
                    break;
                } else if (res->items.find(make_pair(it->first.first + dx[i] * j, it->first.second + dx[i] * j)) !=
                           res->items.end()) {
                    res->items.erase(make_pair(it->first.first + dx[i] * j, it->first.second + dx[i] * j));
                    break;
                }
            }
        }
        res->bombs.erase(it);
    }
    return res;
}

State *act(const State &s, char actionId) {
    if (actionId >= 5 && s.players[s.myId].remainingBombs <= 0) return nullptr;
    if (actions[actionId].dir < 4
        && (s.players[s.myId].pos.x + dx[actions[actionId].dir] < 0
            || s.players[s.myId].pos.x + dx[actions[actionId].dir] >= 13
            || s.players[s.myId].pos.y + dy[actions[actionId].dir] < 0
            || s.players[s.myId].pos.y + dy[actions[actionId].dir] >= 11
            || s.f.getType(s.players[s.myId].pos.x + dx[actions[actionId].dir],
                       s.players[s.myId].pos.y + dy[actions[actionId].dir]) != Field::Empty))
        return nullptr;

    State *res = new State(s);

    if (actions[actionId].place) {
        res->bombs[make_pair(res->players[res->myId].pos.x, res->players[res->myId].pos.y)]
                = Bomb(res->myId,
                       Vector2i(res->players[res->myId].pos.x, res->players[res->myId].pos.y),
                       8,
                       (int)res->players[res->myId].xpRange);
    }

    if (actions[actionId].dir < 4) {
        res->players[res->myId].pos.x += dx[actions[actionId].dir];
        res->players[res->myId].pos.y += dy[actions[actionId].dir];
        if (res->items.find(make_pair(res->players[res->myId].pos.x, res->players[res->myId].pos.y))!=res->items.end()) {
            auto it = res->items.find(make_pair(res->players[res->myId].pos.x, res->players[res->myId].pos.y));
            switch(it->second.type) {
                case 1:
                    res->players[res->myId].xpRange+=1.0;
                    break;
                case 2:
                    res->players[res->myId].numBombs+=1.0;
                    res->players[res->myId].remainingBombs+=1.0;
                    break;
                case 3:
                    res->players[res->myId].xpRange+=0.5;
                    res->players[res->myId].numBombs+=0.5;
                    res->players[res->myId].remainingBombs+=0.5;
                    break;
            }
            res->items.erase(it);
        }
    }

    return res;
}

int main() {
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId;
    cin.ignore();

    deque<pair<State *, int> > beam[50];

    State current;

    current.myId = myId;


    // game loop
    while (1) {
        current.bombs.clear();
        current.items.clear();
        for (int i = 0; i < height; i++) {
            string row;
            cin >> row;
            cin.ignore();
            for (int j = 0; j < width; j++) {
                current.f.setType(Vector2i(j, i), (row[j] == '0') ? Field::Box : Field::Empty);
            }
        }
        int entities;
        cin >> entities;
        cin.ignore();
        for (int i = 0; i < entities; i++) {
            int entityType;
            int owner;
            int x;
            int y;
            int param1;
            int param2;
            cin >> entityType >> owner >> x >> y >> param1 >> param2;
            cin.ignore();

            switch (entityType) {
                case 0:
                    current.players[owner] = Player(owner, Vector2i(x, y), 0, param1, param2);
                    break;
                case 1:
                    current.bombs[make_pair(x, y)] = Bomb(owner, Vector2i(x, y), param1, param2);
                    break;
                case 2:
                    current.items[make_pair(x, y)] = Item(Vector2i(x, y), param1);
                    break;
            }
        }

        State *blownUp = blow(current);

        State *nextState;

        for (char i = 0; i < 10; i++) {
            if ((nextState = act(*blownUp, i)) != nullptr)
                beam[0].insert(lower_bound(beam[0].begin(), beam[0].end(), make_pair(nextState, i), StateComparator()), make_pair(nextState, i));
        }

        delete blownUp;

        for (int i=0;i<50-1;i++) {
            if (beam[i].size()>100) {
                beam[i].erase(beam[i].begin()+100, beam[i].end());
            }
            while(!beam[i].empty()) {
                pair<State *, int> st = beam[i].front();
                beam[i].pop_front();

                blownUp = blow(*(st.first));

                for (char j = 0; j < 10; j++) {
                    if ((nextState = act(*blownUp, j)) != nullptr)
                        beam[i + 1].insert(
                                lower_bound(beam[i + 1].begin(), beam[i + 1].end(), make_pair(nextState, st.second),
                                            StateComparator()), make_pair(nextState, st.second));
                }

                delete blownUp;
                delete st.first;
            }
        }

        int sol[10] = {};

        for (auto it = beam[49].begin();it!=beam[49].end();it++) {
            sol[it->second] += evalState(*(it->first));
        }

        int ans = 0;

        for (int i = 0;i<10;i++) {
            if (sol[ans]<sol[i])
                ans = i;
        }

        cout << (actions[ans].place?"BOMB ":"MOVE ")
             << current.players[myId].pos.x + dx[actions[ans].dir]
             << " "
             << current.players[myId].pos.y + dy[actions[ans].dir]
             << endl;
    }
}