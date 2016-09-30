#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <queue>
#include <list>
#include <map>
#include <array>
#include <memory>
#include <cmath>
#include <cstdlib>

const int BEAM_DEPTH = 16;
const int BEAM_WIDTH = 75;

using namespace std;

const int dx[4] = {1, 0, -1, 0};
const int dy[4] = {0, 1, 0, -1};

clock_t start, el1, el2, el3, cend;
double elS1, elS2, elS3, elS4;

int qcnt;

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
    bool dead;

    Player() : dead(true) {}

    Player(int o, Vector2i p, float nBombs, float rBombs, float xpR)
            : Entity(o, p), numBombs(nBombs), remainingBombs(rBombs), xpRange(xpR), dead(false) {}
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

class BombArray {
public:
    BombArray() : s(0) {}

    array<Bomb, 24> bombs;
    int s;

    array<Bomb, 24>::iterator find(int x, int y) const {
        int i = 0;
        auto it = bombs.begin();
        for (; i < s; i++) {
            if ((it + i)->pos.x == x && (it + i)->pos.y == y) return (array<Bomb, 24>::iterator) (it + i);
        }

        return (array<Bomb, 24>::iterator) end();
    }

    class ByCount {
    public:
        bool operator()(const Bomb &l, const Bomb &r) {
            return l.count < r.count;
        }
    };

    array<Bomb, 24>::iterator begin() const {
        return (array<Bomb, 24>::iterator) (bombs.begin());
    }

    array<Bomb, 24>::iterator end() const {
        return (array<Bomb, 24>::iterator) (bombs.begin() + s);
    }

    void sort() {
        std::sort(bombs.begin(), bombs.begin() + s, BombArray::ByCount());
    }

    void push(const Bomb &a) {
        bombs[s] = a;
        s++;
    }

    void erase(array<Bomb, 24>::iterator it) {
        copy(it + 1, bombs.end(), it);
        s--;
    }
};

class ItemArray {
public:
    ItemArray() : s(0) {}

    array<Item, 64> items;
    int s;

    array<Item, 64>::iterator begin() const {
        return (array<Item, 64>::iterator) (items.begin());
    }

    array<Item, 64>::iterator end() const {
        return (array<Item, 64>::iterator) (items.begin() + s);
    }

    array<Item, 64>::iterator find(int x, int y) const {
        int i = 0;
        auto it = items.begin();
        for (; i < s; i++) {
            if ((it + i)->pos.x == x && (it + i)->pos.y == y) return (array<Item, 64>::iterator) (it + i);
        }

        return (array<Item, 64>::iterator) end();
    }

    void push(const Item &a) {
        items[s++] = a;
    }

    void erase(array<Item, 64>::iterator it) {
        copy(it + 1, end(), it);
        s--;
    }
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

class XpCells {
public:
    char height, width;
    int field[11][13];
    int blown[4][9];

    XpCells() : height(11), width(13) {}

    void set(int x, int y, int c, bool b) {
        if (c < 0 || c > 8) return;

        if (b) {
            field[y][x] |= (1 << c);
        } else {
            field[y][x] &= ~(1 << c);
        }

        return;
    }

    bool get(int x, int y, int c) const {
        if (c < 0 || c > 8) return false;

        return (field[y][x] & (1 << c)) != 0;
    }

    bool blownBefore(int x, int y, int c) const {
        return (field[y][x] & ((1 << c) - 1)) != 0;
    }

    bool blownAfter(int x, int y, int c) const {
        return (field[y][x] & ~((1 << (c + 1)) - 1)) != 0;
    }

    int lastBlow(int x, int y) const {
        for (int i = 8; i >= 0; i--)
            if ((field[y][x] & (1 << i)) != 0) return i;

        return -1;
    }

    int firstBlow(int x, int y) const {
        for (int i = 0; i < 9; i++)
            if ((field[y][x] & (1 << i)) != 0) return i;

        return -1;
    }

    int nextBlow(int x, int y, int c) const {
        for (int i = c + 1; i < 9; i++)
            if ((field[y][x] & (1 << i)) != 0) return i;

        return -1;
    }

    int prevBlow(int x, int y, int c) const {
        for (int i = c - 1; i >= 0; i--)
            if ((field[y][x] & (1 << i)) != 0) return i;

        return -1;
    }

