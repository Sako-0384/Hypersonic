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