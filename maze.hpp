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
    uint8_t w;          /// Width
    uint8_t h;          /// Height
    Node *field = NULL; /// Contains all Nodes of the maze

public:
    /**
     * @brief Construct a new Maze object
     * 
     * @param    _w                  width
     * @param    _h                  height
     */
    Maze(uint8_t _w, uint8_t _h);

    /**
     * @brief Allocate space for field and load field from binay array: { w, h, ...}
     * 
     * @param    bin                 Binary array containing data
     */
    void load(uint8_t *bin);

    /**
     * @brief store field in binary array, free field
     * 
     * @return  uint8_t*            Binary array containing data
     */
    uint8_t *unload();
};
} // namespace maze
#pragma endregion

/**
 * @brief Print help to stdout and exit
 * 
 * @param    progname            Name of the program
 * @param    exit_code           exit code, if not 0 print help to stderr
 */
void print_help(char *progname, uint8_t exit_code);