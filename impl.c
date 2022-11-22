#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// From https://stackoverflow.com/questions/3553296/sizeof-single-struct-member-in-c
#define member_size(type, member) sizeof(((type *)0)->member)

// STRUCTURE REQUIREMENTS
/* Basic
- Have a unique ID for each segment. This will allow jumping arbitrarily around
  the story.
  - Ideally, I feel like a namespaced string would be ideal for this. For the
    purposes of being implemented in low-level environments however, a numeric
    ID may be more practical for now.
- Have a simple string to identify the option, no longer than a few characters
- Be able to set an "injection point" where it will be available as an option
- Specify text to be printed once the option is selected
*/
/* Advanced
- Create and update variables
  - There are few ways to go about this, but some would be difficult to
    implement in assembly.
- Programmatic logic, variables available for use in specified text.
  - This could more easily be implemented with jmp and calls, but that opens
    up possible security issues. Perhaps a simple and arbitrary substitution
    or instruction language can be used?
*/

/* Notes
- Strings should be valid cstrings, terminated with a null character
- For the purposes of this initial implementation, there is no limit on segment
  keywords. In a future assembly implementation, a practical limit of 16
  characters may be put in place.
*/

// ----- Datastructure Definitions ----- //

// The base data structure for a story. All stories will start here.
typedef struct segment segment;
struct segment {
  // Specifies this segment's ID
  long id;
  // The segment that this segment should be available under.
  long after;
  // This segment's selector text, this will be what the user will type to enter
  // into this segment's context.
  char* keyword;
  // The text to be printed when this segment is activated
  char* text;
};

// Once a segment is loaded into memory, a separate data structure will need to
// represent it - this allows us to build the tree of segments that compose
// the whole story.
// typedef struct loadedSegment loadedSegment;
// struct loadedSegment {
//   // Pointer to the data of the current segment
//   segment* data;
// };

// A structure that allows us to implement a pseudo-binary tree to easily locate
// segments of a specific ID. This may seem redundant in some cases, but overall
// makes loading the segments easier.
typedef struct sortedSegment sortedSegment;
struct sortedSegment {
  segment* loaded;
  sortedSegment* lt;
  sortedSegment* gt;
  // A count of the number of segments that branch off of this one.
  size_t branchCount;
  // An array of pointers to any segments that branch off of this segment
  sortedSegment** branches;
};

// ----- Story Loading Utilities ----- //

// Reads a file until the end of the current string or eof, returning the length
// of the string
long skipString (FILE *f) {
  long initial = ftell(f);
  char c = fgetc(f);
  while (c != 0 && !feof(f)) {
    c = fgetc(f);
  }
  return ftell(f) - initial;
}

// Loads a segment and returns its pointer
int loadSegment (FILE *f, segment *data) {  
  // Read the statically sized longs
  fread(&data->id,member_size(segment, id), 1, f);
  fread(&data->after, member_size(segment, after), 1, f);

  // Get the length of the strings needed so that we can read them into a
  // correctly sized buffer.
  long length = skipString(f);
  // Seek backwards to the start of the string from the current file position
  fseek(f, -length, SEEK_CUR);

  // Initialize the keyword string to the correct length
  data->keyword = malloc(length);

  // Read data into the buffer
  fread(data->keyword, length, 1, f);

  // Repeat the same for the main text of the segment
  length = skipString(f);
  fseek(f, -length, SEEK_CUR);
  data->text = malloc(length);
  fread(data->text, 1, length, f);

  return !feof(f);
}

sortedSegment* findBranch(sortedSegment* root, long id) {
  sortedSegment* r = root;
  while (r && r->loaded->id != id) {
    if (r->loaded->id > id) {
      r = r->lt;
    } else {
      r = r->gt;
    }
  }
  return r;
}

void insertBranch(sortedSegment* root, sortedSegment* leaf) {
  // TODO: This will not handle duplicate IDs correctly and will likely cause
  //  memory leaks.
  sortedSegment* r = root;
  while (r && r->loaded->id != leaf->loaded->id) {
    if (r->loaded->id > leaf->loaded->id) {
      if (r->lt == NULL) {
        r->lt = leaf;
        break;
      } else {
        r = r->lt;
      }
    } else {
      if (r->gt == NULL) {
        r->gt = leaf;
        break;
      } else {
        r = r->gt;
      }
    }
  }
}

