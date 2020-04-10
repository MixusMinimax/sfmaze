#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <unistd.h>

namespace maze
{
class Node
{
private:
    uint8_t _bin;

public:
    Node(uint8_t bin)
    {
        _bin = bin;
    }

#define temp(name, index)                        \
    bool name(uint8_t val = 2)                   \
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
    }

    temp(north, 0);
    temp(east, 1);
    temp(south, 2);
    temp(west, 3);

#undef temp

    uint8_t bin()
    {
        return _bin;
    }

    operator std::string()
    {
        char buf[5];
        sprintf(buf, "%c%c%c%c",
                ((_bin & 0b1000) ? '1' : '0'),
                ((_bin & 0b0100) ? '1' : '0'),
                ((_bin & 0b0010) ? '1' : '0'),
                ((_bin & 0b0001) ? '1' : '0'));
        return std::string(buf);
    }
};

class Maze
{
private:
    uint w, h;
    Node *field = NULL;

public:
    Maze(uint _w, uint _h)
    {
        w = _w;
        h = _h;
    }

    void load(uint8_t *bin)
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

    uint8_t *unload()
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
};
} // namespace maze

int main(int argc, char **argv)
{
    std::string input_path = "";

    // Parse command line arguments
    int c;
    while ((c = getopt(argc, argv, "i:o:w:h:vg")) != -1)
    {
        switch (c)
        {
        case 'i':
            input_path = optarg;
            break;
        
        default:
            break;
        }
    }


    sf::RenderWindow window(sf::VideoMode(200, 200), "Floating");
    window.setTitle("Yay");

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    maze::Node node(0b0101);
    std::cout << std::string(node) << std::endl;

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

    return 0;
}
