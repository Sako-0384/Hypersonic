#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <list>
#include <map>
#include <array>

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
    char owner;
    Vector2i pos;

    Entity() {}

    Entity(int o, Vector2i p)
            : owner(o), pos(p) {}
};

class Player : public Entity {
public:
    float numBombs, remainingBombs, xpRange;

    Player() {}

    Player(int o, Vector2i p, float nBombs, float rBombs, float xpR)
            : Entity(o, p), numBombs(nBombs), remainingBombs(rBombs), xpRange(xpR) {}
};

class Bomb : public Entity {
public:
    int count, range;

    Bomb() {}

    Bomb(int o, Vector2i p, int c, int r)
            : Entity(o, p), count(c), range(r) {}
};

class Item : public Entity {
public:
    int type;

    Item() {}

    Item(Vector2i p, int t)
            : Entity(0, p), type(t) {}
};

class Field {
public:
    char height, width;
    char field[11][13];

    const static char Empty = 0;
    const static char ItemBox1 = 1;
    const static char ItemBox2 = 2;
    const static char EmptyBox = 3;
    const static char Wall = 4;

    Field() : height(11),
              width(13) {}

    void setType(const Vector2i &v, int s) {
        field[v.y][v.x] = (char) s;
    }

    void setType(const int x, const int y, const int s) {
        field[y][x] = (char) s;
    }

    int getType(const Vector2i &v) const {
        return (field[v.y][v.x]);
    }

    int getType(int x, int y) const {
        return (field[y][x]);
    }
};

class State {
public:
    State() : dead(false) {
    }

    int myId;
    Field f;
    array<Bomb, 24> bombs;
    array<Item, 64> items;

    int bs, is;

    Player players[2];
    int boxesBroke;
    bool dead;

    array<Bomb, 16>::iterator findBomb(int x, int y) const {
        int i=0;
        auto it = bombs.begin();
        for (; i<bs; i++) {
            if ((it+i)->pos.x == x && (it+i)->pos.y == y) return (array<Bomb, 16>::iterator)(it+i);
        }

        return (array<Bomb, 16>::iterator)bombs.end();
    }

    array<Item, 64>::iterator findItem(int x, int y) const {
        int i=0;
        auto it = items.begin();
        for (; i<is; i++) {
            if ((it+i)->pos.x == x && (it+i)->pos.y == y) return (array<Item, 64>::iterator)(it+i);
        }

        return (array<Item, 64>::iterator)items.end();
    }
};

double evalState(const State &s) {
    double res = 0;

    bool checked[11][13] = {};
    auto it = s.bombs.begin();
    for (int i=0; i<s.bs; i++, it++) {
        if (it->owner != s.myId) continue;
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j < it->range; j++) {
                if (it->pos.x + dx[i] * j < 0 || it->pos.x + dx[i] * j >= s.f.width
                    || it->pos.y + dy[i] * j < 0 || it->pos.y + dy[i] * j >= s.f.height)
                    break;
                else if (!checked[it->pos.x + dx[i] * j][it->pos.y + dy[i] * j] &&
                        (s.f.getType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) == Field::EmptyBox
                         ||s.f.getType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) == Field::ItemBox1
                         ||s.f.getType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) == Field::ItemBox2)) {
                    res += 8.0 * ((double)(8 - it->count + 1) / 4.0 + 6);
                    checked[it->pos.x + dx[i] * j][it->pos.y + dy[i] * j] = true;
                    break;
                } else if (s.findBomb(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) !=
                           s.bombs.end()) {
                    if (s.findBomb(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j)->owner
                        == s.myId) {
                        res += 4.0;
                    } else {
                        res -= 2.0;
                    }
                    break;
                } else if (s.findItem(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) !=
                           s.items.end()) {
                    res -= 0.3;
                    break;
                }
            }
        }

        //cerr << it->pos.x << " " << it->pos.y << ": " << res << endl;
    }


    /*
    cerr << res << " "
         << s.boxesBroke << " "
         << s.players[s.myId].numBombs << " "
         << s.players[s.myId].xpRange << " "
         << (s.players[s.myId].numBombs - s.players[s.myId].remainingBombs)
         << endl;
    */


    res += (double) s.boxesBroke * 64.0;
    res += (double) s.players[s.myId].numBombs * 1.0;
    res += (double) s.players[s.myId].xpRange * 1.0;
    res += (double) (s.players[s.myId].numBombs - s.players[s.myId].remainingBombs) * 0.2;

    for (int i = 0; i < s.f.height; i++) {
        for (int j = 0; j < s.f.width; j++) {
            if (s.f.getType(j, i) == Field::EmptyBox
                ||s.f.getType(j, i) == Field::ItemBox1
                ||s.f.getType(j, i) == Field::ItemBox2) {
                res -= (abs(j - s.players[s.myId].pos.x) + abs(i - s.players[s.myId].pos.y)) * 0.2;
            }
        }
    }

    //cerr << res << endl;

    return res;
};

