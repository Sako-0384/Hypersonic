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
#include <random>
#include <cstdio>

const int BEAM_DEPTH = 17;
const int BEAM_WIDTH = 70;

using namespace std;

const int dx[4] = {1, 0, -1, 0};
const int dy[4] = {0, 1, 0, -1};

clock_t start, el1, el2, el3, cend;
double elS1, elS2, elS3, elS4;

int qcnt;


const char *ZDK[] = {"zun", "doko", "KI!", "YO!!", "SHI!!!"};

class ZunDokoKiyoshi {
public:
    ZunDokoKiyoshi() : r(114514), s(0) {}

    mt19937 r;
    int s;
    int zd;

    int doZDK() {
        if (s>=5) {
            if (s==7) {
                s = 0;
                return 7;
            } else {
                return s++;
            }
        } else {
            zd = r()%2;
            if (s<4&&zd==0) {
                return s++;
            } else if (s==4&&zd==1) {
                return s++;
            } else if (s==4&&zd==0) {
                s = 1;
                return s;
            } else {
                return s;
            }
        }
    }

    const char *getZDK() {
        int input = doZDK();
        if (input <= 4) {
            return ZDK[zd];
        } else {
            return ZDK[input - 3];
        }
    }
};

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
    int numBombs, remainingBombs, bombDelta, xpRange;
    bool dead, initialized;

    Player() : dead(true), initialized(false) {}

    Player(int o, Vector2i p, int nBombs, int rBombs, int xpR)
            : Entity(o, p), numBombs(nBombs), remainingBombs(rBombs), bombDelta(0), xpRange(xpR), dead(false), initialized(true) {}
};

class Bomb : public Entity {
public:
    int count, range;
    int ic;

    Bomb() {}

    Bomb(int o, Vector2i p, int c, int r)
            : Entity(o, p), count(c), range(r), ic(c) {}
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

    array<Bomb, 32> bombs;
    int s;

    array<Bomb, 32>::iterator begin() const {
        return (array<Bomb, 32>::iterator) (bombs.begin());
    }

    array<Bomb, 32>::iterator end() const {
        return (array<Bomb, 32>::iterator) (bombs.begin() + s);
    }

    array<Bomb, 32>::iterator findNext(int x, int y, array<Bomb, 32>::iterator it) const {
        it++;
        for (; it != end(); it++) {
            if (it->pos.x == x && it->pos.y == y) return (array<Bomb, 32>::iterator) it;
        }

        return (array<Bomb, 32>::iterator) end();
    }

    array<Bomb, 32>::iterator find(int x, int y) const {
        int i = 0;
        auto it = bombs.begin();
        for (; i < s; i++) {
            if ((it + i)->pos.x == x && (it + i)->pos.y == y) return (array<Bomb, 32>::iterator) (it + i);
        }

        return (array<Bomb, 32>::iterator) end();
    }

    class ByCount {
    public:
        bool operator()(const Bomb &l, const Bomb &r) {
            return l.count < r.count;
        }
    };

    void sort() {
        std::sort(bombs.begin(), bombs.begin() + s, BombArray::ByCount());
    }

    void push(const Bomb &a) {
        bombs[s] = a;
        s++;
    }

