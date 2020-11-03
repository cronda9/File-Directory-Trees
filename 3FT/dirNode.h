/*--------------------------------------------------------------------*/
/* dirNode.h                                                          */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#ifndef DIR_NODE_INCLUDED
#define DIR_NODE_INCLUDED

#include "a4def.h"
#include <stddef.h>

/*--------------------------------------------------------------------*/
/*
  a DirNode is an object that contains a path payload and references to
  the DirNode's parent (if it exists) and children (if they exist).
*/
typedef struct pDirNode *DirNode;

/*--------------------------------------------------------------------*/
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
DirNode DirNode_create(const char *dir, DirNode parent);

/*--------------------------------------------------------------------*/
/*
  Destroys the entire hierarchy of Nodes rooted at n,
  including n itself.

  Returns the number of Nodes destroyed.
*/
size_t DirNode_destroy(DirNode n);

/*--------------------------------------------------------------------*/
/*
   Returns DirNode n's path.
*/
const char *DirNode_getPath(DirNode n);

/*--------------------------------------------------------------------*/
/*
  Returns the number of child files DirNode n contains.
*/
size_t DirNode_getNumDirs(DirNode n);

/*--------------------------------------------------------------------*/
/*
  Returns the number of child files dir n has.
*/
size_t DirNode_getNumFiles(DirNode n);

/*--------------------------------------------------------------------*/
/*
  Returns the child DirNode of n with identifier childID, if one
  exists, otherwise returns NULL.
*/
DirNode DirNode_getDirChild(DirNode n, size_t childID);

/*--------------------------------------------------------------------*/
/*
  Return the child FileNode of DirNode n with identifier childID,
  if one exists, otherwise return NULL.
*/
FileNode DirNode_getFileChild(DirNode n, size_t childID);

/*--------------------------------------------------------------------*/
/*
  Returns the parent DirNode of n, if it exists, otherwise returns NULL
*/
DirNode DirNode_getParent(DirNode n);

/*--------------------------------------------------------------------*/
/*
  Makes child a child of parent, if possible, and returns SUCCESS.
  This is not possible in the following cases:
  * child's path is not parent's path + / + directory,
    in which case returns PARENT_CHILD_ERROR
  * parent already has a directory or file child with child's path,
    in which case returns ALREADY_IN_TREE
  * parent is unable to allocate memory to store new child link,
    in which case returns MEMORY_ERROR
 */
int DirNode_linkDirChild(DirNode parent, DirNode child);

/*--------------------------------------------------------------------*/
/*
  Makes child a child of parent, if possible, and returns SUCCESS.
  This is not possible in the following cases:
  * child's path is not parent's path: + / + path(file),
    in which case returns PARENT_CHILD_ERROR
  * parent already has a directory or file child with child's path,
    in which case returns ALREADY_IN_TREE
  * parent is unable to allocate memory to store new child link,
    in which case returns MEMORY_ERROR
 */
int DirNode_linkFileChild(DirNode parent, FileNode child);

/*--------------------------------------------------------------------*/
/*
  Unlinks DirNode parent from its child DirNode child, leaving the
  child DirNode unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
 */
int DirNode_unlinkDirChild(DirNode parent, DirNode child);

/*--------------------------------------------------------------------*/
/*
  Unlinks DirNode parent from its child FileNode child, leaving the
  child FileNode unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
 */
int DirNode_unlinkFileChild(DirNode parent, FileNode child);

/*--------------------------------------------------------------------*/
/*
  Return 1 if n has a child directory with path,
  return 0 if it does not have such a child, and
  return -1 if there is an allocation error during search.

  If n does have such a child, and childID is not NULL, store the
  child's identifier in *childID. If n does not have such a child,
  store the identifier that such a child would have in *childID.
*/
int DirNode_hasDirChild(DirNode n, const char *path, size_t *childID);

/*--------------------------------------------------------------------*/
/*
  Return 1 if DirNode n has a child file with path,
  return 0 if it does not have such a child, and
  return -1 if there is an allocation error during search.

  If n does have such a child, and childID is not NULL, store the
  child's identifier in *childID. If n does not have such a child,
  store the identifier that such a child would have in *childID.
*/
static int DirNode_hasFileChild(DirNode n, const char *path,
                                size_t *childID)

#endif