class StateComparator {
public:
    bool operator()(const pair<State *, int> &l, const pair<State *, int> &r) const {
        if (r.first->dead&&l.first->dead) return false;
        if (r.first->dead) return true;
        if (l.first->dead) return false;
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
    queue< pair<int, int> > blowQ;
    for (auto it = res->bombs.begin(); it != res->bombs.end(); it++) {
        it->count--;
        if (it->count <= 0) {
            blowQ.push(make_pair(it->pos.x, it->pos.y));
        }
    }

    while (!blowQ.empty()) {
        auto it = res->findBomb(blowQ.front().first, blowQ.front().second);
        blowQ.pop();
        if (it == res->bombs.end()) continue;
        for (int i = 0; i < 4; i++) {
            for (int j = 1; j < it->range; j++) {
                if (it->pos.x + dx[i] * j < 0 || it->pos.x + dx[i] * j >= res->f.width
                    || it->pos.y + dy[i] * j < 0 || it->pos.y + dy[i] * j >= res->f.height)
                    break;
                else if (res->players[res->myId].pos.x == it->pos.x + dx[i] * j && res->players[res->myId].pos.y == it->pos.y + dy[i] * j ) {
                    res->dead = true;
                    break;
                } else if (res->f.getType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) == Field::EmptyBox) {
                    res->f.setType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j, Field::Empty);
                    if (it->owner == s.myId) res->boxesBroke++;
                    break;
                } else if (res->f.getType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) == Field::ItemBox1) {
                    res->f.setType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j, Field::Empty);
                    res->items[res->is++] = (Item(Vector2i(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j), Field::ItemBox1));
                    if (it->owner == s.myId) res->boxesBroke++;
                    break;
                } else if (res->f.getType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) == Field::ItemBox2) {
                    res->f.setType(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j, Field::Empty);
                    res->items[res->is++] = (Item(Vector2i(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j), Field::ItemBox2));
                    if (it->owner == s.myId) res->boxesBroke++;
                    break;
                } else if (res->findBomb(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) !=
                           res->bombs.end()) {
                    blowQ.push(make_pair(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j));
                    break;
                } else if (res->findItem(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j) !=
                           res->items.end()) {
                    copy(res->findItem(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j)+1, res->items.end(), res->findItem(it->pos.x + dx[i] * j, it->pos.y + dy[i] * j));
                    res->is--;
                    break;
                }
            }
        }
        copy(it+1, res->bombs.end(), it);
        res->bs--;
        //res->bombs.erase(it);
        res->players[res->myId].remainingBombs += 1.0;
    }
    return res;
}

State *act(const State &s, char actionId) {
    if (actionId >= 5 && s.players[s.myId].remainingBombs < 1.0) return nullptr;
    if (actions[actionId].dir < 4
        && (s.players[s.myId].pos.x + dx[actions[actionId].dir] < 0
            || s.players[s.myId].pos.x + dx[actions[actionId].dir] >= 13
            || s.players[s.myId].pos.y + dy[actions[actionId].dir] < 0
            || s.players[s.myId].pos.y + dy[actions[actionId].dir] >= 11
            || s.f.getType(s.players[s.myId].pos.x + dx[actions[actionId].dir],
                           s.players[s.myId].pos.y + dy[actions[actionId].dir]) != Field::Empty)) {
        return nullptr;
    }

    State *res = new State(s);

    if (actions[actionId].place) {
        res->bombs[res->bs++]=(Bomb(res->myId,
                                  Vector2i(res->players[res->myId].pos.x, res->players[res->myId].pos.y),
                                  8,
                                  (int) res->players[res->myId].xpRange));
        res->players[res->myId].remainingBombs -= 1.0;
    }

    if (actions[actionId].dir < 4) {
        res->players[res->myId].pos.x += dx[actions[actionId].dir];
        res->players[res->myId].pos.y += dy[actions[actionId].dir];
        if (res->findItem(res->players[res->myId].pos.x, res->players[res->myId].pos.y) !=
            res->items.end()) {
            auto it = res->findItem(res->players[res->myId].pos.x, res->players[res->myId].pos.y);
            switch (it->type) {
                case 1:
                    res->players[res->myId].xpRange += 1.0;
                    break;
                case 2:
                    res->players[res->myId].numBombs += 1.0;
                    res->players[res->myId].remainingBombs += 1.0;
                    break;
            }
            copy(it+1, res->items.end(), it);
            res->is--;
            //res->items.erase(it);
        }
    }

    return res;
}