    void update(BombArray *ba, ItemArray *ia, const Field &f) {
        memset(field, 0, sizeof(field));
        memset(blown, 0, sizeof(blown));

        ba->sort();

        auto itr = ba->begin();

        queue<array<Bomb, 24>::iterator> q;

        for (int counts = 0; counts < 9; counts++) {
            for (; itr != ba->end() && itr->count <= counts; itr++) {
                if (itr->count == counts) q.push(itr);
            }

            while (!q.empty()) {
                auto it = q.front();
                q.pop();

                int c = it->count;

                set(it->pos.x, it->pos.y, c, true);
                for (int i = 0; i < 4; i++) {
                    for (int r = 1; r < it->range; r++) {
                        int x = it->pos.x + dx[i] * r, y = it->pos.y + dy[i] * r;
                        if (x < 0 || x >= width
                            || y < 0 || y >= height
                            || f.getType(it->pos) == Field::Wall) {
                            break;
                        } else {
                            set(x, y, c, true);
                            if (f.getType(x, y) != Field::Empty
                                && !blownBefore(x, y, c)) {
                                blown[it->owner][c]++;
                                break;
                            } else if (ia->find(x, y) != ia->end()
                                && !blownBefore(x, y, c)) {
                                break;
                            } else if (ba->find(x, y) != ba->end()
                                       && ba->find(x, y)->count > c) {
                                ba->find(x, y)->count = c;
                                q.push(ba->find(x, y));
                                break;
                            } else if (ba->find(x, y) != ba->end()
                                       && ba->find(x, y)->count == c) {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
};

class State {
public:
    State() : dead(false), boxesBroke(0) {
    }

    int myId;
    Field f;
    XpCells xpc;
    BombArray ba;
    ItemArray ia;

    int bs, is;

    Player players[4];
    int boxesBroke;
    bool dead;
    double value;
    double bombScore;
    int reachableCells;
    int evade;

    bool canOccupy(int x, int y, int round) const {
        if (x < 0 || x >= f.width || y < 0 || y >= f.height) return false;
        if (f.getType(x, y) != Field::Empty) return false;
        return !(ba.find(x, y) != ba.end() && ba.find(x, y)->count > round);
    }

    void updateBombScore() {
        double res = 0;
        for (int i=0;i<9;i++) {
            res += (double)xpc.blown[myId][i] * (double)xpc.blown[myId][i] * (8-i);
        }

        bombScore = res;
    }
};

class StateEvaluator {
public:
    static int calcEvade(const State &s, int player) {
        int res = 0;

        queue< pair< int, pair<int, int> > > q;
        q.push(make_pair(0, make_pair(s.players[player].pos.x,s.players[player].pos.y)));

        while(!q.empty()) {
            auto p = q.front();
            q.pop();

            if (s.xpc.get(p.second.first, p.second.second, p.first)) {
                continue;
            }

            if (p.first>s.xpc.lastBlow(p.second.first, p.second.second)) {
                cerr << "    " << p.second.first << " " << p.second.second << endl;
                res = 1;
                break;
            }

            for (int i=0;i<4;i++) {
                int nx = p.second.first + dx[i], ny = p.second.second + dy[i];
                if(s.canOccupy(nx, ny, p.first+1) && (!s.xpc.get(nx, ny, p.first+1)))
                    q.push(make_pair(p.first+1, make_pair(nx, ny)));
            }

            if (!s.xpc.get(p.second.first, p.second.second, p.first+1))
                q.push(make_pair(p.first+1, p.second));
        }

        return res;
    }

    /*
    static int calcEvade(const State &s, int player) {
        int res = 0;

        int memo[11][13] = {};
        memset(memo, 0xff, sizeof(memo));

        deque<pair<int, pair<int, int> > > q;
        q.push_back(make_pair(0, make_pair(s.players[player].pos.x, s.players[player].pos.y)));

        //memo[s.players[player].pos.y][s.players[player].pos.x] = 0;

        while (!q.empty()) {
            qcnt++;
            auto p = q.front();
            q.pop_front();

            if (!s.canOccupy(p.second.first, p.second.second, p.first) ||
                s.xpc.get(p.second.first, p.second.second, p.first)) {
                continue;
            }

            if (memo[p.second.second][p.second.first] ==
                max(0, s.xpc.prevBlow(p.second.first, p.second.second, p.first)))
                continue;
            else
                memo[p.second.second][p.second.first] = max(0,
                                                            s.xpc.prevBlow(p.second.first, p.second.second, p.first));

            if (s.xpc.nextBlow(p.second.first, p.second.second, p.first) == -1) {
                res = 1;
                //cerr << "evade: " << p.second.first << " " << p.second.second << " " << p.first << endl;
                break;
            }

            for (int i = 0; i < 4; i++) {
                int nx = p.second.first + dx[i], ny = p.second.second + dy[i];
                for (int n = p.first;
                     n != -1 && n < s.xpc.nextBlow(p.second.first, p.second.second, p.first); n = s.xpc.nextBlow(nx, ny,
                                                                                                                 n)) {
                    q.insert(lower_bound(q.begin(), q.end(), make_pair(n + 1, make_pair(nx, ny)),
                                         greater<pair<int, pair<int, int> > >()), make_pair(n + 1, make_pair(nx, ny)));
                }
            }
        }

        return res;
    }
     */

    static double evaluate(const State &s) {
        if (s.players[s.myId].dead || s.evade == 0) return 0;
        double res = 0;
        res += s.bombScore * 100;
        res += s.boxesBroke * s.boxesBroke * 800;
        res += (double) s.evade * 10.0;
        //res += (double)s.reachableCells * 10.0;
        res += s.players[s.myId].xpRange * 900.0 + s.players[s.myId].numBombs * 1200.0;
        for (int i = 0; i < 4; i++) {
            if (i != s.myId) res += (11.0 - abs(s.players[i].pos.x - 6) - abs(s.players[i].pos.y - 5)) * 100.0;
            else res += (abs(s.players[i].pos.x - 6) + abs(s.players[i].pos.y - 5)) * 40.0;
        }
        return res;
    }
};

class StateComparator {
public:
    bool operator()(const pair<State *, int> &l, const pair<State *, int> &r) const {
        if (abs(StateEvaluator::evaluate(*l.first) - StateEvaluator::evaluate(*r.first)) < 0.01) {
            if ((l.second > 4 && r.second > 4) || (l.second < 5 && r.second < 5)) return l.second < r.second;
            return rand() % 2 == 0;
        }
        return StateEvaluator::evaluate(*l.first) > StateEvaluator::evaluate(*r.first);
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

    for (auto it = res->ba.begin(); it != res->ba.end(); it++) {
        it->count--;
    }

    res->xpc.update(&(res->ba), &(res->ia), (res->f));

    for (int y = 0; y < 13; y++) {
        for (int x = 0; x < 11; x++) {
            if (res->xpc.get(x, y, 0)) {
                if (res->f.getType(x, y) == Field::EmptyBox) {
                    res->f.setType(x, y, Field::Empty);
                    res->boxesBroke++;
                } else if (res->f.getType(x, y) == Field::ItemBox1) {
                    res->f.setType(x, y, Field::Empty);
                    res->ia.push(Item(Vector2i(x, y), 1));
                    res->boxesBroke++;
                } else if (res->f.getType(x, y) == Field::ItemBox2) {
                    res->f.setType(x, y, Field::Empty);
                    res->ia.push(Item(Vector2i(x, y), 2));
                    res->boxesBroke++;
                } else if (res->ia.find(x, y) != res->ia.end()) {
                    res->ia.erase(res->ia.find(x, y));
                } else if (res->ba.find(x, y) != res->ba.end()) {
                    res->players[res->ba.find(x, y)->owner].remainingBombs += 1.0;
                    res->ba.erase(res->ba.find(x, y));
                }

                for (int i=0;i<4;i++)
                    if (res->players[i].pos.x == x && res->players[i].pos.y == y) {
                        res->players[i].dead = true;
                    }
            }
        }
    }

    return res;
}

State *act(const State &s, char actionId) {
    if (actionId >= 5 && (s.players[s.myId].remainingBombs < 1.0
                          || s.ba.find(s.players[s.myId].pos.x, s.players[s.myId].pos.y) != s.ba.end()))
        return nullptr;
    if (actions[actionId].dir < 4
        && !s.canOccupy(s.players[s.myId].pos.x + dx[actions[actionId].dir],
                        s.players[s.myId].pos.y + dy[actions[actionId].dir], 0)) {
        return nullptr;
    }

    start = clock();

    State *res = new State(s);

    if (actions[actionId].place) {
        res->ba.push(Bomb(res->myId,
                          Vector2i(res->players[res->myId].pos.x, res->players[res->myId].pos.y),
                          8,
                          (int) res->players[res->myId].xpRange));
        res->players[res->myId].remainingBombs -= 1.0;
    }

    if (actions[actionId].dir < 4) {
        res->players[res->myId].pos.x += dx[actions[actionId].dir];
        res->players[res->myId].pos.y += dy[actions[actionId].dir];
        if (res->ia.find(res->players[res->myId].pos.x, res->players[res->myId].pos.y) !=
            res->ia.end()) {
            auto it = res->ia.find(res->players[res->myId].pos.x, res->players[res->myId].pos.y);
            switch (it->type) {
                case 1:
                    res->players[res->myId].xpRange += 1.0;
                    break;
                case 2:
                    res->players[res->myId].numBombs += 1.0;
                    res->players[res->myId].remainingBombs += 1.0;
                    break;
            }
            res->ia.erase(it);
        }
    }

    el1 = clock();

    State *del = res;

    res = blow(*res);

    delete del;

    el2 = clock();

    res->updateBombScore();

    el3 = clock();

    //res->updateEfficiency();

    res->evade = StateEvaluator::calcEvade(*res, res->myId);

    res->value = StateEvaluator::evaluate(*res);

    cend = clock();

    elS1 += (double) (el1 - start) / CLOCKS_PER_SEC;
    elS2 += (double) (el2 - el1) / CLOCKS_PER_SEC;
    elS3 += (double) (el3 - el2) / CLOCKS_PER_SEC;
    elS4 += (double) (cend - el3) / CLOCKS_PER_SEC;

    return res;
}

int main() {
    //clock_t start, el1, el2, el3, end;
    //double elS1, elS2, elS3, elS4;
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId;
    cin.ignore();

    State current;

    current.myId = myId;


    // game loop
    while (1) {
        qcnt = 0;

        deque<pair<State *, int> > beam[BEAM_DEPTH + 1];

        current.ba.s = 0;
        current.ia.s = 0;
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
                    case 'X':
                        current.f.setType(j, i, Field::Wall);
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
                    current.ba.push(Bomb(owner, Vector2i(x, y), param1, param2));
                    current.players[owner].numBombs += 1.0;
                    break;
                case 2:
                    current.ia.push(Item(Vector2i(x, y), param1));
                    break;
            }
        }

        State *nextState;

        State *b = blow(current);

        for (char i = 0; i < 10; i++) {
            if ((nextState = act(*b, i)) == nullptr) continue;
            cerr << (int)i << ": " << nextState->evade << endl;
            if (nextState->players[myId].dead || nextState->evade == 0) {
                delete nextState;
                continue;
            }
            beam[0].insert(lower_bound(beam[0].begin(),
                                       beam[0].end(),
                                       make_pair(nextState, i),
                                       StateComparator()),
                           make_pair(nextState, i));
        }

        delete b;

        elS1 = 0;
        elS2 = 0;
        elS3 = 0;
        elS4 = 0;

        for (int i = 0; i < BEAM_DEPTH; i++) {
            if (beam[i].size() > BEAM_WIDTH) {
                for (auto it = beam[i].begin() + BEAM_WIDTH; it != beam[i].end(); it++)
                    delete it->first;
                beam[i].erase(beam[i].begin() + BEAM_WIDTH, beam[i].end());
            }

            while (!beam[i].empty()) {
                pair<State *, int> st = beam[i].front();
                beam[i].pop_front();

                //blownUp = blow(*(st.first));

                for (char j = 0; j < 10; j++) {
                    if ((nextState = act(*(st.first), j)) != nullptr) {
                        if (nextState->players[myId].dead || nextState->evade == 0) {
                            delete nextState;
                            continue;
                        }
                        beam[i + 1].insert(
                                lower_bound(beam[i + 1].begin(), beam[i + 1].end(), make_pair(nextState, st.second),
                                            StateComparator()), make_pair(nextState, st.second));
                    }
                }

                delete st.first;
            }
        }

        cerr << elS1 << " "
             << elS2 << " "
             << elS3 << " "
             << elS4 << " "
             << endl;

        double sol[10] = {};
        int cnt[10] = {};

        for (int i = 0; i < 10; i++)
            sol[i] = cnt[i] = 0;

        double hoge = 1;
        while (!beam[BEAM_DEPTH].empty()) {
            auto item = beam[BEAM_DEPTH].front();
            beam[BEAM_DEPTH].pop_front();
            double evalResult = StateEvaluator::evaluate(*(item.first));
            sol[item.second] += evalResult * hoge;
            cnt[item.second]++;

            //cerr << item.second << " "
            //     << evalResult << endl;

            hoge *= 0.9;
        }

        int ans = 0;

        for (int i = 0; i < 10; i++) {
            if (sol[ans] / max(1, cnt[ans]) < sol[i] / max(1, cnt[i]))
                ans = i;
            cerr << cnt[i] << " " << sol[i] << endl;
        }

        cerr << "qcnt: " << qcnt << endl;
        current.xpc.update(&current.ba, &current.ia, current.f);
        cerr << (int)current.xpc.field[1][6] << endl;

        cout << (actions[ans].place ? "BOMB " : "MOVE ")
             << current.players[myId].pos.x + ((ans % 5 != 4) ? dx[actions[ans].dir] : 0)
             << " "
             << current.players[myId].pos.y + ((ans % 5 != 4) ? dy[actions[ans].dir] : 0)
             << endl;
    }
}