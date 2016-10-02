//
// Created by nonsako on 2016/10/01.
//



bool checked[13][11] = {};
queue < pair<int, int> > q;

int maxEfficiency = 0;

q.push(make_pair(b->players[myId].pos.x, b->players[myId].pos.y));

while(!q.empty()) {
auto p = q.front();
q.pop();

if (!b->canOccupy(p.first, p.second, 0)) continue;

for (int i = 0; i < 4; i++) {
for (int r = 1; r < b->players[myId].range; r++) {
int x = p.first + dx[i] * r, y = p.second + dy[i] * r;
if (x < 0 || x >= width
|| y < 0 || y >= height
|| b->f.getType(it->pos) == Field::Wall) {
break;
} else {
set(x, y, c, true);
if (f.getType(x, y) != Field::Empty
&& !blownBefore(x, y, c)) {
if(!checked[y][x]) {
blown[it->owner][c]+=1.0;
if (s.targetX==x&&s.targetY==y) blown[it->owner][c]+=0.3;
checked[y][x] = true;
}
break;
} else if (ia->find(x, y) != ia->end()
&& !blownBefore(x, y, c)) {
break;
} else if (ba->find(x, y) != ba->end()
&& ba->find(x, y)->count > c) {
if(ba->find(x, y)->ic == ba->find(x, y)->count) ba->find(x, y)->ic = c;
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