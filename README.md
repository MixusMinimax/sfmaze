# sfmaze
Generates random mazes using depth first search.  
Basically the same as my python version: [pymaze](https://github.com/MixusMinimax/pymaze)

### Demo (100x100)
![100x100 Demo](https://i.imgur.com/S7hsELS.gif)

### Usage
```
Usage: ./sfmaze [options]
Options:
  -i PATH, --input=PATH      Read maze from PATH.
  -o PATH, --output=PATH     Write maze to PATH.
  -x N, --width=N            Set maze width. Get's overriden if maze is generated from file.
  -y N, --height=N           Set maze height. Get's overriden if maze is generated from file.
  -s N, --steps=N            Calculation steps per frame of drawing, ignored if -d is not set.
  -d, --display              Render maze to an SFML window.
  -g, --generate             Generate a random maze using depth first search.
  -h, --help                 Print this message and exit.

Debugging:
  --verbose                  Be verbose.
  --legacy                   Use old file format (single byte for width and height).
```

Command to generate a `100x100` maze and display on screen:
```
./sfmaze -x100 -y100 -dgs 10
```

### Info
- Uses Depth-First Search with randomized push order to populate the maze
- Stores and Reads from custom binary files
  - The file format is like this: [width,height,...fields]
  - width and height are stored in little endian and are four bytes each
  - The rest of the file are the nodes, which are four bits each: north, east, south, west (1 for connected, 0 for disconnected)
  - If width and height are odd, the last byte in the file is padded with four zeros, and ignored on read
- For exporting a maze to a file, you do not need to open the sfml window with `-d`, but if you do, you can close it at any time. the maze generation will still finish, if an output path is supplied.
- If your computer is slow, you might increase the step size. This means, that in between frames more calculation steps are done, as drawing is the most expensive thing.




