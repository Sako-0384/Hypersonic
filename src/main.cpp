#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

const int dx[4] = {1,0,-1,0};
const int dy[4] = {0,1,0,-1};

int main() {
    int width;
    int height;
    int myId;

    int turnCount = 0;
    string field[11];

    int w[11][13] = {};
    int bCount = 0;
    double score = 0;
    double maxScore = 0;
    int destX = 0;
    int destY = 0;
    int playerX = 0, playerY = 0;
    int enemyBombX = -1, enemyBombY = -1;
    bool bombRemains = false;

    cin >> width >> height >> myId; cin.ignore();

    // game loop
    while (1) {
        bombRemains = false;
        enemyBombX = -1, enemyBombY = -1;

        for (int i = 0; i < height; i++) {
            getline(cin, field[i]);
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

            if (entityType==0&&owner==myId) {
                playerX = x;
                playerY = y;
            } else if (entityType==1&&owner==myId) {
                bombRemains = true;
            } else if (entityType==1&&owner!=myId) {
                enemyBombX = x;
                enemyBombY = y;
            }
        }

        if (enemyBombX != -1) {
            for (int j=0;j<4;j++) {
                for (int k=1;k<3;k++) {
                    if (enemyBombX + dx[j] * k < 0 || 13 <= enemyBombX + dx[j] * k || enemyBombY + dy[j] * k < 0 || 11 <= enemyBombY + dy[j] * k) continue;
                    if (field[enemyBombY + dy[j] * k][enemyBombX + dx[j] * k] == '0') {
                        field[enemyBombY + dy[j] * k][enemyBombX + dx[j] * k] = '.';
                        break;
                    }
                }
            }
        }

        if (bombRemains||playerX!=destX||playerY!=destY) {
            cerr << bombRemains << endl;
            cout << "MOVE " << destX << " " << destY << endl;
        } else {
            cout << "BOMB " << destX << " " << destY << endl;

            for (int j=0;j<4;j++) {
                for (int k=1;k<3;k++) {
                    if (destX + dx[j] * k < 0 || 13 <= destX + dx[j] * k || destY + dy[j] * k < 0 || 11 <= destY + dy[j] * k) continue;
                    if (field[destY + dy[j] * k][destX + dx[j] * k] == '0') {
                        field[destY + dy[j] * k][destX + dx[j] * k] = '.';
                        break;
                    }
                }
            }

            maxScore = 0;

            destX = 0;
            destY = 0;

            queue< pair<int, int> > q;

            for (int i=0;i<height;i++)
                for (int j=0;j<width;j++)
                    w[i][j] = 0;
            w[playerY][playerX] = -1;
            q.push(make_pair(playerX, playerY));

            while(!q.empty()) {
                pair<int, int> p = q.front();
                q.pop();

                if(w[p.second][p.first]>8) continue;

                bCount = 0;

                for (int j=0;j<4;j++) {
                    for (int k=1;k<3;k++) {
                        if (p.first + dx[j] * k < 0 || 13 <= p.first + dx[j] * k || p.second + dy[j] * k < 0 || 11 <= p.second + dy[j] * k) continue;
                        if (field[p.second + dy[j] * k][p.first + dx[j] * k] == '0') {
                            bCount++;
                            break;
                        }
                    }
                }

                score = (double)bCount + (-((double)w[p.second][p.first]-5)*((double)w[p.second][p.first]-5)*2.5)/25 + 2.5;

                if (score > maxScore) {
                    cerr << p.first << " " << p.second << " " << score << endl;
                    maxScore = score;
                    destX = p.first;
                    destY = p.second;
                }

                for(int i=0;i<4;i++) {
                    if (p.first+dx[i]<0||13<=p.first+dx[i]||p.second+dy[i]<0||11<=p.second+dy[i]) continue;
                    if (w[p.second+dy[i]][p.first+dx[i]]!=0||field[p.second+dy[i]][p.first+dx[i]]=='0') continue;

                    w[p.second+dy[i]][p.first+dx[i]] = max(w[p.second][p.first], 0)+1;
                    q.push(make_pair(p.first+dx[i], p.second+dy[i]));
                }
            }
            cout << "MOVE " << destX << " " << destY << endl;
        }

        turnCount++;
    }
}