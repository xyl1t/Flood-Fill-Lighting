//  Created by Xylit on 01.11.19.

#include <iostream>
#include <queue>
#include <SDL2/SDL.h>
#include <time.h>

int max(int val1, int val2);
int min(int val1, int val2);
int clamp(int val, int min, int max);

const int WIDTH = 1024;
const int HEIGHT = 768;
const int TILE_SIZE = 8;
const int MAP_WIDTH = WIDTH / TILE_SIZE;
const int MAP_HEIGHT = HEIGHT / TILE_SIZE;
const int MAX_LIGHT_INTENSITY = 16;
int currentSkyLight = MAX_LIGHT_INTENSITY;

enum BLOCKTYPE
{
    AIR,
    SOLID,
    LIGHT,
};
struct Tile
{
    int x;
    int y;
    int blockType;
    int lightValue;
    int sunLight;
};
struct LightNode
{
    LightNode(int x, int y) { this->x = x; this->y = y; }
    int x;
    int y;
};
struct LightRemovalNode
{
    LightRemovalNode(int x, int y, int val) { this->x = x; this->y = y; this->val = val; }
    int x;
    int y;
    int val;
};

Tile tiles[MAP_HEIGHT][MAP_WIDTH];


inline int getSunlight(int x, int y) {
    return tiles[y][x].sunLight;
}
inline void setSunlight(int x, int y, int val) {
    tiles[y][x].sunLight = val;
}

inline int getTorchlight(int x, int y) {
    return tiles[y][x].lightValue;
}
inline void setTorchlight(int x, int y, int val) {
    tiles[y][x].lightValue = val;
}



std::queue <LightNode> lightBfsQueue;
std::queue <LightRemovalNode> lightRemovalBfsQueue;
std::queue <LightNode> sunlightBfsQueue;

