/*--------------------------------------------------------------------*/
/* dirNode.h                                                          */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#ifndef DIR_NODE_INCLUDED
#define DIR_NODE_INCLUDED

#include <stddef.h>
#include "a4def.h"

/*
   a DirNode is an object that contains a path payload and references to
   the DirNode's parent (if it exists) and children (if they exist).
*/
typedef struct pDirNode* DirNode;
/*
   Given a parent DirNode and a directory string dir, returns a new
   DirNode structure or NULL if any allocation error occurs in creating
   the node or its fields.

   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory string parameter,
   separated by a slash. It is also initialized with its parent link
   as the parent parameter value (but the parent itself is not changed
   to link to the new DirNode.  The children links are initialized but
   do not point to any children.
*/

DirNode DirNode_create(const char* dir, DirNode parent);

/*
  Destroys the entire hierarchy of Nodes rooted at n,
  including n itself.

  Returns the number of Nodes destroyed.
*/
size_t DirNode_destroy(DirNode n);

/*
   Returns DirNode n's path.
*/
const char* DirNode_getPath(DirNode n);

/*
  Returns the number of child directories n has.
*/
size_t DirNode_getNumChildren(DirNode n);

/*
   Returns the child DirNode of n with identifier childID, if one exists,
   otherwise returns NULL.
*/
DirNode DirNode_getChild(DirNode n, size_t childID);

/*
   Returns the parent DirNode of n, if it exists, otherwise returns NULL
*/
DirNode DirNode_getParent(DirNode n);

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
int DirNode_linkChild(DirNode parent, DirNode child);

/*
  Unlinks DirNode parent from its child DirNode child, leaving the
  child DirNode unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
 */
int DirNode_unlinkChild(DirNode parent, DirNode child);

/*
  Creates a new DirNode such that the new DirNode's path is dir appended
  to n's path, separated by a slash, and that the new DirNode has no
  children of its own. The new node's parent is n, and the new node is
  added as a child of n.

  (Reiterating for clarity: unlike with DirNode_create, parent *is*
  changed so that the link is bidirectional.)

  Returns SUCCESS upon completion, or:
  MEMORY_ERROR if the new DirNode cannot be created,
  ALREADY_IN_TREE if parent already has a child with that path
*/
int DirNode_addChild(DirNode parent, const char* dir);

/*
  Returns a string representation n, or NULL if there is an allocation
  error.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char* DirNode_toString(DirNode n);

#endif

/*
  Compares node1 and node2 based on their paths.
  Returns <0, 0, or >0 if node1 is less than,
  equal to, or greater than node2, respectively.
*/
static int DirNode_compare(DirNode node1, DirNode node2);

/*
   Returns 1 if n has a child directory with path,
   0 if it does not have such a child, and -1 if
   there is an allocation error during search.

   If n does have such a child, and childID is not NULL, store the
   child's identifier in *childID. If n does not have such a child,
   store the identifier that such a child would have in *childID.
*/
static int DirNode_hasChild(DirNode n, const char* path, size_t* childID);
