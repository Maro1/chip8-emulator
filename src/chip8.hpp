#include <bitset>
#include <stdint.h>
#include <vector>

#include "SDL/SDL.h"

#define X 0x0F00
#define Y 0x00F0
#define N 0x000F
#define NN 0x00FF
#define NNN 0x0FFF
#define VF 0xF

class Chip8
{
public:
    Chip8();

    void run();

private:
    uint16_t fetch();
    void execute(uint16_t instruction);
    void updateScreen();
    void printScreen();

    void draw(uint16_t instruction);
    void conditional(bool condition);

    uint8_t m_Memory[4096] = {0};
    bool m_Screen[32][64] = {0};

    std::vector<uint16_t> m_Stack;

    uint8_t m_Registers[16];
    uint16_t m_PC;
    uint16_t m_I;

    SDL_Surface* m_SDLScreen = nullptr;
    unsigned int m_SurfaceScale = 10;
};