    void erase(array<Bomb, 32>::iterator it) {
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
    const static char JustBlown = 5;

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

class State;

class XpCells {
public:
    char height, width;
    int field[11][13];
    double blown[4][9];

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

        return 10000;
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

    void update(BombArray *ba, ItemArray *ia, const Field &f, const State &s);
};

class State {
public:
    State() : dead(false), boxesBroke(0), prevRes(0) {
    }

    int myId;
    Field f;
    XpCells xpc;
    BombArray ba;
    ItemArray ia;

    int bs, is;

    Player players[4];
    double boxesBroke;
    bool dead;
    double value;
    double bombScore;
    int evade;
    double prevRes;

    bool canOccupy(int x, int y, int round) const {
        if (x < 0 || x >= f.width || y < 0 || y >= f.height) return false;
        if ((f.getType(x, y) != Field::Empty&&f.getType(x, y) != Field::Wall) && xpc.firstBlow(x, y) < round)
            return true;
        if (f.getType(x, y) != Field::Empty) return false;
        if (ba.find(x, y) != ba.end()) {
            if (ba.find(x, y)->count == ba.find(x, y)->ic)
                return true;
            if (ba.find(x, y)->count >= round)
                return false;
        }
        return true;
    }

    void updateBombScore() {
        double res = 0;
        for (int i = 0; i < 9; i++) {
            res += (double) xpc.blown[myId][i];
        }

        bombScore = res;
    }
};

void XpCells::update(BombArray *ba, ItemArray *ia, const Field &f, const State &s) {
    memset(field, 0, sizeof(field));
    memset(blown, 0, sizeof(blown));

    bool checked[11][13] = {};

    ba->sort();

    auto itr = ba->begin();

    queue<array<Bomb, 32>::iterator> q;

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
                        || f.getType(x, y) == Field::Wall) {
                        break;
                    } else {
                        set(x, y, c, true);
                        if (f.getType(x, y) != Field::Empty
                            && !blownBefore(x, y, c)) {
                            if(!checked[y][x]) {
                                blown[it->owner][c]+=1.0;
                                checked[y][x] = true;
                            }
                            break;
                        } else if (ia->find(x, y) != ia->end()
                                   && !blownBefore(x, y, c)) {
                            break;
                        } else {
                            auto it2 = ba->find(x, y);
                            bool brFlag = false;
                            while(it2!=ba->end()) {
                                if (it2->count > c) {
                                    if (it2->ic == it2->count) it2->ic = c;
                                    it2->count = c;
                                    q.push(it2);
                                    brFlag = true;
                                    continue;
                                }
                                if (it2->count == c) {
                                    brFlag = true;
                                    break;
                                }

                                it2 = ba->findNext(x, y, it2);
                            }
                            if (brFlag) break;
                        }
                    }
                }
            }
        }
    }
}

