#include "chip8.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string.h>

Chip8::Chip8()
{
    std::ifstream f("test_opcode.ch8", std::ios::binary | std::ios::in);

    char c;
    for (int i = 0x200; f.get(c); i++)
    {
        m_Memory[i] = (uint8_t)c;
    }

    SDL_Init(SDL_INIT_EVERYTHING);
    m_SDLScreen = SDL_SetVideoMode(64 * m_SurfaceScale, 32 * m_SurfaceScale, 32, SDL_HWSURFACE);
    SDL_Event event;
    int quit = 0;

    m_PC = 0x200;
}

void Chip8::updateScreen()
{
    SDL_Rect rect;

    for (unsigned int i = 0; i < 32; i++)
    {
        for (unsigned int j = 0; j < 64; j++)
        {
            rect.x = j * m_SurfaceScale;
            rect.y = i * m_SurfaceScale;
            rect.h = m_SurfaceScale;
            rect.w = m_SurfaceScale;

            if (m_Screen[i][j] == 0)
            {
                SDL_FillRect(m_SDLScreen, &rect, SDL_MapRGB(m_SDLScreen->format, 0, 0, 0));
            }
            else
            {
                SDL_FillRect(m_SDLScreen, &rect, SDL_MapRGB(m_SDLScreen->format, 255, 255, 255));
            }
        }
    }
    SDL_Flip(m_SDLScreen);
}

uint16_t Chip8::fetch()
{
    uint16_t instruction = (m_Memory[m_PC] << 8) | m_Memory[m_PC + 1];
    m_PC += 2;
    return instruction;
}

void Chip8::draw(uint16_t instruction)
{
    uint8_t x = m_Registers[(instruction & X) >> 8];
    uint8_t y = m_Registers[(instruction & Y) >> 4];
    uint8_t n = instruction & N;

    for (uint8_t i = y; i < y + n; i++)
    {
        uint8_t byte = m_Memory[m_I + (i - y)];
        for (uint8_t j = x; j < x + 8; j++)
        {
            bool bit = byte & (0x1 << (7 - (j - x)));
            m_Screen[i][j] = m_Screen[i][j] ^ bit;
            if (m_Screen[i][j] == 0)
            {
                m_Registers[VF] = 1;
            }
        }
    }
}

void Chip8::printScreen()
{
    for (unsigned int i = 0; i < 32; i++)
    {
        for (unsigned int j = 0; j < 64; j++)
        {
            std::cout << m_Screen[i][j];
        }
        std::cout << std::endl;
    }
}

void Chip8::conditional(bool condition)
{
    if (condition)
    {
        m_PC += 2;
    }
}

void Chip8::execute(uint16_t instruction)
{
    uint8_t category = (instruction & 0xF000) >> 12;
    switch (category)
    {
    case 0x0:
        switch (instruction)
        {
        case 0x00E0:
            // CLEAR
            memset(m_Screen, 0, sizeof(m_Screen));
            updateScreen();
            break;
        case 0x00EE:
            // RETURN FROM SUBROUTINE
            m_PC = m_Stack.back();
            m_Stack.pop_back();
            break;
        }

    case 0x1:
        // JUMP
        m_PC = instruction & NNN;
        break;
    case 0x2:
        // CALL SUBROUTINE
        m_Stack.push_back(m_PC);
        m_PC = instruction & NNN;
        break;
    case 0x3:
        // EQUAL CONDITIONAL SKIP
        conditional((instruction & NN) == m_Registers[(instruction & X) >> 8]);
        break;
    case 0x4:
        // NOT EQUAL CONDITIONAL SKIP
        conditional((instruction & NN) != m_Registers[(instruction & X) >> 8]);
        break;
    case 0x5:
        // VX VY EQUAL CONDITIONAL SKIP
        conditional(m_Registers[(instruction & Y) >> 4] == m_Registers[(instruction & X) >> 8]);
        break;
    case 0x6:
        m_Registers[(instruction & X) >> 8] = instruction & NN;
        break;
    case 0x7:
        m_Registers[(instruction & X) >> 8] += instruction & NN;
        break;
    case 0x9:
        // VX VY NOT EQUAL CONDITIONAL SKIP
        conditional(m_Registers[(instruction & Y) >> 4] != m_Registers[(instruction & X) >> 8]);
        break;
    case 0x8:
        // ARITHMETIC INSTRUCTIONS
        switch (instruction & N)
        {
        case 0x0:
            // SET
            m_Registers[(instruction & X) >> 8] = m_Registers[(instruction & Y) >> 4];
            break;
        case 0x1:
            // Binary OR
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & X) >> 8] | m_Registers[(instruction & Y) >> 4];
            break;
        case 0x2:
            // Binary AND
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & X) >> 8] & m_Registers[(instruction & Y) >> 4];
            break;
        case 0x3:
            // Binary XOR
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & X) >> 8] ^ m_Registers[(instruction & Y) >> 4];
            break;
        case 0x4:
            // ADD
            m_Registers[VF] =
                ((uint16_t)m_Registers[(instruction & X) >> 8] + (uint16_t)m_Registers[(instruction & Y) >> 4]) > 0xFF;
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & X) >> 8] + m_Registers[(instruction & Y) >> 4];
            break;
        case 0x5:
            // VX - VY
            m_Registers[VF] = m_Registers[(instruction & X) >> 8] > m_Registers[(instruction & Y) >> 4];
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & X) >> 8] - m_Registers[(instruction & Y) >> 4];
            break;
        case 0x6:
            // RIGHT SHIFT
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & Y) >> 4]; //! ONLY SOME IMPLEMENTATIONS USE THIS
            m_Registers[VF] = m_Registers[(instruction & X) >> 8] & 0x1;
            m_Registers[(instruction & X) >> 8] >>= 1;
            break;
        case 0x7:
            // VY - VX
            m_Registers[VF] = m_Registers[(instruction & X) >> 8] < m_Registers[(instruction & Y) >> 4];
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & Y) >> 4] - m_Registers[(instruction & X) >> 8];
            break;
        case 0xE:
            // LEFT SHIFT
            m_Registers[(instruction & X) >> 8] =
                m_Registers[(instruction & Y) >> 4]; //! ONLY SOME IMPLEMENTATIONS USE THIS
            m_Registers[VF] = m_Registers[(instruction & X) >> 8] & 0x80;
            m_Registers[(instruction & X) >> 8] <<= 1;
            break;
        }
        break;
    case 0xA:
        m_I = instruction & NNN;
        break;
    case 0xB:
        m_PC = (instruction & NNN) + m_Registers[0];
        break;
    case 0xC:
        m_Registers[(instruction & X) >> 8] = rand() & (instruction & NN);
        break;
    case 0xD:
        draw(instruction);
        updateScreen();
        break;
    case 0xF:
        switch (instruction & NN)
        {
        case 0x1E:
            // ADD TO INDE
            m_I += m_Registers[(instruction & X) >> 8];
            break;
        case 0x55:
            for (unsigned int i = 0; i <= (instruction & X) >> 8; i++)
            {
                m_Memory[m_I + i] = m_Registers[i];
            }
            break;
        case 0x65:
            for (unsigned int i = 0; i <= (instruction & X) >> 8; i++)
            {
                m_Registers[i] = m_Memory[m_I + i];
            }
            break;
        default:
            std::cout << "instruction " << instruction << " not implemented!" << std::endl;
            break;
        }
        break;
    default:
        std::cout << "instruction " << instruction << " not implemented!" << std::endl;
        break;
    }
}

void Chip8::run()
{
    while (true) // TODO: 700x per second
    {
        execute(fetch());
        SDL_Delay(1);
    }
}