int lightValue;
SDL_Rect rect {0,0, TILE_SIZE, TILE_SIZE};
int start, end;
int main(int argc, const char * argv[])
{
    srand(time(NULL));
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* surface;
    SDL_Event event;
    
    SDL_Init(SDL_INIT_VIDEO);
    
    window = SDL_CreateWindow("Flood fill lighting",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WIDTH,
                                          HEIGHT,
                                          SDL_WINDOW_ALLOW_HIGHDPI);
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    surface = SDL_CreateRGBSurfaceWithFormat(0, WIDTH, HEIGHT, 32, SDL_PIXELFORMAT_ABGR32);
    
    
    // Init
    bool running = true;
    
    
    int direction = 0;
    
    for(int i = 0; i < MAP_WIDTH;i++)
    {
        if(rand()%2 == 0)
        {
            direction += rand() % 3 - 1;
        }
        else if(rand()%3 == 0)
        {
            direction += rand() % 4 - 2;
        }
        int y = MAP_HEIGHT - (MAP_HEIGHT/4) + direction;
        tiles[y][i] = { i * TILE_SIZE, y * TILE_SIZE, BLOCKTYPE::SOLID, 0 };
    }
    
    bool solid = false;
    for (int j = 0; j < MAP_WIDTH; ++j)
    {
        for (int i = 0; i < MAP_HEIGHT; ++i)
        {
            solid |= tiles[i][j].blockType == SOLID;
            if(solid)
                tiles[i][j] = { j * TILE_SIZE, i * TILE_SIZE, BLOCKTYPE::SOLID, 0 };
            else
                tiles[i][j] = { j * TILE_SIZE, i * TILE_SIZE, BLOCKTYPE::AIR, 0 };
            
            
            
        }
        solid = false;
    }

    for (int i = 0; i < MAP_WIDTH; i++)
    {
        tiles[0][i].sunLight = currentSkyLight;
        sunlightBfsQueue.emplace(i, 0);
    }
    
    int mouseX = 0;
    int mouseY = 0;
    int displaySolids = 0;
    
    
    // Main loop
    while(running)
    {
        const Uint64 start = SDL_GetPerformanceCounter();
        
        
        // Handle events
        while(SDL_PollEvent(&event))
        {
            
            
            if(SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                tiles[mouseY/TILE_SIZE][mouseX/TILE_SIZE].blockType = BLOCKTYPE::SOLID;
            }
            else if(SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_RIGHT))
            {
                Tile& currentTile = tiles[mouseY/TILE_SIZE][mouseX/TILE_SIZE];
                if(currentTile.blockType == LIGHT)
                {
                    lightRemovalBfsQueue.emplace(mouseX/TILE_SIZE, mouseY/TILE_SIZE, currentTile.lightValue);
                    setTorchlight(mouseX/TILE_SIZE, mouseY/TILE_SIZE, 0);
                }
                currentTile.blockType = BLOCKTYPE::AIR;
            }
            
            switch(event.type)
            {
                // If users closes window, stop main loop
                case SDL_QUIT:
                    running = false;
                    break;
                
                case SDL_KEYDOWN:
                {
                    if(event.key.keysym.sym == SDLK_l)
                    {
                        tiles[mouseY/TILE_SIZE][mouseX/TILE_SIZE].blockType = BLOCKTYPE::LIGHT;
                        tiles[mouseY/TILE_SIZE][mouseX/TILE_SIZE].lightValue = MAX_LIGHT_INTENSITY;
                        lightBfsQueue.emplace(mouseX/TILE_SIZE,mouseY/TILE_SIZE);
                    }
                    
                    if(event.key.keysym.sym == SDLK_f)
                    {
                        for (int i = 0; i < MAP_HEIGHT; ++i)
                        {
                            for (int j = 0; j < MAP_WIDTH; ++j)
                            {
                                if(tiles[i][j].blockType == BLOCKTYPE::LIGHT)
                                {
                                    tiles[i][j].blockType = BLOCKTYPE::AIR;
                                
                                    lightRemovalBfsQueue.emplace(j, i, tiles[i][j].lightValue);
                                    setTorchlight(j, i, 0);
                                }
                            }
                        }
                        
                        if(tiles[mouseY/TILE_SIZE][mouseX/TILE_SIZE].blockType == BLOCKTYPE::AIR)
                        {
                            tiles[mouseY/TILE_SIZE][mouseX/TILE_SIZE].blockType = BLOCKTYPE::LIGHT;
                            tiles[mouseY/TILE_SIZE][mouseX/TILE_SIZE].lightValue = MAX_LIGHT_INTENSITY;
                        }
                    }
                    if(event.key.keysym.sym == SDLK_PLUS)
                    {
                        if(currentSkyLight < MAX_LIGHT_INTENSITY)
                            ++currentSkyLight;
                    }
                    if(event.key.keysym.sym == SDLK_MINUS)
                    {
                        if(currentSkyLight > 0)
                            --currentSkyLight;
                    }
                    
                    if(event.key.keysym.sym == SDLK_d)
                        displaySolids = 128;
                    break;
                }
                case SDL_KEYUP:
                {
                    if(event.key.keysym.sym == SDLK_d)
                        displaySolids = 0;
                    
                    break;
                }
            }            
            
            for (int i = 0; i < MAP_HEIGHT; ++i)
            {
                for (int j = 0; j < MAP_WIDTH; ++j)
                {
                    if(tiles[i][j].blockType != BLOCKTYPE::LIGHT)
                    {
                        tiles[i][j].lightValue = 0;
                        tiles[i][j].sunLight = 0;
                    }
                    else
                    {
                        lightBfsQueue.emplace(j,i);
                    }
                }
            }
            
            for (int i = 0; i < MAP_WIDTH; i++)
            {
                
                tiles[0][i].sunLight = currentSkyLight;
                sunlightBfsQueue.emplace(i, 0);
            }
            
            // SUNLIGHT --------------
            while(sunlightBfsQueue.empty() == false)
            {
                LightNode& node = sunlightBfsQueue.front();
                
                int nx = node.x; // node x
                int ny = node.y; // node y
                
                sunlightBfsQueue.pop();
                
                int lightLevel = getSunlight(nx, ny);
                
                if (nx + 1 < MAP_WIDTH && getSunlight(nx + 1, ny) + 2 <= lightLevel)
                {
                    if(tiles[ny][nx+1].blockType != SOLID)
                    {
                        setSunlight(nx + 1, ny, clamp(lightLevel - 1, 0, MAX_LIGHT_INTENSITY));
                    }
                    else
                        setSunlight(nx + 1, ny, clamp(lightLevel - 3, 0, MAX_LIGHT_INTENSITY));

                    sunlightBfsQueue.emplace(nx + 1, ny);

                }

                if (nx - 1 >= 0 && getSunlight(nx - 1, ny) + 2 <= lightLevel)
                {
                    if(tiles[ny][nx - 1].blockType != SOLID)
                    {
                        setSunlight(nx - 1, ny, clamp(lightLevel - 1, 0, MAX_LIGHT_INTENSITY));
                    }
                    else
                        setSunlight(nx - 1, ny, clamp(lightLevel - 3, 0, MAX_LIGHT_INTENSITY));
                    sunlightBfsQueue.emplace(nx - 1, ny);

                }

                if (ny -1 >= 0 && getSunlight(nx, ny - 1) + 2 <= lightLevel)
                {
                    if(tiles[ny-1][nx].blockType != SOLID)
                    {
                        setSunlight(nx, ny - 1, clamp(lightLevel - 1, 0, MAX_LIGHT_INTENSITY));
                    }
                    else
                        setSunlight(nx, ny - 1, clamp(lightLevel - 3, 0, MAX_LIGHT_INTENSITY));

                    sunlightBfsQueue.emplace(nx, ny - 1);

                }
                
                if (ny + 1 < MAP_HEIGHT && getSunlight(nx, ny + 1) <= lightLevel)
                {
                    if(tiles[ny + 1][nx].blockType != SOLID)
                    {
                        if(getSunlight(nx, clamp(ny - 1, 0, HEIGHT)) < currentSkyLight)
                            setSunlight(nx, ny + 1, lightLevel - 1);
                        else
                            setSunlight(nx, ny + 1, lightLevel);
                    }
                    else
                    {
                        setSunlight(nx, ny + 1, clamp(lightLevel - 3, 0, MAX_LIGHT_INTENSITY));
                    }
                    
                    sunlightBfsQueue.emplace(nx, ny + 1);
                }
            }
            
            // TORCHLIGHT --------------
            while(lightBfsQueue.empty() == /**/false)
            {
                LightNode& node = lightBfsQueue.front();
                
                int nx = node.x; // node x
                int ny = node.y; // node y
                
                lightBfsQueue.pop();
                
                int lightLevel = getTorchlight(nx, ny);
                
                
                if (nx + 1 < MAP_WIDTH && getTorchlight(nx + 1, ny) + 2 <= lightLevel)
                {
                    if(tiles[ny][nx+1].blockType != SOLID)
                    {
                        setTorchlight(nx + 1, ny, lightLevel - 1);
                    }
                    else
                        setTorchlight(nx + 1, ny, clamp(lightLevel - 2, 0, MAX_LIGHT_INTENSITY));
                    lightBfsQueue.emplace(nx + 1, ny);
                    
                    
                }
                
                if (nx - 1 >= 0 && getTorchlight(nx - 1, ny) + 2 <= lightLevel)
                {
                    if(tiles[ny][nx - 1].blockType != SOLID)
                    {
                        setTorchlight(nx - 1, ny, lightLevel - 1);
                    }
                    else
                        setTorchlight(nx - 1, ny, clamp(lightLevel - 2, 0, MAX_LIGHT_INTENSITY));
                    lightBfsQueue.emplace(nx - 1, ny);
                    
                }
                
                if (ny -1 >= 0 && getTorchlight(nx, ny - 1) + 2 <= lightLevel)
                {
                    if(tiles[ny-1][nx].blockType != SOLID)
                    {
                        setTorchlight(nx, ny - 1, lightLevel - 1);
                    }
                    else
                        setTorchlight(nx, ny - 1, clamp(lightLevel - 2, 0, MAX_LIGHT_INTENSITY));
                    
                    lightBfsQueue.emplace(nx, ny - 1);
                    
                }
                
                if (ny + 1 < MAP_HEIGHT && getTorchlight(nx, ny + 1) + 2 <= lightLevel)
                {
                    if(tiles[ny+1][nx].blockType != SOLID)
                    {
                        setTorchlight(nx, ny + 1, lightLevel - 1);
                    }
                    else
                        setTorchlight(nx, ny + 1, clamp(lightLevel - 2, 0, MAX_LIGHT_INTENSITY));
                    lightBfsQueue.emplace(nx, ny + 1);
                    
                }
            }
            
        }
        const Uint64 end = SDL_GetPerformanceCounter();
        
        
        
//        while(lightRemovalBfsQueue.empty() == false)
//        {
//            LightRemovalNode& node = lightRemovalBfsQueue.front();
//            int nx = node.x;
//            int ny = node.y;
//            int nv = node.val;
//            lightRemovalBfsQueue.pop();
//
//            int neighborLevel = getTorchlight(nx - 1, ny);
//            if(nx - 1 >= 0 && neighborLevel != 0 && neighborLevel < nv)
//            {
//                setTorchlight(nx - 1, ny, 0);
//                lightRemovalBfsQueue.emplace(nx - 1, ny, neighborLevel);
//                std::cout << "light remove call\n";
//            }
//            else if(nx - 1 >= 0 && neighborLevel >= nv)
//            {
//                lightBfsQueue.emplace(nx - 1, ny);
//            }
//
//            neighborLevel = getTorchlight(nx + 1, ny);
//            if(nx + 1 < MAP_WIDTH && neighborLevel != 0 && neighborLevel < nv)
//            {
//                setTorchlight(nx + 1, ny, 0);
//                lightRemovalBfsQueue.emplace(nx + 1, ny, neighborLevel);
//                std::cout << "light remove call\n";
//            }
//            else if(nx + 1 < MAP_WIDTH && neighborLevel >= nv)
//            {
//                lightBfsQueue.emplace(nx + 1, ny);
//            }
//
//            neighborLevel = getTorchlight(nx, ny + 1);
//            if(ny + 1 < MAP_HEIGHT && neighborLevel != 0 && neighborLevel < nv)
//            {
//                setTorchlight(nx, ny + 1, 0);
//                lightRemovalBfsQueue.emplace(nx, ny + 1, neighborLevel);
//                std::cout << "light remove call\n";
//            }
//            else if(ny + 1 < MAP_HEIGHT && neighborLevel >= nv)
//            {
//                lightBfsQueue.emplace(nx, ny + 1);
//            }
//            neighborLevel = getTorchlight(nx, ny - 1);
//            if(ny - 1 >= 0 && neighborLevel != 0 && neighborLevel < nv)
//            {
//                setTorchlight(nx, ny - 1, 0);
//                lightRemovalBfsQueue.emplace(nx, ny - 1, neighborLevel);
//                std::cout << "light remove call\n";
//            }
//            else if(ny - 1 >= 0 && neighborLevel >= nv)
//            {
//                lightBfsQueue.emplace(nx, ny - 1);
//            }
//        }
//        
        
       

        
        
        
        
        
        // Clear
        SDL_RenderClear(renderer);
        uint32_t* colors = (uint32_t*)surface->pixels;
        
        
        // Draw
        for (int i = 0; i < MAP_HEIGHT; ++i)
        {
            for (int j = 0; j < MAP_WIDTH; ++j)
            {
                int index = (i * TILE_SIZE * WIDTH + j * TILE_SIZE);
                //lightValue = clamp(tiles[i][j].lightValue * (255/MAX_LIGHT_INTENSITY) + tiles[i][j].sunLight * (255/MAX_LIGHT_INTENSITY), 0, 255);
                int torchValue = clamp(tiles[i][j].lightValue * (255/MAX_LIGHT_INTENSITY),0,255);
                int sunValue = clamp(tiles[i][j].sunLight * (255/MAX_LIGHT_INTENSITY),0,255);
                lightValue = max(torchValue, sunValue);
                
                int color = 0x00000000;
                
                switch(tiles[i][j].blockType)
                {
                    case SOLID:
                    {
                        color = (displaySolids << 24) + (clamp(lightValue*1.5,0,255) << 16) + ((int)(lightValue/2.5) << 8) + 0xff;
                        break;
                    }
                    case AIR:
                    {
                        int airLightVal = clamp((int)pow(lightValue, 1.05), 0, 255);
                        color =
                        (clamp(airLightVal / 2, 0, 255)     << 24) +
                        (clamp(airLightVal / 1.3, 0, 255)   << 16) +
                        (clamp(airLightVal, 0, 255)         << 8) +
                        0xff;
                        break;
                    }
                    case LIGHT:
                    {
                        color = (0xff << 24) + (0xff << 16) + (0x00 << 8) + 0xff;
                        break;
                    }
                }
                
                for(int k = 0; k < TILE_SIZE; k++)
                {
                    for (int m = 0; m < TILE_SIZE; m++)
                    {
                        colors[index + (k*WIDTH)+m] = color;
                    }
                }
            }
        }
        
        for(int i = 0; i < TILE_SIZE; i++)
        {
            for (int j = 0; j < TILE_SIZE; j++)
            {
                if(i == 0 || i == TILE_SIZE - 1 || j == 0 || j == TILE_SIZE-1)
                {
                    int index = (mouseX - mouseX % TILE_SIZE) + (mouseY - mouseY % TILE_SIZE) * WIDTH + i + j * WIDTH;
                    colors[index] = 0xff0000ff;
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
        rect = {
            mouseX - mouseX % TILE_SIZE,  // x
            mouseY - mouseY % TILE_SIZE,  // y
            TILE_SIZE,                    // width
            TILE_SIZE                     // height
        };
        SDL_RenderDrawRect(renderer, &rect);
        
        
        
        
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture);
        
        const static Uint64 freq = SDL_GetPerformanceFrequency();
        const double seconds = ( end - start ) / static_cast< double >( freq );
        std::cout << "Frame time: " << seconds * 1000.0 << "ms" << std::endl;
        
    }
    
    // Clear resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

int clamp(int val, int min, int max)
{
    if(val > max)
        return max;
    if(val < min)
        return min;
    
    return val;
}

int max(int val1, int val2)
{
    if(val1 > val2)
        return val1;
    
    return val2;
}
int min(int val1, int val2)
{
    if(val1 < val2)
        return val1;
    
    return val2;
}

/*
 Hi, could you help me out a little?
 Why is `SDL_SetRenderDrawColor` so slow?
 The more often the colors get changed the slower it gets.
 I am very new to sdl, so I don't really now what's going on.
 
 
 */
