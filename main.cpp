#include <cstdint>
#include <SDL2/SDL.h>
#include <stack>
#include <array>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <random>
#include <map>
#include <string>

class chip8
{

public:
    bool debug_print {false};
    bool debug_step {false};

    bool memory_quirk {true};
    bool shift_quirk {true};
    bool jump_quirk {true};

    int instructions_per_second {700};

private:
    static const int screen_width {64};
    static const int screen_height {32};
    std::array<uint8_t,4096> memory{};
    std::array<uint8_t,screen_width * screen_height> display{};
    std::array<uint8_t,16> v{};  //register
    uint16_t pc{};
    uint16_t i{};
    std::stack<uint16_t> stack{};
    uint8_t delay{};
    uint8_t sound{};

    std::map<int,Uint8> keymap {
        {0x1,SDL_SCANCODE_1},
        {0x2,SDL_SCANCODE_2},
        {0x3,SDL_SCANCODE_3},
        {0xC,SDL_SCANCODE_4},
        {0x4,SDL_SCANCODE_Q},
        {0x5,SDL_SCANCODE_W},
        {0x6,SDL_SCANCODE_E},
        {0xD,SDL_SCANCODE_R}, 
        {0x7,SDL_SCANCODE_A},
        {0x8,SDL_SCANCODE_S},
        {0x9,SDL_SCANCODE_D},
        {0xE,SDL_SCANCODE_F},
        {0xA,SDL_SCANCODE_Z},
        {0x0,SDL_SCANCODE_X},
        {0xB,SDL_SCANCODE_C},
        {0xF,SDL_SCANCODE_V}
    };

    std::map<int,Uint8> keymap2 {
        {0x0,SDL_SCANCODE_1},
        {0x1,SDL_SCANCODE_2},
        {0x2,SDL_SCANCODE_3},
        {0x3,SDL_SCANCODE_4},
        {0x4,SDL_SCANCODE_Q},
        {0x5,SDL_SCANCODE_W},
        {0x6,SDL_SCANCODE_E},
        {0x7,SDL_SCANCODE_R}, 
        {0x8,SDL_SCANCODE_A},
        {0x9,SDL_SCANCODE_S},
        {0xA,SDL_SCANCODE_D},
        {0xB,SDL_SCANCODE_F},
        {0xC,SDL_SCANCODE_Z},
        {0xD,SDL_SCANCODE_X},
        {0xE,SDL_SCANCODE_C},
        {0xF,SDL_SCANCODE_V}
    };

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen_texture;

    bool set_pc(uint16_t pc) {
        if (pc >= 4096 or pc < 0 )
        {
            printf("PC value out of bounds: %d\n",pc);
            return false;
        }
        this->pc = pc;
        return true;
    }

public:
    chip8(/* args */);
    ~chip8();

