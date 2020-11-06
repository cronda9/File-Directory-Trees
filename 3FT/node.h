/*--------------------------------------------------------------------*/
/* node.h                                                             */
/* Author: Christian Ronda & Benjamin Herber                          */
/*--------------------------------------------------------------------*/

#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include "a4def.h"
#include <stddef.h>

/*
   a Node is an object that contains a path payload and references to
   the Node's parent (if it exists), children (if DIR and they exist) as
   well as file contents and length (if FIL type and it exists).
*/
typedef struct node *Node;

/*--------------------------------------------------------------------*/
/* A flag to tell if node is representative of a file or directory. */
enum { DIR, FIL };

/*--------------------------------------------------------------------*/
/*
   Given Node n of type FIL, returns the contents of the file.
*/
void *Node_getFileContents(Node n);

/*--------------------------------------------------------------------*/
/*
   Given Node n of type FIL, replaces the contents with newContents,
   and updates the length to newLength and returns the old contents.
   Returns NULL if n is of type DIR or memory allocation error
*/
void *Node_replaceFileContents(Node n, void *newContents,
                               size_t newLength);

/*--------------------------------------------------------------------*/
/*
   Given a parent Node and a directory string dir, returns a new
   Node DIR type structure or NULL if any allocation error occurs in
   creating the node or its fields.

   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory string parameter,
   separated by a slash. It is also initialized with its parent link
   (ig parent is given) as the parent parameter value (but the parent
   itself is not changed to link to the new Node. The children links are
   initialized but do not point to any children.
*/
Node Node_createDir(const char *dir, Node parent);

/*--------------------------------------------------------------------*/
/*
  Given a path to a file, creates a FIL Node structure with associated
  file contents and length metadata. No links to parent node are made.

  return the new file-type node if successful.
  return NULL any allocation error occurs.
*/
Node Node_createFile(char *path, void *contents, size_t length);

/*--------------------------------------------------------------------*/
/*
  Destroys the entire hierarchy of Nodes rooted at n,
  including n itself.

  Returns the number of Nodes destroyed.
*/
size_t Node_destroy(Node n);

/*--------------------------------------------------------------------*/
/*
   Returns Node n's path.
*/
const char *Node_getPath(Node n);

/*--------------------------------------------------------------------*/
/*
  Returns the number of children (FIL or DIR) n has.
*/
size_t Node_getNumChildren(Node n);

/*--------------------------------------------------------------------*/
/*
   Returns the child Node of n with identifier childID, if one exists,
   otherwise returns NULL.
*/
Node Node_getChild(Node n, size_t childID);

/*--------------------------------------------------------------------*/
/*
   Returns the parent Node of n, if it exists, otherwise returns NULL
*/
Node Node_getParent(Node n);

/*--------------------------------------------------------------------*/
/*
  Makes child a child of parent, if possible, and returns SUCCESS.
  This is not possible in the following cases:
  * child's path is not parent's path + / + directory,
    in which case returns PARENT_CHILD_ERROR
  * parent already has a child with child's path,
    in which case returns ALREADY_IN_TREE
  * parent is unable to allocate memory to store new child link,
    in which case returns MEMORY_ERROR
 */
int Node_linkChild(Node parent, Node child);

/*--------------------------------------------------------------------*/
/*
  Unlinks Node parent from its child Node child, leaving the
  child Node unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
 */
int Node_unlinkChild(Node parent, Node child);

/*--------------------------------------------------------------------*/
/*
  Returns the type of Node n. DIR or FIL
*/
int Node_getType(Node n);

/*--------------------------------------------------------------------*/
/*
  Returns the length of Node n.
*/
size_t Node_getLength(Node n);

#endif
