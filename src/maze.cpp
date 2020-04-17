/**
 * @file maze.cpp
 * @author Maxi Barmetler (https://github.com/MixusMinimax)
 * @brief Generates and Displays Mazes.
 * @version 0.1
 * @date 2020-04-11
 */

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
#include <unordered_set>
#include <SFML/Graphics.hpp>

#include "maze.hpp"

#define DEBUG(x) //std::cout << x << std::endl;

#define MAX_WIDTH 1800
#define MAX_HEIGHT 950

static const std::string title = "SFMaze";
static int verbose_flag = 0;
static int bLegacy = 0;
static std::string input_path = "";
static std::string output_path = "";
static bool bDisplay = false;
static bool bGenerate = false;
static uint stepsPerFrame = 1;

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

#define temp(name, index)                              \
    bool Node::name(uint8_t val = 2)                   \
    {                                                  \
        if (val == 2)                                  \
            return ((_bin >> (3 - (index))) & 1) == 1; \
        if (val == 0)                                  \
        {                                              \
            _bin &= ~(1 << (3 - (index)));             \
            return false;                              \
        }                                              \
        if (val == 1)                                  \
        {                                              \
            _bin |= (1 << (3 - (index)));              \
            return true;                               \
        }                                              \
        return false;                                  \
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

Maze::Maze(uint _w, uint _h)
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
        memset(changed, true, w * h);
        for (uint i = 0; i < w * h; ++i)
            field[i] = Node(0);
        return;
    }

    if (bLegacy)
    {
        w = bin[0];
        h = bin[1];
        bin += 2;
    }
    else
    {
        w = ((uint *)bin)[0];
        h = ((uint *)bin)[1];
        bin += 8;
    }

    changed = (bool *)malloc(w * h * sizeof(bool));
    memset(changed, true, w * h);

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
    uint8_t *bin = (uint8_t *)malloc(((l + 1) / 2) * sizeof(uint8_t) + (bLegacy ? 2 : 8));

    if (bLegacy)
    {
        bin[0] = w;
        bin[1] = h;
        bin += 2 * sizeof(uint8_t);
    }
    else
    {
        ((uint *)bin)[0] = w;
        ((uint *)bin)[1] = h;
        bin += 8;
    }

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
    return bin - (bLegacy ? 2 : 8);
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

/***************************************
// Maze Generator                     //
***************************************/
#pragma region Maze Generator

MazeGenerator::MazeGenerator(Maze *_maze, point start)
{
    maze = _maze;
    stack.push_back({start, {-1, -1}});
    std::srand(unsigned(std::time(0)));
}

bool MazeGenerator::has_next()
{
    return !stack.empty();
}

void MazeGenerator::next()
{
    uint w = maze->w;
    uint h = maze->h;

    std::pair<point, point> top;
    while (1)
    {
        if (stack.empty())
            return;

        top = stack.back();
        stack.pop_back();
        if (visited.find(top.first) == visited.end())
        {
            visited.insert(top.first);
            break;
        }
    }

    if (top.second != point{-1, -1})
    {
        // Update field
        if (top.first.first > top.second.first) // new is to the east
        {
            maze->field[top.second.second * w + top.second.first].east(1);
            maze->field[top.first.second * w + top.first.first].west(1);
        }
        else if (top.first.first < top.second.first) // new is to the west
        {
            maze->field[top.second.second * w + top.second.first].west(1);
            maze->field[top.first.second * w + top.first.first].east(1);
        }
        else if (top.first.second < top.second.second) // new is to the north
        {
            maze->field[top.second.second * w + top.second.first].north(1);
            maze->field[top.first.second * w + top.first.first].south(1);
        }
        else if (top.first.second > top.second.second) // new is to the south
        {
            maze->field[top.second.second * w + top.second.first].south(1);
            maze->field[top.first.second * w + top.first.first].north(1);
        }

        maze->changed[top.first.second * w + top.first.first] = true;
        maze->changed[top.second.second * w + top.second.first] = true;
    }

    std::vector<point> neighbors = {
        {top.first.first - 1, top.first.second},
        {top.first.first + 1, top.first.second},
        {top.first.first, top.first.second - 1},
        {top.first.first, top.first.second + 1},
    };

    std::random_shuffle(neighbors.begin(), neighbors.end());

    for (auto n : neighbors)
    {
        if (n.first >= 0 && n.first < w && n.second >= 0 && n.second < h)
            if (visited.find(n) == visited.end())
                stack.push_back({n, top.first});
    }
}

