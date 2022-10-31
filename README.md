# Assembly Adventures

## Topic
Creating an extensible assembly-friendly standard for text-adventure games.

## Goals
- Create a data structure that is easy to implement in assembly.
- A structure that would allow easy implementation of "mods", not requiring existing branches to be modified to add more.
- A simple interpreter that would allow playing through a set of these data structures.

### Reach Goals
These goals are not core to the project, but would be nice to have.
- A user-friendly language parser.
- Very far reach, but an image that can be booted to in a VM or physical machine to play games.

## Prior Work
Many text-based adventure games have been created in the past. Some of these have their source code available, and as such we can look into how they solved different challenges within the game development.

### [Zork](https://github.com/MITDDC/zork)
Written in the MDL programming languages, Zork uses "game objects" to represent nearly everything within the game. A game object has a name, description, action, and a list of adjectives that describe it, as well as other required information. All objects are stores in the object tree, which represents the whole structure of the game.

Zork also has a parser that allow players to use basic english to explore the world, rather than strict pre-set commands.

### The Hitchhiker's Guide to the Galaxy

## Resources
- https://blog.steve.fi/writing_a_text_based_adventure_game_for_cp_m.html
- https://github.com/Benjamin00/SpaceAdventure
