#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include <fstream>

#define TARGET_FPS 7
#define TARGET_FRAMETIME 1000 / TARGET_FPS

#define NUM_KEYS 256

#define COLUMNS 13
#define ROWS 23

#define BOXSIZE 35

#define WIDTH ROWS * BOXSIZE
#define HEIGHT COLUMNS * BOXSIZE

#define BLACK  0
#define BLUE   1 //reserved for walls
#define RED    2
#define GREEN  3
#define WHITE  4
#define YELLOW 5 //reserved for coin spawn position

/*TODO:
    - coin generation and pickup
    - enemy ghosts
    - gameover screen
    - sound effects
    - show flashing coins and timer
    - infinate gameloop
*/

// .bss
uint8_t screen [COLUMNS][ROWS]{{0}};

typedef struct {
    int x;
    int y;
}position;

enum class direction {
    forward = 0,
    backward = 1,
    left = 2,
    right = 3,
    none = 4
};

class player {
    private:
        position pos;
        direction dir;
        direction queued_dir;


    public:
        int points = 0;

    public:
        player(int x, int y) {
            pos.x = x;
            pos.y = y;

            queued_dir = direction::none;
            dir = direction::none;
        }

        inline position *get_pos() {
            return &this->pos;
        }

        inline direction get_dir() {
            return this->dir;
        }

        inline void set_dir(direction dir_src) {
            dir = dir_src;
        }

        //sets queued movement
        inline void move(direction dir_src) {
            //check if player uses portal
            if ((pos.y <= 7 && pos.y >= 5)) {
                if(pos.x <= -1)
                    pos.x = 22;
                
                if (pos.x >= 23) {
                    pos.x = 0;
                }
            }

            if (valid_movement(dir_src)) {
                move_dir(dir);
            } else if (valid_movement(dir)) move_dir(dir);
        
        }

        inline void move_dir(direction dir_src) {
            if (dir_src == direction::forward) pos.y--;
            if (dir_src == direction::backward) pos.y++;
            if (dir_src == direction::right) pos.x++;
            if (dir_src == direction::left) pos.x--;
        }

        bool valid_movement(direction dir_src) {
            switch(dir_src) {
                case direction::forward:
                    if (screen[pos.y-1][pos.x] != BLUE){
                        //pos.y--;
                        return true;
                    } else return false;
                    
                case direction::backward:
                    if (screen[pos.y + 1][pos.x] != BLUE){
                        //pos.y++;
                        return true;
                    } else return false;  

                case direction::right:
                if (pos.x+1 >= ROWS) return true;
                    if (screen[pos.y][pos.x + 1] != BLUE){
                        //pos.x++;
                        return true;
                    } else return false;

                case direction::left:
                    if (pos.y-1 <= -1) return true;
                    if (screen[pos.y][pos.x - 1] != BLUE){
                        //pos.x--;
                        return true;
                    } else return false;                        
            }
            return false;
        }
};

//assuming that map.txt is in the current directory
std::vector<std::vector<uint8_t>> fetch_map() {
    std::vector<std::vector<uint8_t>> buffer;
    std::ifstream fd("./map.txt", std::ios::binary);
    if (fd.is_open()) {
        std::string line;
        while (std::getline(fd, line)) {
            // create a new row in the buffer for each line
            std::vector<uint8_t> row;
            for (int i = 0; i<line.size(); i++) {
                uint8_t color = line[i] - '0';
                row.push_back(color);
            }
            buffer.push_back(row);
        }
    }

    fd.close();
    return buffer;
}

void draw_screen() {
    for (int i = 0; i<COLUMNS; i++) {
        for (int j = 0; j<ROWS; j++) {
            std::cout << "pos x" << i << "y" << j << (int)screen[i][j] << ' ';
        }
        std::cout << '\n';
    }
}

