#include <iostream>
#include <unistd.h>
#include <string>

#pragma region namespace maze
namespace maze
{
class Node
{
private:
    uint8_t _bin;

public:
    Node(uint8_t bin);

#define temp(name) \
    bool name(uint8_t val);

    temp(north);
    temp(east);
    temp(south);
    temp(west);
#undef temp

    uint8_t bin();

    operator std::string();
};

class Maze
{
private:
    uint w, h;
    Node *field = NULL;

public:
    Maze(uint _w, uint _h);

    void load(uint8_t *bin);

    uint8_t *unload();
};
} // namespace maze
#pragma endregion

void print_help(char *progname, uint8_t exit_code);