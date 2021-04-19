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
