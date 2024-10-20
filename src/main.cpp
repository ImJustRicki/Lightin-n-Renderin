#include <raylib.h>
#include "Math.h"

#include <cassert>
#include <array>
#include <vector>
#include <algorithm>

const float SCREEN_SIZE = 800;

const int TILE_COUNT = 20;
const float TILE_SIZE = SCREEN_SIZE / TILE_COUNT;

enum TileType : int
{
    GRASS,      // Marks unoccupied space, can be overwritten 
    DIRT,       // Marks the path, cannot be overwritten
    WAYPOINT,   // Marks where the path turns, cannot be overwritten
    TURRET,
    COUNT
};

struct Cell
{
    int row;
    int col;
};

constexpr std::array<Cell, 4> DIRECTIONS{ Cell{ -1, 0 }, Cell{ 1, 0 }, Cell{ 0, -1 }, Cell{ 0, 1 } };

inline bool InBounds(Cell cell, int rows = TILE_COUNT, int cols = TILE_COUNT)
{
    return cell.col >= 0 && cell.col < cols && cell.row >= 0 && cell.row < rows;
}

void DrawTile(int row, int col, Color color)
{
    DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
}

void DrawTile(int row, int col, int type)
{
    Color color = type > 0 ? BEIGE : GREEN;
    DrawTile(row, col, color);
}

Vector2 TileCenter(int row, int col)
{
    float x = col * TILE_SIZE + TILE_SIZE * 0.5f;
    float y = row * TILE_SIZE + TILE_SIZE * 0.5f;
    return { x, y };
}

Vector2 TileCorner(int row, int col)
{
    float x = col * TILE_SIZE;
    float y = row * TILE_SIZE;
    return { x, y };
}

// Returns a collection of adjacent cells that match the search value.
std::vector<Cell> FloodFill(Cell start, int tiles[TILE_COUNT][TILE_COUNT], TileType searchValue)
{
    // "open" = "places we want to search", "closed" = "places we've already searched".
    std::vector<Cell> result;
    std::vector<Cell> open;
    bool closed[TILE_COUNT][TILE_COUNT];
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            // We don't want to search zero-tiles, so add them to closed!
            closed[row][col] = tiles[row][col] == 0;
        }
    }

    // Add the starting cell to the exploration queue & search till there's nothing left!
    open.push_back(start);
    while (!open.empty())
    {
        // Remove from queue and prevent revisiting
        Cell cell = open.back();
        open.pop_back();
        closed[cell.row][cell.col] = true;

        // Add to result if explored cell has the desired value
        if (tiles[cell.row][cell.col] == searchValue)
            result.push_back(cell);

        // Search neighbours
        for (Cell dir : DIRECTIONS)
        {
            Cell adj = { cell.row + dir.row, cell.col + dir.col };
            if (InBounds(adj) && !closed[adj.row][adj.col] && tiles[adj.row][adj.col] > 0)
                open.push_back(adj);
        }
    }

    return result;
}

// TODO - Make an Enemy structure. Its data is up to you!

struct Enemy
{
    Vector2 position{};
    int health = 15;
    size_t curr = 0;
    size_t next = curr + 1;
    bool atEnd = false;
};

// TODO - Make a Turret structure. Its data is up to you!

struct Turret
{
    Vector2 position{};
    float range = 222.2f;
    float cooldown = 0.0f;
    float firerate = 0.75f;
};

struct Bullet
{
    Vector2 position{};
    Vector2 direction{};
    float time = 0.0f;
    bool enabled = true;
};