#pragma endregion // Maze Generator end

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
        << "  -s N, --steps=N            Calculation steps per frame of drawing, ignored if -d is not set." << std::endl
        << "  -d, --display              Render maze to an SFML window." << std::endl
        << "  -g, --generate             Generate a random maze using depth first search." << std::endl
        << "  -h, --help                 Print this message and exit." << std::endl
        << std::endl
        << "Debugging:" << std::endl
        << "  --verbose                  Be verbose." << std::endl
        << "  --legacy                   Use old file format (single byte for width and height)." << std::endl;

    exit(exit_code);
}

void draw_rect(sf::RenderWindow *window, int x, int y, int w, int h, sf::Color color)
{
    sf::RectangleShape rect = sf::RectangleShape({(float)w, (float)h});
    rect.setFillColor(color);
    rect.setPosition(x, y);
    window->draw(rect);
}

void save_to_file(std::string path, uint8_t *bin, size_t length)
{
    if (verbose_flag)
        std::cout << "Saving to file " << path << ", size: " << length << std::endl;

    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);

    ofs.write((char *)bin, length);

    if (verbose_flag)
        std::cout << "Writing successful" << std::endl;
}

/***************************************
// Main                               //
***************************************/
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
                {"verbose", no_argument, &verbose_flag, 1},
                {"legacy", no_argument, &bLegacy, 1},
                {"input", required_argument, 0, 'i'},
                {"output", required_argument, 0, 'o'},
                {"width", required_argument, 0, 'x'},
                {"height", required_argument, 0, 'y'},
                {"steps", required_argument, 0, 's'},
                {"display", no_argument, 0, 'd'},
                {"generate", no_argument, 0, 'g'},
                {"help", no_argument, 0, 'h'},
                {0, 0, 0, 0}};
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long(argc, argv, "i:o:x:y:s:dgh", long_options, &option_index);
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

        case 's':
            parsed = atoi(optarg);
            stepsPerFrame = (uint)std::clamp(parsed, 1, 1024);
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

#pragma region Maze initialization
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

        if (verbose_flag)
            std::cout << "Reading successful" << std::endl;

        m.load((uint8_t *)result.data());
        width = m.w;
        height = m.h;

        if (verbose_flag)
        {
            std::cout << "size from file: " << width << 'x' << height << std::endl;
        }
    }
    else
    {
        // Initialize empty field
        m.load(NULL);
    }

    DEBUG("Made it past Maze init")

#pragma endregion

    maze::MazeGenerator *generator = NULL;
    if (bGenerate)
        generator = new maze::MazeGenerator(&m, {0, 0});

    if (!bDisplay)
    {
        timespec t;
        clock_gettime(CLOCK_MONOTONIC, &t);
        ulong start_us = t.tv_sec * 1000000 + t.tv_nsec / 1000;

        if (bGenerate)
            while (generator->has_next())
                generator->next();

        clock_gettime(CLOCK_MONOTONIC, &t);
        ulong end_us = t.tv_sec * 1000000 + t.tv_nsec / 1000;
        ulong calc_time_us = end_us - start_us;

        if (verbose_flag)
        {
            if (m.w <= 12 && m.h <= 20)
                m.print();
            else
                std::cout << "Too big to draw..." << std::endl;

            std::cout << "Done!" << std::endl
                      << "Compute time: " << (calc_time_us / 1000.f) << " ms" << std::endl;
        }

        uint8_t *bin = m.unload();
        if (output_path.length())
            save_to_file(output_path, bin, (bLegacy ? 2 : 8) + (width * height + 1) / 2);
        free(bin);

        return EXIT_SUCCESS;
    }

