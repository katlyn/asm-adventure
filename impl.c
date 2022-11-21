#include <stdio.h>
#include <stdlib.h>

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
typedef struct loadedSegment loadedSegment;
struct loadedSegment {
  // Pointer to the data of the current segment
  segment* data;
  // A count of the number of segments that branch off of this one.
  long branchCount;
  // An array of pointers to any segments that branch off of this segment
  segment** branches;
};

// A structure that allows us to implement a pseudo-binary tree to easily locate
// segments of a specific ID. This may seem redundant in some cases, but overall
// makes loading the segments easier.
typedef struct sortedSegment sortedSegment;
struct sortedSegment {
  loadedSegment* loaded;
  sortedSegment* lt;
  sortedSegment* gt;
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
  // segment *data = malloc(sizeof(segment));

  printf("loading segment\n");
  
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
  while (r && r->loaded->data->id != id) {
    if (r->loaded->data->id > id) {
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
  while (r && r->loaded->data->id != leaf->loaded->data->id) {
    if (r->loaded->data->id > leaf->loaded->data->id) {
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

  printf("Leaf %i added under %i\n", leaf->loaded->data->id, r->loaded->data->id);
}


void updateLinks(sortedSegment* root) {
  if (root->lt) {
    updateLinks(root->lt);
  }
  
}

// Read the given file into a sortedSegment for easier parsing
int loadFile (FILE *f, sortedSegment* story) {
  // Load the initial segment into the tree
  loadedSegment *loaded = malloc(sizeof(loadedSegment));
  loaded->branchCount = 0;
  loaded->data = malloc(sizeof(segment));
  int success = loadSegment(f, loaded->data);

  if (!success) {
    printf("Unable to load initial segment!\n");
    //return 0;
  }

  story->loaded = loaded;

  while (getc(f) != EOF) {
    fseek(f, -1, SEEK_CUR);
    loaded = malloc(sizeof(loadedSegment));
    loaded->branchCount = 0;
    loaded->data = malloc(sizeof(segment));
    success = loadSegment(f, loaded->data);
    if (!success) {
      printf("At eof, %i, %i\n", ftell(f), feof(f));
      // return 0;
    }
    sortedSegment *sorted = malloc(sizeof(sortedSegment));
    sorted->loaded = loaded;
    printf("%i\n", loaded->data->id);
    insertBranch(story, sorted);
    // sortedSegment *branch = findBranch(story, loaded->data->id);
  }

  printf("ID: %i AFTER: %i\n%s\n%s\n", story->loaded->data->id, story->loaded->data->after, story->loaded->data->keyword, story->loaded->data->text);
  printf("ID: %i AFTER: %i\n%s\n%s\n", story->gt->loaded->data->id, story->gt->loaded->data->after, story->gt->loaded->data->keyword, story->gt->loaded->data->text);
  printf("GT: %i LT: %i\n", !story->gt, !story->lt);

  return 1;
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

  int success = loadFile(storyFile, story);
  fclose(storyFile);

  // free(story);

  if (!success) {
    printf("Error loading story");
    return 1;
  }

  return 0;
}