int main()
{
    // TODO - Modify this grid to contain turret tiles. Instantiate turret objects accordingly
    int tiles[TILE_COUNT][TILE_COUNT]
    {
        //col:0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19    row:
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0 }, // 0
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 1
            { 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 2
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 3
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 0 }, // 4
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 5
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 6
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0 }, // 7
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
            { 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 10
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 11
            { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 12
            { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 13
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 14
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0 }, // 15
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 16
            { 0, 0, 3, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 17
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 18
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  // 19
    };

    std::vector<Cell> waypoints = FloodFill({ 0, 12 }, tiles, WAYPOINT);
    std::vector<Enemy> enemies;
    std::vector<Turret> turrets;

    for (int row = 0; row < TILE_COUNT; ++row)
    {
        for (int col = 0; col < TILE_COUNT; ++col)
        {
            if (tiles[row][col] == 3)
            {
                Turret turret;
                turret.position = TileCenter(row, col);
                turrets.push_back(turret);
            }
        }
        
    }

  // size_t curr = 0;
  // size_t next = curr + 1;
  // bool atEnd = false;

    
    const float enemySpeed = 250.0f;
    const float enemyRadius = 20.0f;

    const float bulletTime = 1.0f;
    const float bulletSpeed = 500.0f;
    const float bulletRadius = 15.0f;

    std::vector<Bullet> bullets;
    float shootCurrent = 0.0f;
    float shootTotal = 0.25f;

    float enemyTime = 0.0f;
    int totalEnemies = 10;
    int enemyCount = 0;

    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Tower Defense");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        // TODO - Spawn 10 enemies
        // TODO - Make enemies follow the path
        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();
        Cell mouseCell;
        mouseCell.row = mouse.y / TILE_SIZE;
        mouseCell.col = mouse.x / TILE_SIZE;

        enemyTime += dt;

        
        

        if (enemyTime >= 1.0f && enemyCount < totalEnemies)
        {
            Enemy enemy;
            enemyCount++;
            enemy.position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);;
            enemy.curr = 0;
            enemy.next = enemy.curr + 1;
            enemy.atEnd = false;
            enemies.push_back(enemy);
            enemyTime = 0;
        }

        // Path following
        for (Enemy& enemy:enemies)
        {
            if (!enemy.atEnd)
            {
                Vector2 from = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
                Vector2 to = TileCenter(waypoints[enemy.next].row, waypoints[enemy.next].col);
                Vector2 direction = Normalize(to - from);
                enemy.position = enemy.position + direction * enemySpeed * dt;
                if (CheckCollisionPointCircle(enemy.position, to, enemyRadius))
                {
                    enemy.curr++;
                    enemy.next++;
                    enemy.atEnd = enemy.next == waypoints.size();
                    enemy.position = TileCenter(waypoints[enemy.curr].row, waypoints[enemy.curr].col);
                }
            }

        }
       
        for (Turret& turret : turrets)
        {
            turret.cooldown += dt;
            Enemy* dweebs = nullptr;
            for (Enemy& enemy : enemies)
            {
                float enemyDistance = Distance(turret.position, enemy.position);
                if (enemyDistance < turret.range)
                {
                    dweebs = &enemy;
                }
            }
            if (dweebs && turret.cooldown >= turret.firerate)
            {
                turret.cooldown = 0.0f;
                Bullet bullet;
                bullet.position = turret.position;
                bullet.direction = Normalize(dweebs->position - bullet.position);
                bullets.push_back(bullet);
            }
        }

        // TODO - Loop through all turrets & enemies, shoot bullets accordingly
    

        // TODO - Loop through all bullets & enemies, handle collision & movement accordingly
        for (Bullet& bullet : bullets)
        {
            bullet.position = bullet.position + bullet.direction * bulletSpeed * dt;
            bullet.time += dt;

            bool expired = bullet.time >= bulletTime;
            for (int i = 0; i < enemies.size();)
            {
                Enemy& enemy = enemies[i];
                bool collision = CheckCollisionCircles(enemy.position, enemyRadius, bullet.position, bulletRadius);
                if (collision)
                {
                    enemy.health--;
                    if (enemy.health <= 0)
                    {
                        enemies.erase(enemies.begin() + i);
                        bullet.enabled = false;
                        break;
                    }
                }
                else
                {
                    i++;
                }
            }
            bullet.enabled = !expired && bullet.enabled;
        }

        // Bullet removal
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [&bullets](Bullet bullet) {
                return !bullet.enabled;
            }), bullets.end());

        BeginDrawing();
        ClearBackground(BLACK);
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }
        //DrawCircleV(enemy.position, enemyRadius, RED);

        // Render bullets
        for (const Bullet& bullet : bullets)
            DrawCircleV(bullet.position, bulletRadius, BLUE);
        for (const Enemy& enemy : enemies)
            DrawCircleV(enemy.position, enemyRadius, PINK);
        for (const Turret& turret : turrets)
            DrawCircleV(turret.position, bulletRadius, DARKPURPLE);
        DrawText(TextFormat("Total bullets: %i", bullets.size()), 10, 10, 20, BLUE);
        DrawTile(mouseCell.row, mouseCell.col, PURPLE);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}