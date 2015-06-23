//
// Created by Fiskie on 22/06/15.
//

#include "FirstPersonCamera.h"

#define min(a, b)           a < b ? a : b
#define max(a, b)           a < b ? a : b
#define clamp(a, mi, ma)    min(max(a, mi), ma)
#define vxs(x1, y1, x2, y2) (x1*y2 - y1*x2)
#define overlap(a0, a1, b0, b1) (min(a0,a1) <= max(b0,b1) && min(b0,b1) <= max(a0, a1))

Pos intersect(double x1, double z1, double x2, double z2, double x3, double z3, double x4, double z4) {
    Pos pos;

    double x = vxs(x1, z1, x2, z2);
    double z = vxs(x3, z3, x4, z4);
    double det = vxs(x1-x2, z1-z2, x3-x4, z3-z4);

    x = vxs(x, x1 - x2, z, x3-x4) / det;
    z = vxs(x, z1 - z2, z, z3-z4) / det;

    pos.x = x;
    pos.z = z;

    return pos;
}

void FirstPersonCamera::drawUI() {
    SDL_Renderer *renderer = game->getRenderer();

    // Draw a crosshair
    SDL_SetRenderDrawColor(renderer, 255, 66, 255, 1);
    SDL_RenderDrawLine(renderer, game->originX - 5, game->originZ, game->originX + 5, game->originZ);
    SDL_RenderDrawLine(renderer, game->originX, game->originZ - 5, game->originX, game->originZ + 5);
}

void FirstPersonCamera::drawWall(Wall *wall) {
    SDL_Renderer *renderer = game->getRenderer();
    Player *player = game->getPlayer();

    // Fetch some values we're going to be using a lot.
    double oX = game->originX;
    double oZ = game->originZ;
    double pCos = cos(player->rot.x + 1.57);
    double pSin = sin(player->rot.x + 1.57);
    double fovH = 0.16f * game->resY;
    double fovW = 0.16f * game->resY;

    int points[4][2];
    int unseenPoints = 0;

    for (int i = 0; i < 4; i++) {
        Pos vertice = wall->getPoint(i);

        // Get the coordinates relative to the player.
        double vX = vertice.x - player->loc.x, vY = vertice.y - player->loc.y, vZ = vertice.z - player->loc.z;

        // Rotate the coordinates around the player's view.
        double tX = vX * pCos - vZ * pSin;
        double tZ = vX * pSin + vZ * pCos;

        if (tX <= 0) {
            float nearz = 1e-4f, farz = 5, nearside = 1e-5f, farside = 20.f;

            Pos i1 = intersect(tX, tZ, -tX, -tZ, -nearside, nearz, -farside, farz);
            Pos i2 = intersect(tX, tZ, -tX, -tZ, nearside, nearz, farside, farz);

            if (tX < nearz) {
                if (i1.z > 0) {
                    drawLabel(format("Updating coords from (%.2f, %.2f) to (%.2f, %.2f)", tX, tZ, i1.x, i1.z), 5, i1.z);
                    tX = i1.x;
                    tZ = i1.z;
                } else {
                    drawLabel(format("Updating coords from (%.2f, %.2f) to (%.2f, %.2f)", tX, tZ, i2.x, i2.z), 5, i2.z);
                    tX = i2.x;
                    tZ = i2.z;
                }
            }

            unseenPoints++;
        }

        double xScale = fovH / tX, zScale = fovW / tX;

        int screenX = (int) (oX + vX * xScale);
        int screenY = (int) (oZ + vY * zScale);

        points[i][0] = screenX;
        points[i][1] = screenY;

        drawLabel(format("Screen: (%d, %d)", points[i][0], points[i][1]), points[i][0], points[i][1]);
        drawLabel(format("World: (%.2f,%.2f,%.2f)", vertice.x, vertice.y, vertice.z), points[i][0], points[i][1] + 15);
        drawLabel(format("Rotated: (%.2f,%.2f)", tX, tZ), points[i][0], points[i][1] + 30);
        drawLabel(format("Relative: (%.2f,%.2f,%.2f)", vX, vY, vZ), points[i][0], points[i][1] + 45);
        drawLabel(format("Scale: (%.2f,%.2f)", xScale, zScale), points[i][0], points[i][1] + 60);
    }

    if (unseenPoints == 4)
        return;

    // Draw lines.
    SDL_SetRenderDrawColor(renderer, 66, 255, 66, 1);
    SDL_RenderDrawLine(renderer, points[0][0], points[0][1], points[1][0], points[1][1]);
    SDL_RenderDrawLine(renderer, points[1][0], points[1][1], points[2][0], points[2][1]);
    SDL_RenderDrawLine(renderer, points[2][0], points[2][1], points[3][0], points[3][1]);
    SDL_RenderDrawLine(renderer, points[3][0], points[3][1], points[0][0], points[0][1]);
}

void FirstPersonCamera::render() {
    SDL_Renderer *renderer = game->getRenderer();
    Map *map = game->getMap();

    SDL_RenderClear(renderer);

    list<Wall> *walls = map->getWalls();

    list<Wall>::iterator j;

    for (j = walls->begin(); j != walls->end(); ++j) {
        drawWall(&(*j));
    }

    drawUI();
    drawDebugInfo();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);

    // SDL_UpdateWindowSurface(window);
    SDL_RenderPresent(renderer);
}

FirstPersonCamera::FirstPersonCamera(Game *game) {
    this->game = game;
}

void FirstPersonCamera::drawDebugInfo() {
    Player *player = game->getPlayer();

    drawLabel("fisk3d", 5, 5);
    drawLabel("First Person Camera", 5, 20);
    drawLabel(format("Pos: (%.2f,%.2f,%.2f)", player->loc.x, player->loc.y, player->loc.z), 5, 35);
    drawLabel(format("Rot: (%.2f,%.2f,%.2f)", player->rot.x, player->rot.y, player->rot.z), 5, 50);
    drawLabel(format("xSin, xCos: (%.2f,%.2f)", sin(player->rot.x), cos(player->rot.x)), 5, 65);
}
