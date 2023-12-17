#ifndef PACMAN_MAIN
#define PACMAN_MAIN

#include <iostream>
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include <time.h>
#include <fstream>

#define f32 float
#define f64 double

#define TARGET_FPS 60.0

#define TARGET_FRAMETIME 1000.0 / TARGET_FPS
#define DELTA_TIME TARGET_FPS / 1000.0

#define NUM_KEYS 256

#define COLUMNS 13
#define ROWS 23

#define BOXSIZE 35

#define WIDTH ROWS * BOXSIZE
#define HEIGHT COLUMNS * BOXSIZE

#define BLUE   1 //reserved for walls

//freely usable colors
#define BLACK  0
#define WHITE  4
#define RED    2
#define GREEN  3
#define CYAN   6 //rgb(0, 255, 255)
#define ORANGE 7 //rgb(255, 153, 51)
#define PINK   8 //rgb(255, 153, 51)
#define YELLOW 5

#define COIN_SPAWN 'C'
#define PLAYER_SPAWN 'P'
#define ENEMY_SPAWN 'E'

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

template <typename TA, typename TB>
void swap(TA *src, TB *dir) {
    TA tmp = *src;
    *src = *dir;
    *dir = tmp;
}

// i don't know why but for some reason the normal floor returns a type double
inline int _floor( const f32 src )
{
  return static_cast< int >( std::floor(src) );
}

inline int _round( const f32 src )
{
  return static_cast< int >( std::floor(src + 0.5) );
}

enum class direction {
    forward = 0,
    backward = 1,
    left = 2,
    right = 3,
    none = 4
};

typedef struct {
    f32 x;
    f32 y;
}position;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
}color;

bool valid_movement(direction dir_src, position &pos) {
    switch(dir_src) {
        case direction::forward:
            if (screen[_round(pos.y - 0.5)][_round(pos.x)] != BLUE){
                        //pos.y--;
                return true;
            } else return false;
                    
        case direction::backward:
            if (screen[_round(pos.y + 0.5)][_round(pos.x)] != BLUE){
                return true;
            } else return false;  

        case direction::right:
            if (pos.x+0.5 >= ROWS-1) return true;
            if (screen[_round(pos.y)][_round(pos.x + 0.5)] != BLUE){

                return true;
            } else return false;

        case direction::left:
            if (pos.x-0.5 <= 0) return true;
            if (screen[_round(pos.y)][_round(pos.x - 0.5)] != BLUE){
                //pos.x--;
                return true;
            } else return false;                        
    }
    return false;
}

typedef struct ghost{
    position pos;
    direction dir;
    color clr;

    ghost(int x, int y) {
        pos.x = x;
        pos.y = y;

        clr.r = rand() % 256;
        clr.g = rand() % 256;
        clr.b = rand() % 256;
    }

}ghost;

//enemy class encapsulates all of the enemies 
class enemy { 
    private:
        std::vector<ghost*> actors;

    public:
        void add_enemy(f32 x, f32 y) {
            ghost *buffer = new ghost(x, y);
            actors.push_back(buffer);
        }

        ~enemy() {
            for (int i = 0; i<actors.size(); i++) {
                delete actors[i];
            }
        }
};

class player {
    private:
        bool is_alive;
        direction dir;
        direction alt_dir;

        direction queued_dir;


    public:
        int points = 0;
        position pos;

    public:
        player() {
            queued_dir = direction::none;
            dir = direction::none;
        }

        inline position *get_pos() {
            return &this->pos;
        }

        inline direction &get_dir() {
            return this->dir;
        }

        inline direction &get_alt() {
            return this->alt_dir;
        }

        inline void set_dir(direction dir_src) {
            alt_dir = dir;
    
            dir = dir_src;
        }

        inline void set_alt_dir(direction dir_src) {
            dir = alt_dir;
        }

        //sets queued movement
        inline void move(direction &dir_src) {
            if ((pos.y <= 7 && pos.y >= 5)) {   //check if player uses portal
                if(pos.x <= -1.0 && dir == direction::left)
                    pos.x = 23.0;
                
                if (pos.x >= 23.0 && dir == direction::right) {
                    pos.x = -1.0;
                }
            }

            if (valid_movement(dir_src, this->pos)) {
                move_dir(dir);
            } else if (valid_movement(alt_dir, this->pos)) {
                move_dir(alt_dir);
                //alt_dir = dir;
                //alt_dir = direction::none;
            } else {
                std::cout << "Fuck off cunt\n";
                dir_src = direction::none;
                alt_dir = direction::none;
            }
        }

