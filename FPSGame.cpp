﻿#include <iostream>
#include <Windows.h>
#include <string>
#include <chrono>
#include <vector>
#include <algorithm>   // std::sort
#include <limits>
#include <float.h>


using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 35;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16;
float fAngle = 0.0f; //initial player angle 
// maxValue = FLT_MAX;


int main() {

    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    std::wstring map;

    map += L"################";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.....###......#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..#...........#";
    map += L"#..#.......#####";
    map += L"#..#...........#";
    map += L"#..#...........#";
    map += L"#..#...........#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    while (1) {

        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
            fPlayerA -= (1.2f) * fElapsedTime;
            fAngle -= (360 / 3.14159) * fElapsedTime;
        }

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
            fPlayerA += (1.2f) * fElapsedTime;
            fAngle += (360 / 3.14159) * fElapsedTime;
        }

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

            if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        /*if (fAngle < 0) {
            fAngle += 360;
        }

        if (fAngle >= 360) {
            fAngle -= 360;
        }*/

        if (fAngle < 0) {
            fAngle = 360;
        }

        if (fAngle >= 360) {
            fAngle = 0;
        }

        for (int x = 0; x < nScreenWidth; x++) {

            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fDistanceToWall = 0;
            bool bHitByWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            while (!bHitByWall && fDistanceToWall < fDepth) {

                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {

                    bHitByWall = true;
                    fDistanceToWall = fDepth;
                }
                else {

                    if (map[nTestY * nMapWidth + nTestX] == '#') {

                        bHitByWall = true;

                        vector<pair<float, float>> p;

                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first;});

                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound) {
                            bBoundary = true;
                        }

                        if (acos(p.at(1).second) < fBound) {
                            bBoundary = true;
                        }

                        if (acos(p.at(2).second) < fBound) {
                            bBoundary = true;
                        }
                    }
                }
            }

            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';
            short nFloorShade = ' ';

            if (fDistanceToWall <= fDepth / 4.0f)           nShade = 0x2588;
            else if (fDistanceToWall < fDepth / 3.0f)       nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)       nShade = 0x2592;
            else if (fDistanceToWall < fDepth)              nShade = 0x2591;
            else                                            nShade = ' ';

            if (bBoundary) {
                nShade = ' ';
            }

            for (int y = 0; y < nScreenHeight; y++) {
                if (y < nCeiling)
                    screen[y * nScreenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else {
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < .25) {
                        nFloorShade = '#';
                    }
                    else if (b < .75) {
                        nFloorShade = 'x';
                    }
                    else if (b < .9) {
                        nFloorShade = '.';
                    }
                    else {
                        nFloorShade = ' ';
                    }
                    screen[y * nScreenWidth + x] = nFloorShade;
                }

            }
        }

        swprintf_s(screen, 40, L"X = %3.2f, Y = %3.2f, Angle = %3.2f", fPlayerX, fPlayerY, fAngle);

        for (int nx = 0; nx < nMapWidth; nx++) {
            for (int ny = 0; ny < nMapWidth; ny++) {
                screen[(ny + 1) * nScreenWidth + (nx + 1)] = map[ny * nMapWidth + nx];
            }
        }


        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';



        screen[nScreenHeight * nScreenWidth - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
    }
    return 0;
}