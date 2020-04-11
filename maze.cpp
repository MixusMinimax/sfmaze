#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <string>
#include <string.h>
#include <getopt.h>
#include <filesystem>
#include <iosfwd>
#include <tuple>
#include <SFML/Graphics.hpp>

#include "maze.hpp"

#define MAX_WIDTH 1000
#define MAX_HEIGHT 700

static int verbose_flag;
static std::string input_path = "";
static std::string output_path = "";
static bool bDisplay = false;
static bool bGenerate = false;

#pragma region namespace maze
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
            return ((_bin >> (index)) & 1) == 1; \
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

std::ostream &operator<<(std::ostream &os, Node &node)
{
    os << std::string(node);
    return os;
}

// Node end
#pragma endregion

/***************************************
// Maze                               //
***************************************/
#pragma region Maze

Maze::Maze(uint8_t _w, uint8_t _h)
{
    w = _w;
    h = _h;
}

void Maze::load(uint8_t *bin)
{
    // Make sure that field isn't already pointing to some place in memory
    if (field)
    {
        free(field);
        field = NULL;
    }

    // Make sure that changed isn't already pointing to some place in memory
    if (changed)
    {
        free(changed);
        changed = NULL;
    }

    if (!bin)
    {
        field = (Node *)malloc(w * h * sizeof(Node));
        changed = (bool *)malloc(w * h * sizeof(bool));
        for (uint i = 0; i < w * h; ++i)
        {
            field[i] = Node(0);
            changed[i] = true;
        }
        return;
    }

    w = bin[0];
    h = bin[1];
    bin += 2;
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
    memset(changed, true, w * h);
} // namespace maze

uint8_t *Maze::unload()
{
    uint l = w * h;
    uint8_t *bin = (uint8_t *)malloc(((l + 1) / 2 + 2) * sizeof(uint8_t));
    bin[0] = w;
    bin[1] = h;
    bin += 2;
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
    free(changed);
    field = NULL;
    changed = NULL;
    return bin - 2;
}

void Maze::print()
{
    std::string top, middle, bottom;
    for (uint y = 0; y < h; ++y)
    {
        top = "";
        middle = "";
        bottom = "";
        for (uint x = 0; x < w; ++x)
        {
            Node *n = &field[y * w + x];
            top += n->north() ? "+  +" : "+--+";
            middle += n->west() ? "  " : "| ";
            middle += n->east() ? "  " : " |";
            bottom += n->south() ? "+  +" : "+--+";
        }
        std::cout << top << std::endl
                  << middle << std::endl
                  << bottom << std::endl;
    }
}

#pragma endregion // Maze end

} /* namespace maze */
#pragma endregion

void print_help(char *progname, uint8_t exit_code = 0)
{
    std::ostream *stream = exit_code ? &(std::cerr) : &(std::cout);
    *stream
        << "Usage: " << progname << " [options]" << std::endl
        << "Options:" << std::endl
        << "  -i PATH, --input=PATH      Read maze from PATH." << std::endl
        << "  -o PATH, --output=PATH     Write maze to PATH." << std::endl
        << "  -x N, --width=N            Set maze width. Get's overriden if maze is generated from file." << std::endl
        << "  -y N, --height=N           Set maze height. Get's overriden if maze is generated from file." << std::endl
        << "  -d, --display              Render maze to an SFML window." << std::endl
        << "  -g, --generate             Generate a random maze using depth first search." << std::endl
        << "  -h, --help                 Print this message and exit." << std::endl;

    exit(exit_code);
}

int main(int argc, char **argv)
{
    static uint width = 1;
    static uint height = 1;

#pragma region Parse command line arguments

    int c;
    int parsed;
    uint8_t error = false;

    while (1)
    {
        static struct option long_options[] =
            {
                /* These options set a flag. */
                {"verbose", no_argument, &verbose_flag, 1},
                {"brief", no_argument, &verbose_flag, 0},
                {"input", required_argument, 0, 'i'},
                {"output", required_argument, 0, 'o'},
                {"width", required_argument, 0, 'x'},
                {"height", required_argument, 0, 'y'},
                {"display", no_argument, 0, 'd'},
                {"generate", no_argument, 0, 'g'},
                {"help", no_argument, 0, 'h'},
                {0, 0, 0, 0}};
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long(argc, argv, "i:o:x:y:dgh", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            std::cout << "option " << long_options[option_index].name;
            if (optarg)
                std::cout << " with arg " << optarg;
            std::cout << std::endl;
            break;

        case 'i':
            if (std::filesystem::is_regular_file(optarg))
                input_path = optarg;
            else
                std::cerr << "Invalid path: " << '"' << optarg << '"' << std::endl;
            break;

        case 'o':
            if (!std::filesystem::exists(optarg) || std::filesystem::is_regular_file(optarg))
                output_path = optarg;
            else
                std::cerr << "Invalid path: " << '"' << optarg << '"' << std::endl;
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
            print_help(argv[0], error);
        }
    }

    if (verbose_flag)
        std::cout
            << "input path: \"" << input_path << '"' << std::endl
            << "output path: \"" << output_path << '"' << std::endl
            << "size: " << width << 'x' << height << std::endl
            << "display: " << (bDisplay ? "true" : "false") << std::endl
            << "generate: " << (bGenerate ? "true" : "false") << std::endl;

#pragma endregion

    maze::Maze m(width, height);

    // Read from file
    if (input_path.length())
    {
        if (verbose_flag)
            std::cout << "Loading from file " << input_path << std::endl;

        std::ifstream ifs(input_path, std::ios::binary | std::ios::ate);
        std::ifstream::pos_type pos = ifs.tellg();

        std::vector<char> result(pos);

        ifs.seekg(0, std::ios::beg);
        ifs.read(result.data(), pos);

        m.load((uint8_t *)result.data());
        width = m.w;
        height = m.h;

        if (verbose_flag)
        {
            std::cout << "size from file: " << width << 'x' << height << std::endl;
            if (!bDisplay)
                m.print();
        }
    }
    else
    {
        // Initialize empty field
        m.load(NULL);
    }

    uint cellSize = 1;
    {
        int csx = MAX_WIDTH / width;
        int csy = MAX_HEIGHT / height;
        cellSize = std::max(1, std::min(csx, csy));
    }

    uint wWidth = width * cellSize;
    uint wHeight = height * cellSize;

    sf::RenderWindow window(sf::VideoMode(wWidth, wHeight), "Floating");
    window.setTitle("SFMaze");

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

        
        window.display();
    }

    //TODO: save maze to file if path is specified
    m.unload();

    return EXIT_SUCCESS;
}