int main() {
    clock_t start, el1, el2, el3, end;
    double elS1, elS2, elS3, elS4;
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId;
    cin.ignore();

    State current;

    current.myId = myId;


    // game loop
    while (1) {
        deque<pair<State *, int> > beam[21];

        current.bs=0;
        current.is=0;
        current.boxesBroke = 0;
        for (int i = 0; i < height; i++) {
            string row;
            cin >> row;
            cin.ignore();
            for (int j = 0; j < width; j++) {
                switch (row[j]) {
                    case '.':
                        current.f.setType(j, i, Field::Empty);
                        break;
                    case '0':
                        current.f.setType(j, i, Field::EmptyBox);
                        break;
                    case '1':
                        current.f.setType(j, i, Field::ItemBox1);
                        break;
                    case '2':
                        current.f.setType(j, i, Field::ItemBox2);
                        break;
                }
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
                    current.players[owner] = Player(owner, Vector2i(x, y), param1, param1, param2);
                    break;
                case 1:
                    current.bombs[current.bs++]=(Bomb(owner, Vector2i(x, y), param1, param2));
                    if (owner == myId) current.players[myId].numBombs += 1.0;
                    break;
                case 2:
                    current.items[current.is++]=(Item(Vector2i(x, y), param1));
                    break;
            }
        }

        State *blownUp = blow(current);

        State *nextState;

        for (char i = 0; i < 10; i++) {
            if ((nextState = act(*blownUp, i)) != nullptr)
                beam[0].insert(lower_bound(beam[0].begin(),
                                           beam[0].end(),
                                           make_pair(nextState, i),
                                           StateComparator()),
                               make_pair(nextState, i));
        }

        delete blownUp;

        elS1 = 0;
        elS2 = 0;
        elS3 = 0;
        elS4 = 0;

        for (int i = 0; i < 9 - 1; i++) {
            if (beam[i].size() > 18) {
                for (auto it = beam[i].begin() + 18; it != beam[i].end(); it++)
                    delete it->first;
                beam[i].erase(beam[i].begin() + 18, beam[i].end());
            }

            while (!beam[i].empty()) {
                start = clock();
                pair<State *, int> st = beam[i].front();
                beam[i].pop_front();

                el1 = clock();

                blownUp = blow(*(st.first));

                el2 = clock();

                for (char j = 0; j < 10; j++) {
                    if ((nextState = act(*blownUp, j)) != nullptr)
                        beam[i + 1].insert(
                                lower_bound(beam[i + 1].begin(), beam[i + 1].end(), make_pair(nextState, st.second),
                                            StateComparator()), make_pair(nextState, st.second));
                }

                el3 = clock();

                delete blownUp;
                delete st.first;

                end = clock();

                elS1 += (double)(el1-start)/CLOCKS_PER_SEC;
                elS2 += (double)(el2-el1)/CLOCKS_PER_SEC;
                elS3 += (double)(el3-el2)/CLOCKS_PER_SEC;
                elS4 += (double)(end-el3)/CLOCKS_PER_SEC;
            }
        }

        cerr << elS1 << " "
             << elS2 << " "
             << elS3 << " "
             << elS4 << " "
             << endl;

        int sol[10] = {};

        for (int i = 0; i < 10; i++)
            sol[i] = 0;

        while (!beam[8].empty()) {
            auto item = beam[8].front();
            beam[8].pop_front();
            cerr << item.first->dead << endl;
            sol[item.second] += evalState(*(item.first));
        }

        int ans = 0;

        for (int i = 0; i < 10; i++) {
            if (sol[ans] < sol[i])
                ans = i;
            //cerr << sol[i] << endl;
        }

        cout << (actions[ans].place ? "BOMB " : "MOVE ")
             << current.players[myId].pos.x + ((ans % 5 != 4) ? dx[actions[ans].dir] : 0)
             << " "
             << current.players[myId].pos.y + ((ans % 5 != 4) ? dy[actions[ans].dir] : 0)
             << endl;
    }
}