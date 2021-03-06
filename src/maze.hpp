#include <iostream>
#include <unistd.h>
#include <string>
#include <unordered_set>
#include <boost/functional/hash.hpp>
#include <SFML/Graphics.hpp>

#pragma region namespace maze
namespace maze
{
class Node
{
private:
    uint8_t _bin; /// Connection bitfield (north, east, south, west)

public:
    /**
     * @brief Construct a new Node object
     * 
     * @param    bin                 Connection bitfield
     */
    Node(uint8_t bin);

#define temp(name) \
    bool name(uint8_t val);

    temp(north);
    temp(east);
    temp(south);
    temp(west);
#undef temp

    /**
     * @brief Getter for _bin
     * 
     * @return uint8_t Connection bitfield
     */
    uint8_t bin();

    operator std::string();
};

class Maze
{
public:
    uint w;               /// Width
    uint h;               /// Height
    Node *field = NULL;   /// Contains all Nodes of the maze
    bool *changed = NULL; /// Stores for each Node, if it has changed, so that only changed Nodes are drawn to the screen

public:
    /**
     * @brief Construct a new Maze object
     * 
     * @param    _w                  width
     * @param    _h                  height
     */
    Maze(uint _w, uint _h);

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

    /**
     * @brief Prints the maze into the console
     */
    void print();
};

class MazeGenerator
{
private:
    typedef std::pair<int, int> point;

    Maze *maze;                                            /// Pointer to maze::Maze object that should be generated
    std::vector<std::pair<point, point>> stack;            /// Contains next points and their parents
    std::unordered_set<point, boost::hash<point>> visited; /// Contains all visited points

public:
    /**
     * @brief Construct a new Maze Generator object
     * 
     * @param    _maze               Pointer to maze::Maze object
     * @param    start               Starting position
     */
    MazeGenerator(Maze *_maze, point start);

    /**
     * @brief to be called before next()
     * 
     * @return true if there is a next step
     */
    bool has_next();

    /**
     * @brief Executes the next step in the algorithm
     */
    void next();
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

/**
 * @brief Draws a Rectangle
 * 
 * @param    window              sf::RenderWindow to draw into
 * @param    x                   Position
 * @param    y                   -""-
 * @param    w                   Size
 * @param    h                   -""-
 * @param    color               sf::Color of the Rectangle
 */
void draw_rect(sf::RenderWindow *window, int x, int y, int w, int h, sf::Color color);

/**
 * @brief Saves byte Array to file
 * 
 * @param    path                Path to the file to write to
 * @param    bin                 byte array
 * @param    length              size of bin
 */
void save_to_file(std::string path, uint8_t *bin, size_t length);