        inline void move_dir(direction dir_src) {
            if (dir_src == direction::forward) {
                 pos.y -= DELTA_TIME;
                 pos.x = _round(pos.x);
            } else

            if (dir_src == direction::backward)  {
                pos.y += DELTA_TIME;
                pos.x = _round(pos.x);
            } else
            if (dir_src == direction::right)  {
                pos.x += DELTA_TIME;
                pos.y = _round(pos.y);
            } else

            if (dir_src == direction::left) {
                pos.x -= DELTA_TIME;
                pos.y = _round(pos.y);
            }

            std::cout << "X: " << (f32)pos.x << ", Y: " << (f32)pos.y << '\n';
            std::cout << "DELTA TIME: " << DELTA_TIME << '\n';
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
                uint8_t color;
                
                if (line[i] >= '0' && line[i] >= '9')
                    color = line[i];
                else 
                    color = line[i] - '0';
                

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
            std::cout << "pos x, " << i << "y " << j << (int)screen[i][j] << ' ';
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

        //player and enemy positions are passed as coin positions are static
		void render(player &p, enemy &e) {
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

                        case CYAN:
                            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 0);
                            break;

                        case PINK:
                            SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
                            break;

                        case ORANGE: 
                            SDL_SetRenderDrawColor(renderer, 255, 102, 0, 0);
                            break;
                        
                        case YELLOW:
                            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

                            break;
                    
                        case COIN_SPAWN:
                            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                            
                            rect.x = j * BOXSIZE + ((BOXSIZE * 0.5) - BOXSIZE/10);
                            rect.y = i * BOXSIZE + ((BOXSIZE * 0.5) - BOXSIZE/10);
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

            SDL_Rect rect;
            rect.x = player_pos->x * BOXSIZE + ((BOXSIZE * 0.75) - BOXSIZE/1.65);
            rect.y = player_pos->y * BOXSIZE + ((BOXSIZE * 0.75) - BOXSIZE/1.65);
            rect.w = BOXSIZE * 0.75;
            rect.h = BOXSIZE * 0.75;

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
        enemy ghosts;

    public:
        pacman(): window("pacman"){
            std::vector<std::vector<uint8_t>> map = fetch_map();
            
            srand(time(NULL));

            // map preprocessing to get enemy, player and coin spawn positions
            for (int i = 0; i<map.size(); i++) {
                for (int j = 0; j<map[i].size(); j++) {
                    switch(map[i][j]) {
                        case 'P': //player spawn position
                            client.pos.x = j;
                            client.pos.y = i;
                            break;

                        case 'E':
                            ghosts.add_enemy(j, i);

                            break;
                    }
                    
                    screen[i][j] = map[i][j];
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
                        //    if (client.valid_movement(direction::forward))
                                client.set_dir(direction::forward);
                   //         else
                     //           client.set_alt_dir(direction::forward);
                            
                            break;
                        
                        case SDLK_DOWN: 
                            std::cout << "down\n";
                            //if (client.valid_movement(direction::backward))
                                client.set_dir(direction::backward);
                            //else
                            //    client.set_alt_dir(direction::backward);
                            
                            break;
                        
                        case SDLK_LEFT: 
                            std::cout << "left\n";
                            //if (client.valid_movement(direction::left))
                                client.set_dir(direction::left);
                            //else
  //                              client.set_alt_dir(direction::left);
                            
                            break;
                        
                        case SDLK_RIGHT: 
                            std::cout << "right\n";
    //                        if (client.valid_movement(direction::right))
                                client.set_dir(direction::right);
      //                      else 
        //                        client.set_alt_dir(direction::right);
                            
                            break;

                        default:
                            std::cout << "idfk bro\n";
                            break; 
                        }

                end_time = SDL_GetTicks();
                } 
                
            }
            
            // game logic
            client.move(client.get_dir());
            if (screen[_floor(pos->y)][_floor(pos->x)] == COIN_SPAWN) {
                screen[_floor(pos->y)][_floor(pos->x)] = BLACK;
                client.points++;
            }

            window.render(client, ghosts);
            elapsed_time = end_time - start_time;
            if (elapsed_time < TARGET_FRAMETIME) {
                SDL_Delay(TARGET_FRAMETIME - elapsed_time); 
            }
        }
    }
};

#endif