/***************************************
// SFML Window                        //
***************************************/
#pragma region SFML Window

    int cellSize = 3;
    {
        int csx = MAX_WIDTH / width;
        int csy = MAX_HEIGHT / height;
        cellSize = std::max(1, std::min(csx, csy));
    }

    int wall_thickness = std::max(1, cellSize / 16);
    int wall_length = std::max(1, cellSize - 2 * wall_thickness);
    int wall_offset = cellSize - wall_thickness;

    int wWidth = std::min((uint)MAX_WIDTH, width * cellSize);
    int wHeight = std::min((uint)MAX_HEIGHT, height * cellSize);

    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(wWidth, wHeight), "Floating");
    window->setTitle(title);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    uint fps_target = 60;
    float fps = 0;
    float fps_weight = std::min(1.f, 10.f / fps_target);

    ulong last_time_us = 0;
    ulong frameTime = 0;
    timespec curr_time;
    ulong curr_time_us;

    sf::Color wall_color(50, 50, 50);

    int redrawIndex = 0;

    DEBUG("Created Window")
    DEBUG("CellSize: " << cellSize)
    DEBUG("Wall Thickness: " << wall_thickness)

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window->close();
        }

        // Calculate fps
        {
            clock_gettime(CLOCK_MONOTONIC, &curr_time);
            curr_time_us = curr_time.tv_sec * 1000000ul + curr_time.tv_nsec / 1000ul;

            frameTime = curr_time_us - last_time_us;
            float fps_new = 1e6f / frameTime;
            fps = fps * (1 - fps_weight) + fps_new * fps_weight;

            last_time_us = curr_time_us;
        }

        window->setTitle(title + " - " + std::to_string((int)(fps * 10) / 10) + "fps");

        for (uint i = 0; i < 64; ++i)
        {
            m.changed[redrawIndex] = true;
            redrawIndex = (redrawIndex + 1) % (width * height);
        }

        for (uint y = 0; y < height; ++y)
        {
            for (uint x = 0; x < width; ++x)
            {
                if (m.changed[y * width + x])
                {
                    m.changed[y * width + x] = false;
                    uint rx = x * cellSize;
                    uint ry = y * cellSize;
                    maze::Node *node = &m.field[y * width + x];

                    draw_rect(window, rx, ry, cellSize, cellSize,
                              ((node->bin()) ? sf::Color{230, 230, 230} : sf::Color{100, 20, 100}));

                    if (cellSize > 1)
                    {
                        draw_rect(window, rx, ry, wall_thickness, wall_thickness, wall_color);
                        if (cellSize > 2)
                        {
                            draw_rect(window, rx, ry + wall_offset, wall_thickness, wall_thickness, wall_color);
                            draw_rect(window, rx + wall_offset, ry, wall_thickness, wall_thickness, wall_color);
                            draw_rect(window, rx + wall_offset, ry + wall_offset, wall_thickness, wall_thickness, wall_color);
                        }
                    }

                    if (!node->north() && cellSize > 1)
                        draw_rect(window, rx + wall_thickness, ry, wall_length, wall_thickness, wall_color);
                    if (!node->east() && cellSize > 2)
                        draw_rect(window, rx + wall_offset, ry + wall_thickness, wall_thickness, wall_length, wall_color);
                    if (!node->south() && cellSize > 2)
                        draw_rect(window, rx + wall_thickness, ry + wall_offset, wall_length, wall_thickness, wall_color);
                    if (!node->west() && cellSize > 1)
                        draw_rect(window, rx, ry + wall_thickness, wall_thickness, wall_length, wall_color);
                }
            }
        }

        window->display();

        // Next generator steps
        for (uint i = 0; i < stepsPerFrame; ++i)
            if (bGenerate && generator->has_next())
                generator->next();

        // Wait remaining time to keep fps constant
        {
            clock_gettime(CLOCK_MONOTONIC, &curr_time);
            ulong calc_time_us = curr_time.tv_sec * 1000000ul + curr_time.tv_nsec / 1000ul - curr_time_us;
            ulong target_time_us = 1000000ul / fps_target;
            ulong remaining_time_us = 100;
            if (calc_time_us < target_time_us - 100)
                remaining_time_us = target_time_us - calc_time_us;
            usleep(remaining_time_us);
        }
    }

    if (bGenerate)
        while (generator->has_next())
            generator->next();

    uint8_t *bin = m.unload();
    if (output_path.length())
        save_to_file(output_path, bin, (bLegacy ? 2 : 8) + (width * height + 1) / 2);
    free(bin);
    delete generator;
    delete window;

// SFML Window end
#pragma endregion

    return EXIT_SUCCESS;
}
