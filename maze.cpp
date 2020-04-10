#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <string>
#include <string.h>
#include <SFML/Graphics.hpp>

#include "maze.hpp"

#define VERBOSE

namespace maze
{
/***************************************
// Node                               //
***************************************/
#pragma region Node

Node::Node(uint8_t bin)
{
    _bin = bin;
}

#define temp(name, index)                        \
    bool Node::name(uint8_t val = 2)             \
    {                                            \
        if (val == 2)                            \
            return (_bin & (1 << (index))) == 1; \
        if (val == 0)                            \
        {                                        \
            _bin &= ~(1 << (index));             \
            return false;                        \
        }                                        \
        if (val == 1)                            \
        {                                        \
            _bin |= (1 << (index));              \
            return true;                         \
        }                                        \
        return false;                            \
    }

temp(north, 0);
temp(east, 1);
temp(south, 2);
temp(west, 3);

#undef temp

uint8_t Node::bin()
{
    return _bin;
}

Node::operator std::string()
{
    char buf[5];
    sprintf(buf, "%c%c%c%c",
            ((_bin & 0b1000) ? '1' : '0'),
            ((_bin & 0b0100) ? '1' : '0'),
            ((_bin & 0b0010) ? '1' : '0'),
            ((_bin & 0b0001) ? '1' : '0'));
    return std::string(buf);
}

// Node end
#pragma endregion

/***************************************
// Maze                               //
***************************************/
#pragma region Maze

Maze::Maze(uint _w, uint _h)
{
    w = _w;
    h = _h;
}

void Maze::load(uint8_t *bin)
{
    uint l = w * h;
    field = (Node *)malloc(l * sizeof(Node));
    for (uint i = 0; i < l - 1; i += 2)
    {
        char element = bin[i / 2];
        field[i] = Node((element >> 4) & 0xf);
        field[i + 1] = Node(element & 0xf);
    }
    if (l % 2 == 1)
        field[l - 1] = Node(bin[l / 2] >> 4);
}

uint8_t *Maze::unload()
{
    uint l = w * h;
    uint8_t *bin = (uint8_t *)malloc(((l + 1) / 2) * sizeof(uint8_t));
    uint8_t byte;
    bool half = false;
    uint index = 0;
    for (uint y = 0; y < h; ++y)
    {
        for (uint x = 0; x < w; ++x)
        {
            if (!half)
            {
                byte = (field[y * w + x].bin()) << 4;
                half = true;
            }
            else
            {
                bin[index++] = byte | field[y * w + x].bin();
                half = false;
            }
        }
    }
    if (half)
    {
        bin[index] = byte;
    }
    free(field);
    field = NULL;
    return bin;
}

#pragma endregion // Maze end
} // namespace maze

void replace_wordargs(int argc, char **argv)
{
    for (uint i = 1; i < argc; ++i)
    {
        if (!strncmp(argv[i], "--", 2))
        {
            argv[i][1] = argv[i][2];
            argv[i][2] = '\0';
        }
    }
}

void print_help(uint8_t exit_code = 0)
{
    std::ostream *stream = exit_code ? &(std::cerr) : &(std::cout);
    *stream << "help";
    exit(exit_code);
}

int main(int argc, char **argv)
{
    std::string input_path = "";
    std::string output_path = "";
    uint width = 1;
    uint height = 1;
    bool bDisplay = false;
    bool bGenerate = false;

#pragma region Parse command line arguments
    // replace wordargs with their starting letters, i.e. "--help" will become "-h"
    replace_wordargs(argc, argv);

    int c;
    int parsed;
    uint8_t error = false;
    while ((c = getopt(argc, argv, "i:o:x:y:dgh")) != -1)
    {
        switch (c)
        {
        case 'i':
            input_path = optarg;
            break;

        case 'o':
            output_path = optarg;
            break;

        case 'x':
            parsed = atoi(optarg);
            width = std::clamp(parsed, 1, 1024);
            break;

        case 'y':
            parsed = atoi(optarg);
            height = std::clamp(parsed, 1, 1024);
            break;

        case 'd':
            bDisplay = true;
            break;

        case 'g':
            bGenerate = true;
            break;

        default:
            error = true;
        case 'h':
            print_help(error);
        }
    }

#ifdef VERBOSE

    std::cout
        << "input path: \"" << input_path << '"' << std::endl
        << "output path: \"" << output_path << '"' << std::endl
        << "size: " << width << 'x' << height << std::endl
        << "display: " << (bDisplay ? "true" : "false") << std::endl
        << "generate: " << (bGenerate ? "true" : "false") << std::endl;

#endif
#pragma endregion

    sf::RenderWindow window(sf::VideoMode(200, 200), "Floating");
    window.setTitle("Yay");

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return EXIT_SUCCESS;
}