// Recursively iterate over all items in the tree and update their references to
// each other.
void updateLinks(sortedSegment* realRoot, sortedSegment* root) {
  if (root->lt) {
    updateLinks(realRoot, root->lt);
  }
  // If the insertAfter ID is greater than 0, set up links
  // Negatives are reserved for ending segments
  if (root->loaded->after >= 0) {
    sortedSegment* after = findBranch(realRoot, root->loaded->after);

    int newBranchCount = after->branchCount + 1;

    // Allocate the new array
    sortedSegment** newBranches = malloc(sizeof(sortedSegment*) * newBranchCount);
    newBranches[newBranchCount - 1] = root;

    // printf("Updating links for %li: %li total\n", after->loaded->id, after->branchCount);

    // If the old array has data, copy it and then free the memory
    if (after->branchCount > 0) {
      memcpy(newBranches, after->branches,sizeof(sortedSegment*) * after->branchCount);
      free(after->branches);
    }

    after->branches = newBranches;
    after->branchCount = newBranchCount;
  }
  if (root->gt) {
    updateLinks(realRoot, root->gt);
  }
}

// Read the given file into a sortedSegment for easier parsing
int loadFile (FILE *f, sortedSegment* story) {
  // Load the initial segment into the tree
  story->branchCount = 0;
  story->loaded = malloc(sizeof(segment));
  int success = loadSegment(f, story->loaded);

  if (!success) {
    printf("Unable to load initial segment!\n");
    //return 0;
  }

  while (getc(f) != EOF) {
    fseek(f, -1, SEEK_CUR);
    sortedSegment *sorted = malloc(sizeof(sortedSegment));
    sorted->lt = NULL;
    sorted->gt = NULL;
    sorted->branchCount = 0;
    sorted->branches = NULL;
    sorted->loaded = malloc(sizeof(segment));
    success = loadSegment(f, sorted->loaded);
    if (!success) {
      printf("At eof, %li, %i\n", ftell(f), feof(f));
      // return 0;
    }
    insertBranch(story, sorted);
  }

  updateLinks(story, story);

  return 1;
}

// Given a pointer to the root node of a story, free all nodes
void freeStory (sortedSegment* story) {
  // Don't double free if we've been passed a null pointer
  if (!story) {
    return;
  }

  freeStory(story->lt);
  freeStory(story->gt);

  free(story->loaded->keyword);
  free(story->loaded->text);

  free(story->loaded);
  free(story->branches);

  free(story);
}

// Get one line of input, and flush any input that doesn't fit into the provided
// buffer. Also removes any newline from the end of the string.
void getInput(char* buff, size_t size) {
  // Derived from https://stackoverflow.com/a/4023921
  fgets(buff, size, stdin);

  size_t len = strlen(buff);

  if (buff[len-1] != '\n') {
    char ch;
    while (((ch = getchar()) != '\n') && (ch != EOF));
  } else {
    buff[len-1] = 0;
  }
}

// Start a story at the given segment.
int gameLoop (sortedSegment* startingSegment) {
  sortedSegment* currentSegment = startingSegment;
  while (currentSegment) {
    printf("%s\n> ", currentSegment->loaded->text);

    // Get the user's input and check branches
    char input[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    sortedSegment* nextSegment = NULL;

    // Loop until we have a valid match
    while (1) {
      getInput(input, 16);
      // Provide a way to exit the game early if the user wants to
      if (!strcmp("EXIT_GAME", input)) {
        return (long) currentSegment;
      }

      // Check each of the possible branches to see if their keyword matches
      for (size_t i = 0; i < currentSegment->branchCount; ++i) {
        sortedSegment* potentialSegment = currentSegment->branches[i];
        if (!strcmp(input, potentialSegment->loaded->keyword)) {
          nextSegment = potentialSegment;
        }
      }

      // If a valid segment has been found, stop searching and continue
      if (nextSegment) {
        currentSegment = nextSegment;
        break;
      } else {
        printf("UNKNOWN SELECTION, POSSIBLE SELECTIONS: ");

        for (size_t i = 0; i < currentSegment->branchCount; ++i) {
          printf("%s ", currentSegment->branches[i]->loaded->keyword);
        }
        printf("\n> ");
      }
    }
    // Provide some space between segments
    printf("\n");
  }
  return (long)currentSegment;
}

// ----- Main Logic ----- //
int main (int argc, char** argv) {
  if (argc != 2) {
    printf("Invalid usage: provide a single argument in the form of a path to the story to load.");
    return 1;
  }

  FILE *storyFile = fopen(argv[1], "r");
  if (!storyFile) {
    printf("Unable to open specified file!");
    return 2;
  }

  sortedSegment *story = malloc(sizeof(sortedSegment));
  story->lt = NULL;
  story->gt = NULL;
  story->branchCount = 0;
  story->branches = NULL;

  int success = loadFile(storyFile, story);
  fclose(storyFile);

  if (!success) {
    printf("Error loading story");
    return 1;
  }

  gameLoop(findBranch(story, 0));

  freeStory(story);

  return 0;
}