class w_window {
	private:
		SDL_Window *window;
		SDL_Renderer *renderer;

	
	public:
		w_window(const char *name) {
			if (SDL_Init(SDL_INIT_VIDEO) < 0) {
				SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
			}

			window = SDL_CreateWindow(
				name,
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				WIDTH,
				HEIGHT,
				SDL_WINDOW_SHOWN
			);

			if (window == nullptr) {
				SDL_Log("Window could not be created! SDL_Error%s\n", SDL_GetError());
			}

			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == nullptr) {
				SDL_Log("Renderer could not be created! SDL_ERROR%s\n", SDL_GetError());
			}
		
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    		SDL_RenderClear(renderer);
    		SDL_RenderPresent(renderer);
		}

		void render(player &p) {
            SDL_RenderPresent(renderer);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);	
            SDL_RenderClear(renderer);

            //FRAME GENERATION
            for (int i = 0; i<COLUMNS; i++) {
                for (int j = 0; j<ROWS; j++) {
                    SDL_Rect rect;
                    rect.x = j * BOXSIZE;
                    rect.y = i * BOXSIZE;
                    rect.w = BOXSIZE;
                    rect.h = BOXSIZE;

                    switch(screen[i][j]) {
                        case BLACK:
                            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                            break;

                        case WHITE:
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                            break;

                        case RED:
                            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                            break;

                        case GREEN:
                            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                            break;

                        case BLUE:
                            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                            break;
                        
                        case YELLOW: //reserved for coins, typically 1/3 the size of a normal tile

                            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                            
                            rect.x = j * BOXSIZE + (BOXSIZE * 0.4);
                            rect.y = i * BOXSIZE + (BOXSIZE * 0.4);
                            rect.w = BOXSIZE /5;
                            rect.h = BOXSIZE /5;
                            break;
                    }
                                            
                    SDL_RenderFillRect(renderer, &rect);


                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);	
                }
            }

            //render player, enemies, coins, etc
            position *player_pos = p.get_pos();

            //p.move(p.get_dir());
            
            SDL_Rect rect;
            // no gadamn clue why this works
            rect.x = player_pos->x * BOXSIZE;
            rect.y = player_pos->y * BOXSIZE;
            rect.w = BOXSIZE;
            rect.h = BOXSIZE;

            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
            SDL_RenderFillRect(renderer, &rect);


                //	SDL_RenderPresent(renderer);

            //SDL_UpdateWindowSurface(window);

    /*				
                    SDL_RenderPresent(renderer);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);	
                    SDL_UpdateWindowSurface(window);
    */

            //quit = true;

		//SDL_Delay(60000);
	}

    ~w_window() {
        SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
    }
};

class pacman {
    private:
        w_window window;
        
        SDL_Event event;
        bool running = true;

        player client;

    public:
        pacman(): window("pacman"), client(1, 1) {
            std::vector<std::vector<uint8_t>> map = fetch_map();
            for (int i = 0; i<map.size(); i++) {
                for (int j = 0; j<map[i].size(); j++) {
                    screen[i][j] = map[i][j];
                    std::cout << "x,y: " << i << ' ' << j << ' ' << (int)screen[i][j];

                }
                std::cout << '\n';
            }

        }

    void run() {
        uint32_t start_time = 0, end_time = 0, elapsed_time;
        position *pos = client.get_pos();
        
        while (running) {
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_KEYDOWN) {
                    start_time = SDL_GetTicks();
                    switch( event.key.keysym.sym ) {
                        case SDLK_UP: 
                            std::cout << "up\n";
                            client.set_dir(direction::forward);
                            break;
                        
                        case SDLK_DOWN: 
                            std::cout << "down\n";
                            client.set_dir(direction::backward);
                            break;
                        
                        case SDLK_LEFT: 
                            std::cout << "left\n";
                            client.set_dir(direction::left);
                            
                            break;
                        
                        case SDLK_RIGHT: 
                            std::cout << "right\n";
                            client.set_dir(direction::right);
                            break;

                        default:
                            std::cout << "idfk bro\n";
                            break; 
                        }
                    end_time = SDL_GetTicks();
                } 
                
            }
            client.move(client.get_dir());
            //words cannot explain my sheer confusion to as why in hell this works
            if (screen[pos->y][pos->x] == YELLOW) {
                screen[pos->y][pos->x] = BLACK;
                client.points++;
            }

            std::cout << "points: " << client.points << '\n';


            window.render(client);
            elapsed_time = end_time - start_time;
            if (elapsed_time < TARGET_FRAMETIME) {
                SDL_Delay(TARGET_FRAMETIME - elapsed_time); 
            }
        }
    }
};
