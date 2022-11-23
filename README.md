# Assembly Adventures

## Topic
Creating an extensible assembly-friendly standard for text-adventure games. At its base, this project aims to create an easy binary format for interactive stories.

## Goals
- Create a data structure that is easy to implement in assembly.
- A structure that would allow easy implementation of "mods", not requiring existing branches to be modified to add more.
- A simple interpreter that would allow playing through a set of these data structures.

### Reach Goals
These goals are not core to the project, but would be nice to have.
- A user-friendly language parser.
- Very far reach, but an image that can be booted to in a VM or physical machine to play games.

## Basic Implementation
Currently this repository contains a basic implementation of a story parser and runner in impl.c, as well as a short story in story.asm. To easily run this, you may do something similar to the following:

```sh
gcc impl.c -o runstory
nasm story.asm -o story.bin
./runstory story.bin
```
This basic implementation allows early exit of the game at any time by entering `EXIT_GAME` and pressing enter.

Under the hood, the basic implementation does a few things to be able to run a story:
1. The path provided as the first argument to the program is loaded into memory.
1. Reading each segment from the file sequentially, a binary tree is constructed, similar to the structure that is typically used for treesort. This binary tree is created based on each segment's ID.
1. The binary tree is walked, and each segment is updated with a list of pointers to segments that follow it.
1. Once all links between segments have been created, the game loop starts at segment ID 0 and begins prompting the user for their choice after printing each segment's contents.

## Story structure
Currently stories are stored in a binary file. These can be created using assembly as an easy but low-tech solution.

A story is made up of at least one, but usually multiple segments. These are are listed one after the other with no spacing. The structure of a segment is documented below.

```asm
dq 4         ; ID of the segment
dq 2         ; Id of the segment that this segment will be listed as an option after
db 'skip', 0 ; The keyword that a user will enter to navigate to this segment
; The body text of this segment to be printed to console
db "You decide to skip your notes for today. As soon as you walk out of the classroom, you've happily forgotten everything that was talked about.", 0
```
### Future modifications
A few modifications to this structure would allow for greater functionality:
- Allow specifying multiple injection points for a segment.
- Add support for "control" segments that allow for logic and variables.

## Prior Work
Many text-based adventure games have been created in the past. Some of these have their source code available, and as such we can look into how they solved different challenges within the game development.

### [Zork](https://github.com/MITDDC/zork)
Written in the MDL programming languages, Zork uses "game objects" to represent nearly everything within the game. A game object has a name, description, action, and a list of adjectives that describe it, as well as other required information. All objects are stores in the object tree, which represents the whole structure of the game.

Zork also has a parser that allow players to use basic english to explore the world, rather than strict pre-set commands.

### The Hitchhiker's Guide to the Galaxy

## Resources
- https://blog.steve.fi/writing_a_text_based_adventure_game_for_cp_m.html
- https://github.com/Benjamin00/SpaceAdventure