class StateEvaluator {
public:
    static int calcEvade(const State &s, int player) {
        int res = 0;

        queue<pair<int, pair<int, int> > > q;
        q.push(make_pair(0, make_pair(s.players[player].pos.x, s.players[player].pos.y)));

        while (!q.empty()) {
            auto p = q.front();
            q.pop();

            if (s.xpc.get(p.second.first, p.second.second, p.first)) {
                continue;
            }

            if (p.first > s.xpc.lastBlow(p.second.first, p.second.second)) {
                res = 1;
                break;
            }

            for (int i = 0; i < 4; i++) {
                int nx = p.second.first + dx[i], ny = p.second.second + dy[i];
                if (s.canOccupy(nx, ny, p.first + 1)
                    && (s.ba.find(nx, ny)==s.ba.end() || s.ba.find(nx, ny)->count != s.ba.find(nx, ny)->ic)
                    && (!s.xpc.get(nx, ny, p.first)))
                    q.push(make_pair(p.first + 1, make_pair(nx, ny)));
            }

            if (!s.xpc.get(p.second.first, p.second.second, p.first + 1))
                q.push(make_pair(p.first + 1, p.second));
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

    static double evaluate(State &s) {
        if (s.players[s.myId].dead || s.evade == 0) return 0;
        double res = s.value;
        res += s.bombScore * 15000;
        res += s.boxesBroke * 15000;
        res += (double) s.evade * 1;
        res += (double)(s.players[s.myId].xpRange-2) * 2700.0 + (double)s.players[s.myId].numBombs * 14000.0;

        res += s.prevRes * 0.1;

        for (int i=0;i<4;i++) {
            if (i!=s.myId&&s.players[i].initialized) {
                res += s.players[i].dead * 8000;
            }
        }

        s.prevRes = res;
        return res;
    }
};

class FirstStateComparator {
public:
    bool operator()(const pair<State *, int> &l, const pair<State *, int> &r) const {
        return StateEvaluator::evaluate(*l.first) > StateEvaluator::evaluate(*r.first);
    }
};

class StateComparator {
public:
    bool operator()(const pair<State *, int> &l, const pair<State *, int> &r) const {
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

const Action actions[40] = {
                Action(0, false),
                Action(1, false),
                Action(3, false),
                Action(2, false),
                Action(0, true),
                Action(1, true),
                Action(3, true),
                Action(2, true),
                Action(4, false),
                Action(4, true),
                Action(2, false),
                Action(3, false),
                Action(1, false),
                Action(0, false),
                Action(2, true),
                Action(3, true),
                Action(1, true),
                Action(0, true),
                Action(4, false),
                Action(4, true),
                Action(2, false),
                Action(1, false),
                Action(3, false),
                Action(0, false),
                Action(2, true),
                Action(1, true),
                Action(3, true),
                Action(0, true),
                Action(4, false),
                Action(4, true),
                Action(0, false),
                Action(3, false),
                Action(1, false),
                Action(2, false),
                Action(0, true),
                Action(3, true),
                Action(1, true),
                Action(2, true),
                Action(4, false),
                Action(4, true)
};

State *blow(const State &s) {
    State *res = new State(s);

    for (auto it = res->ba.begin(); it != res->ba.end(); it++) {
        it->count--;
    }

    res->xpc.update(&(res->ba), &(res->ia), (res->f), *res);

    for (int y = 0; y < 13; y++) {
        for (int x = 0; x < 11; x++) {
            if (res->xpc.get(x, y, 0)) {
                if (res->f.getType(x, y) == Field::JustBlown) {
                    res->f.setType(x, y, Field::Empty);
                }

                if (res->f.getType(x, y) == Field::EmptyBox) {
                    res->f.setType(x, y, Field::JustBlown);
                    res->boxesBroke+=1;
                } else if (res->f.getType(x, y) == Field::ItemBox1) {
                    res->f.setType(x, y, Field::JustBlown);
                    res->ia.push(Item(Vector2i(x, y), 1));
                    res->boxesBroke+=1;
                } else if (res->f.getType(x, y) == Field::ItemBox2) {
                    res->f.setType(x, y, Field::JustBlown);
                    res->ia.push(Item(Vector2i(x, y), 2));
                    res->boxesBroke+=1;
                } else if (res->ia.find(x, y) != res->ia.end()) {
                    res->ia.erase(res->ia.find(x, y));
                } else {
                    auto it2 = res->ba.find(x, y);
                    while(it2!=res->ba.end()) {
                        res->players[it2->owner].bombDelta += 1;

                        it2 = res->ba.findNext(x, y, it2);
                    }

                    while(res->ba.find(x, y)!=res->ba.end()) {
                        res->ba.erase(res->ba.find(x, y));
                    }
                }

                for (int i = 0; i < 4; i++)
                    if (res->players[i].pos.x == x && res->players[i].pos.y == y) {
                        res->players[i].dead = true;
                    }
            }
        }
    }

    return res;
}

State *act(const State &s, char actionId) {
    if (actions[actionId].place && (s.players[s.myId].remainingBombs < 1))
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
        res->players[res->myId].bombDelta -= 1;
    }

    if (actions[actionId].dir < 4) {
        res->players[res->myId].pos.x += dx[actions[actionId].dir];
        res->players[res->myId].pos.y += dy[actions[actionId].dir];
        if (res->ia.find(res->players[res->myId].pos.x, res->players[res->myId].pos.y) !=
            res->ia.end()) {
            auto it = res->ia.find(res->players[res->myId].pos.x, res->players[res->myId].pos.y);
            switch (it->type) {
                case 1:
                    res->players[res->myId].xpRange += 1;
                    break;
                case 2:
                    res->players[res->myId].numBombs += 1;
                    res->players[res->myId].bombDelta += 1;
                    break;
            }
            res->ia.erase(it);
        }
    }

    el1 = clock();

    State *del = res;

    res = blow(*res);

    delete del;

    for (int i=0;i<4;i++) {
        res->players[i].remainingBombs += res->players[i].bombDelta;
        res->players[i].bombDelta = 0;
    }

    el2 = clock();

    res->updateBombScore();

    el3 = clock();

    res->evade = StateEvaluator::calcEvade(*res, res->myId);

    res->value = StateEvaluator::evaluate(*res);

    cend = clock();

    elS1 += (double) (el1 - start) / CLOCKS_PER_SEC;
    elS2 += (double) (el2 - el1) / CLOCKS_PER_SEC;
    elS3 += (double) (el3 - el2) / CLOCKS_PER_SEC;
    elS4 += (double) (cend - el3) / CLOCKS_PER_SEC;

    return res;
}

State *tryAllEnemiesPlaceBomb(const State &s) {
    State *res = new State(s);
    for(int i=0;i<4;i++) {
        if(i!=res->myId&&!res->players[i].dead&&res->players[i].remainingBombs > 0) {
            res->ba.push(Bomb(i,
                              Vector2i(res->players[i].pos.x, res->players[i].pos.y),
                              8,
                              (int) res->players[i].xpRange));
        }
    }

    for(auto it = res->ia.begin();it!=res->ia.end();) {
        for (int i=0;i<4;i++) {
            if(i!=res->myId&&!res->players[i].dead&&(abs(res->players[i].pos.x-res->players[res->myId].pos.x)+abs(res->players[i].pos.y-res->players[res->myId].pos.y)>1)) {
                if (abs(res->players[i].pos.x-it->pos.x)+abs(res->players[i].pos.y-it->pos.y)<=3) {
                    res->ia.erase(it);
                    break;
                }
            }
            if (i==3) it++;
        }
    }

    return res;
}

int main() {
    ZunDokoKiyoshi zdk;
    clock_t startCLK, endCLK;

    int width;
    int height;
    int myId;
    scanf("%d %d %d", &width, &height, &myId);

    int cx = 12, cy = 10;

    int co = 0;

    if (myId%2==0) {
        cy = 0;
    }
    if (myId==0 || myId==3) cx = 0;

    State current;

    current.myId = myId;

    startCLK = clock();

    // game loop
    while (1) {
        qcnt = 0;

        deque<pair<State *, int> > beam[BEAM_DEPTH + 1];

        current.ba.s = 0;
        current.ia.s = 0;
        current.boxesBroke = 0;
        int minDist = 100;
        for (int i = 0; i < height; i++) {
            char row[14];
            scanf("%s", row);
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
        scanf("%d", &entities);
        for (int i = 0; i < entities; i++) {
            int entityType;
            int owner;
            int x;
            int y;
            int param1;
            int param2;
            scanf("%d %d %d %d %d %d", &entityType, &owner, &x, &y, &param1, &param2);

            switch (entityType) {
                case 0:
                    current.players[owner] = Player(owner, Vector2i(x, y), param1, param1, param2);
                    break;
                case 1:
                    current.ba.push(Bomb(owner, Vector2i(x, y), param1, param2));
                    current.players[owner].numBombs += 1;
                    break;
                case 2:
                    current.ia.push(Item(Vector2i(x, y), param1));
                    break;
            }
        }

        State *nextState;

        State *b = blow(current);

        State *tryResult = tryAllEnemiesPlaceBomb(*b);

        tryResult->xpc.update(&tryResult->ba, &tryResult->ia, tryResult->f, *tryResult);

        int s = 0;

        float countBoxesDir[4] = {0, 0, 0, 0};

        float bDir[11][13] = {};

        for (int y=0;y<11;y++) {
            for (int x=0;x<13;x++) {
                if (current.f.getType(x, y)==Field::Empty||current.f.getType(x, y)==Field::Wall||current.f.getType(x, y)==Field::JustBlown) continue;
                if (current.xpc.field[y][x]!=0) continue;
                for (int i=0;i<4;i++) {
                    for (int d=1;d<13;d++) {
                        if (x+dx[i]*d<0||x+dx[i]*d>=13||y+dy[i]*d<0||y+dy[i]*d>=11) break;
                        if (current.f.getType(x+dx[i]*d, y+dy[i]*d)!=Field::Empty&&current.f.getType(x, y)!=Field::JustBlown) break;
                        bDir[y+dy[i]*d][x+dx[i]*d]+=5.0/d;
                    }
                }
            }
        }

        for (int y=0;y<11;y++) {
            for (int x=0;x<13;x++) {
                if (current.f.getType(x, y)==Field::Empty) {
                    if ((x-current.players[myId].pos.x)>=abs(y-current.players[myId].pos.y)) countBoxesDir[0]+=bDir[y][x];
                    if (-(x-current.players[myId].pos.x)>=abs(y-current.players[myId].pos.y)) countBoxesDir[1]+=bDir[y][x];
                    if ((y-current.players[myId].pos.y)>=abs(x-current.players[myId].pos.x)) countBoxesDir[2]+=bDir[y][x];
                    if (-(y-current.players[myId].pos.y)>=abs(x-current.players[myId].pos.x)) countBoxesDir[3]+=bDir[y][x];
                }
            }
        }

        for (auto it = current.ba.begin();it != current.ba.end(); it++) {
            if (it->owner==myId) continue;
            if ((it->pos.x-current.players[myId].pos.x)>=abs(it->pos.y-current.players[myId].pos.y)) countBoxesDir[0]-=1;
            if (-(it->pos.x-current.players[myId].pos.x)>=abs(it->pos.y-current.players[myId].pos.y)) countBoxesDir[1]-=1;
            if ((it->pos.y-current.players[myId].pos.y)>=abs(it->pos.x-current.players[myId].pos.x)) countBoxesDir[2]-=1;
            if (-(it->pos.y-current.players[myId].pos.y)>=abs(it->pos.x-current.players[myId].pos.x)) countBoxesDir[3]-=1;
        }

        for (int i=0;i<4;i++) {
            if (i==myId) continue;
            if ((current.players[i].pos.x-current.players[myId].pos.x)>=abs(current.players[i].pos.y-current.players[myId].pos.y)) countBoxesDir[0]-=2;
            if (-(current.players[i].pos.x-current.players[myId].pos.x)>=abs(current.players[i].pos.y-current.players[myId].pos.y)) countBoxesDir[1]-=2;
            if ((current.players[i].pos.y-current.players[myId].pos.y)>=abs(current.players[i].pos.x-current.players[myId].pos.x)) countBoxesDir[2]-=2;
            if (-(current.players[i].pos.y-current.players[myId].pos.y)>=abs(current.players[i].pos.x-current.players[myId].pos.x)) countBoxesDir[3]-=2;
        }

        co = 0;

        fprintf(stderr, "%f %f %f %f\n", countBoxesDir[0], countBoxesDir[1], countBoxesDir[2], countBoxesDir[3]);

        if(countBoxesDir[co]<countBoxesDir[1]) co = 1;
        if(countBoxesDir[co]<countBoxesDir[2]) co = 2;
        if(countBoxesDir[co]<countBoxesDir[3]) co = 3;

        for (char i = 0; i < 10; i++) {
            if ((nextState = act(*tryResult, i + co * 10)) == nullptr) continue;
            fprintf(stderr, "%d: %d\n", i, nextState->evade);
            if (nextState->players[myId].dead || nextState->evade == 0) {
                delete nextState;
                continue;
            }
            s++;
            beam[0].insert(lower_bound(beam[0].begin(),
                                       beam[0].end(),
                                       make_pair(nextState, i),
                                       FirstStateComparator()),
                           make_pair(nextState, i));
            fprintf(stderr, "%d %f %f\n", i, nextState->bombScore, StateEvaluator::evaluate(*nextState));
        }

        if (s==0) {
            for (char i = 0; i < 10; i++) {
                if ((nextState = act(*b, i + co * 10)) == nullptr) continue;
                fprintf(stderr, "%d: %d\n", i, nextState->evade);
                if (nextState->players[myId].dead || nextState->evade == 0) {
                    delete nextState;
                    continue;
                }
                s++;
                beam[0].insert(lower_bound(beam[0].begin(),
                                           beam[0].end(),
                                           make_pair(nextState, i),
                                           FirstStateComparator()),
                               make_pair(nextState, i));
            }
        }

        delete b;
        delete tryResult;

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

                for (char j = 0; j < 10; j++) {
                    if ((nextState = act(*(st.first), j + co * 10)) != nullptr) {
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

        fprintf(stderr, "%f %f %f %f\n", elS1, elS2, elS3, elS4);

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

            hoge *= 1.0;
        }

        int ans = 0;

        for (int i = 0; i < 10; i++) {
            if (sol[ans] / max(1.0, ((double)cnt[ans])) < sol[i] / max(1.0, ((double)cnt[i])))
                ans = i;
            //cerr << cnt[i] << " " << sol[i] / max(1, cnt[i]) << endl;
        }

        //cerr << "qcnt: " << qcnt << endl;
        current.xpc.update(&current.ba, &current.ia, current.f, current);

        cx = current.players[myId].pos.x + ((actions[ans + co * 10].dir != 4) ? dx[actions[ans + co * 10].dir] : 0);
        cy = current.players[myId].pos.y + ((actions[ans + co * 10].dir != 4) ? dy[actions[ans + co * 10].dir] : 0);

        endCLK = clock();

        printf("%s %d %d %s %dms\n",
               (actions[ans].place ? "BOMB " : "MOVE "),
               cx,
               cy,
               zdk.getZDK(),
               (int)(endCLK-startCLK) / (CLOCKS_PER_SEC/1000));

        startCLK = endCLK;
    }

    return 0;
}