    int SDLinit() {
        const int WINDOW_WIDTH = screen_width*10;
        const int WINDOW_HEIGHT = screen_height*10;
        if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0) {
            printf("SDL error : %s\n", SDL_GetError( ));
            return 1;
        }
        window = SDL_CreateWindow("test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            printf("SDL error : %s\n", SDL_GetError( ));
            SDL_Quit();
            return 1;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
        if (renderer == nullptr) {
            SDL_DestroyWindow(window);
            printf("SDL error : %s\n", SDL_GetError( ));
            SDL_Quit();
            return 1;
        }
        SDL_RenderSetLogicalSize(renderer,screen_width,screen_height);
        screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,screen_width,screen_height);
        if (screen_texture == nullptr) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            printf("SDL error : %s\n", SDL_GetError( ));
            SDL_Quit();
            return 1;
        }
        return 0;
    }

    bool load_ROM(std::string romf) {
        std::ifstream rom(romf,std::ios::binary);
        if (rom) {
            rom.seekg(0, std::ios::end);
            auto filesize = rom.tellg();
            rom.seekg(0, std::ios::beg);

            rom.read((char*)&memory[512], filesize);
        } else {
            printf("Failed to load ROM\n");
            return false;
        }
        return true;
    }

    void load_font() {
        std::array<uint8_t,16*5> fonts {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };
        int t = 0;
        for (auto f : fonts) {
            memory[0x50+t] = f;
            t++;
        }
    }

    void cleanup() {
        SDL_DestroyTexture(screen_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        printf("SDL closed\n");
    }

    void render() {
        std::array<unsigned int, screen_width*screen_height> pixels{};
        int pixelcount=0;
        for (auto& pixel : pixels) {
            pixel = (display[pixelcount]==0)?255:4294967295;
            pixelcount++;
        }
        SDL_UpdateTexture(screen_texture, NULL, &pixels, screen_width*4);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    static Uint32 delay_timer_callback(Uint32 interval, void* param) {
        uint8_t* d = static_cast<uint8_t*>(param);
        if(*d > 0) {
            *d = *d - 1;
        }
        return interval;
    }

    static Uint32 instruction_counter_callback(Uint32 interval, void* param) {
        int* t = static_cast<int*>(param);
        *t = 0;
        return interval;
    }

    void run(std::string rom) {
        SDLinit();
        if(!load_ROM(rom)) {
            cleanup();
            return;
        }
        load_font();
        set_pc(512);
        int instructions_done = 0;
        auto instruction_count_timer = SDL_AddTimer(1000,instruction_counter_callback,&instructions_done);
        auto delay_timer = SDL_AddTimer(16,delay_timer_callback,&delay);
        uint16_t instruction;
        SDL_Event e;
        bool quit = false;
        bool step_next = false;
        while(!quit) {
            while (SDL_PollEvent(&e)) {
                switch (e.type)
                {
                case SDL_QUIT:
                    quit = true;
                    break;
                
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_UP)
                    {
                        for (int j = 0; j < 16; j++)
                        {
                            printf("V[%x]:%d  ",j,v[j]);
                        }
                        printf("\n");
                        printf("I : %d\n",i);
                    } else if (e.key.keysym.sym == SDLK_RIGHT)
                    {
                        step_next = true;
                    }
                    
                    break;
                
                default:
                    break;
                }
            }
            if (instructions_done >= instructions_per_second) {
                continue;
            }
            if (debug_step) {
                if(!step_next) {
                    continue;
                }
            }
            instruction = memory[pc] << 8 | memory[pc + 1];
            if (debug_print)
            {
                printf("(%d) : %X\n",pc,instruction);
            }
            set_pc(pc + 2);
            emulate_cycle(instruction);
            instructions_done+=1;
            step_next = false;
        }
        SDL_RemoveTimer(instruction_count_timer);
        SDL_RemoveTimer(delay_timer);
        cleanup();
    }
    void emulate_cycle(uint16_t instruction)
    {
        uint8_t nibble1 = (instruction >> 12) & 0x000f;
        uint8_t nibble2 = (instruction >> 8) & 0x000f;
        uint8_t nibble3 = (instruction >> 4) & 0x000f;
        uint8_t nibble4 = (instruction >> 0) & 0x000f;

        uint8_t x = nibble2;
        uint8_t y = nibble3;
        uint8_t n = nibble4;
        uint8_t nn = instruction & 0x00ff;
        uint16_t nnn = instruction & 0x0fff;

        switch (nibble1)
        {
        case 0x0:
            if (instruction == 0x00E0)
            {
                // clear screen
                display.fill(0);
                render();
            }
            if (instruction == 0x00EE)
            {
                // subroutine return
                set_pc(stack.top());
                stack.pop();
            }
            break;

        case 0x1:
            // jump
            set_pc(nnn);
            break;

        case 0x2:
            // subroutine call
            stack.push(pc);
            set_pc(nnn);
            break;

        case 0x3:
            // skip if equal
            if (v[x] == nn)
            {
                set_pc(pc + 2);
            }
            break;

        case 0x4:
            // skip if not equal
            if (v[x] != nn)
            {
                set_pc(pc + 2);
            }
            break;

        case 0x5:
            // skip if equal
            if (v[x] == v[y])
            {
                set_pc(pc + 2);
            }
            break;

        case 0x6:
            // set register
            v[x] = nn;
            break;

        case 0x7:
            // add to register
            v[x] += nn;
            break;

        case 0x8:
            switch (nibble4)
            {
            case 0x0:
                // set
                v[x] = v[y];
                break;
            case 0x1:
                // bitwise or
                v[x] = v[x] | v[y];
                v[0xf] = 0;
                break;
            case 0x2:
                // bitwise and
                v[x] = v[x] & v[y];
                v[0xf] = 0;
                break;
            case 0x3:
                // XOR
                v[x] = v[x] ^ v[y];
                v[0xf] = 0;
                break;
            case 0x4: {
                // add
                int t = v[x] + v[y];
                int flag{};
                if (t > 255)
                {
                    // overflow
                    flag = 1;
                }
                else
                {
                    flag = 0;
                }
                v[x] = v[x] + v[y];
                v[0xf] = flag;
                break;
            }
            case 0x5: {
                // subtract x - y
                int flag{};
                if (v[x] >= v[y])
                {
                    flag = 1;
                }
                else
                {
                    flag = 0;
                }
                v[x] = v[x] - v[y];
                v[0xf] = flag;
                break;
            }
            case 0x6: {
                // right shift (ambiguous)
                if (shift_quirk)
                {
                    v[x] = v[y];
                }
                auto sb = v[x] & 0x1;
                v[x] = v[x] >> 1;
                v[0xf] = sb;
                break;
            }
            case 0x7: {
                // subtract y - x
                int flag{};
                if (v[y] >= v[x])
                {
                    flag = 1;
                }
                else
                {
                    flag = 0;
                }
                v[x] = v[y] - v[x];
                v[0xf] = flag;
                break;
            }
            case 0xE: {
                // left shift (ambiguous)
                if (shift_quirk)
                {
                    v[x] = v[y];
                }
                auto sb = v[x] & 0x80;
                v[x] = v[x] << 1;
                v[0xf] = (sb == 0) ? 0 : 1;
                break;
            }
            default:
                printf("Unidentified instruction %x\n", instruction);
                break;
            }
            break;

        case 0x9:
            // skip if not equal
            if (v[x] != v[y])
            {
                set_pc(pc + 2);
            }
            break;

        case 0xA:
            // set index
            i = nnn;
            break;

        case 0xB: {
            // jump with offset (ambiguous)
            if (jump_quirk)
            {
                set_pc(nnn + v[0]);
            }
            else
            {
                set_pc(nnn + v[x]);
            }
            break;
        }
        case 0xC: {
            // random
            std::random_device rd;
            std::default_random_engine generator(rd());
            std::uniform_int_distribution<uint8_t> distribution(0, 255);
            v[x] = distribution(generator) & nn;
            break;
        }
        case 0xD: {
            // draw
            auto x_pos = v[x] % screen_width;
            auto y_pos = v[y] % screen_height;
            v[0xf] = 0;
            for (int j = 0; j < n; j++)
            {
                auto sprite_row = memory[i + j];
                uint8_t mask = 128;
                for (int p = 0; p < 8; p++)
                {
                    auto sprite_pixel = sprite_row & mask;
                    if (sprite_pixel)
                    {
                        // need to flip
                        auto t = y_pos * screen_width + x_pos;
                        if (display[t] == 0)
                        {
                            display[t] = 1;
                        }
                        else
                        {
                            display[t] = 0;
                            v[0xf] = 1;
                        }
                    }
                    mask = mask >> 1;
                    x_pos += 1;
                    if (x_pos >= screen_width)
                    {
                        break;
                    }
                }
                y_pos += 1;
                x_pos = v[x] % screen_width;
                if (y_pos >= screen_height)
                {
                    break;
                }
            }
            render();
            break;
        }
        case 0xE: {
            // skip if key
            const Uint8 *key_states = SDL_GetKeyboardState(NULL);
            auto keypress = key_states[keymap[v[x]]];
            if (nn == 0x9E)
            {
                // skip if key pressed (EX9E)
                if (keypress == 1)
                {
                    set_pc(pc + 2);
                }
            }
            if (nn == 0xA1)
            {
                // skip if key not pressed (EXA1)
                if (keypress == 0)
                {
                    set_pc(pc + 2);
                }
            }
            break;
        }
        case 0xF:
            switch (nn)
            {
            case 0x07:
                // set vx to delay
                v[x] = delay;
                break;
            case 0x15:
                // delay timer
                delay = v[x];
                break;
            case 0x18:
                // sound timer
                sound = v[x];
                break;
            case 0x1E:
                // add to index
                i += v[x];
                if (i > 0x0FFF)
                {
                    v[0xf] = 1;
                }
                break;
            case 0x0A: {
                // get key
                const Uint8 *key_states = SDL_GetKeyboardState(NULL);
                for (auto const &[keyval, keyscan] : keymap)
                {
                    if (key_states[keyscan])
                    {
                        v[x] = keyval;
                        set_pc(pc + 2); // move to next instruction
                        break;
                    }
                }
                set_pc(pc - 2); // loop till key press
                break;
            }
            case 0x29: {
                // font
                auto c = v[x] & 0x0F; // last nibble
                i = 0x50 + (c * 5);   // each sprite 5 pixels tall
                break;
            }
            case 0x33: {
                // binary coded decimal
                uint8_t t = v[x];
                memory[i + 2] = t % 10; // last digit
                t = t / 10;
                memory[i + 1] = t % 10;
                t = t / 10;
                memory[i] = t; // first digit
                break;
            }
            case 0x55: {
                // store memory
                for (int j = 0; j <= x; j++)
                {
                    memory[i + j] = v[j];
                }
                if (memory_quirk)
                {
                    i = i + x + 1;
                }
                break;
            }
            case 0x65: {
                // load memory
                for (int j = 0; j <= x; j++)
                {
                    v[j] = memory[i + j];
                }
                if (memory_quirk)
                {
                    i = i + x + 1;
                }
                break;
            }
            default:
                printf("Unindentified instruction %x\n", instruction);
                break;
            }
            break;
        default:
            printf("Unidentified instruction %x\n", instruction);
            break;
        }
    }
};

chip8::chip8()
{
}

chip8::~chip8()
{
}


int main(int argc, char* args[]) {
    chip8 emulator;
    //emulator.debug_print = true;
    //emulator.debug_step = true;
    std::string rom {"roms/IBMlogo.ch8"};
    if(args[1]) {
        rom=std::string("roms/") + args[1] + ".ch8";
    }
    emulator.run(rom);
    return 0;
}