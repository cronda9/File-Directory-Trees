/*--------------------------------------------------------------------*/
/* dirNode.c                                                          */
/* Author: Ben Herber and Christian Ronda                             */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "fileNode.h"
#include "node.h"
#include "a4def.h"

/*
   a DirNode is an object that contains a path payload and references to
   the DirNode's parent (if it exists) and children (if they exist).
*/
struct DirNode{
   char *path;
   DirNode parent;
   DynArray_T children;
   DynArray_T file;
};

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

DirNode DirNode_create(const char* dir, DirNode parent){
   DirNode dirnode;
   
   assert(dir != NULL);

   dirnode = malloc(sizeof(struct DirNode));
   if(dirnode == NULL)
      return NULL;

   dirnode->path = malloc(strlen(dir)+1 + strlen(parent->path)+1);
   if(dirnode->path == NULL){
      free(dirnode);
      return NULL;
   }
   
   if(parent != NULL){
      strcpy(dirnode->path, parent->path);
      strcat(dirnode->path, "/");
   }
   
   strcat(dirnode->path, dir);

   dirnode->parent = parent;
   dirnode->children = DynArray_new(0);
   dirnode->file = DynArray_new(0);
   
   if(dirnode->children == NULL){
      free(dirnode->path);
      free(dirnode);
      return NULL;
   }
   
   return dirnode;
}
                     

/*
  Destroys the entire hierarchy of Nodes rooted at n,
  including n itself.

  Returns the number of Nodes destroyed.
*/
size_t DirNode_destroy(DirNode n){
   size_t i;
   size_t j;
   size_t count;
   Node c;

   assert(n != NULL);

   for(i = 0; i < DynArray_getLength(n->children); i++){
      c = DynArray_get(n->children, i);
      count += DirNode_destroy(c);
   }
   for(j = 0; j < DynArray_getLength(n->file); j++){
      FileNode_destroy(DynArray_get(n->file, j));
   }
   DynArray_free(n->file);

   DynArray_free(n->children);
   free(n->path);
   free(n);
   count++;
   
   return count;
}

/*
   Returns DirNode n's path.
*/
const char* DirNode_getPath(DirNode n){
   assert(n != NULL);
   
   return n->path;
}
/*
  Returns the number of child directories n has.
*/
size_t DirNode_getNumChildren(DirNode n){
   assert(n != NULL);

   return DynArray_getLength(n->children) +DynArray_getLength(n->file);
}

size_t DirNode_getNumFiles(DirNode n){
   assert(n != NULL);

   return DynArray_getLength(n->file);
}

size_t DirNode_getNumDirs(DirNode n){
   assert(n != NULL);

   return DynArray_getLength(n->children);
}
   

/* returns 1 for file, 0 for directory */
int DirNode_type(DirNode n, size_t childID){

   assert(n != NULL);

   if(childID < DynArray_getLength(n->file))
      return 1;
   return 0;
}
/*
   Returns the child DirNode of n with identifier childID, if one exists,
   otherwise returns NULL.
*/
DirNode DirNode_getDirChild(DirNode n, size_t childID){
   
   assert(n != NULL);
   assert(DirNode_type(n, childID) == 0);
   
   childID = childID - DynArray_getLength(n->file);
   if(DynArray_getLength(n->children) > childID)
      return DynArray_get(n-children, childID);
   else
      return NULL;
}

DirNode DirNode_getFileChild(DirNode n, size_t childID){
 
   assert(n != NULL);
   assert(DirNode_type(n, childID) == 1);

   if(DynArray_getLength(n->file) > childID)
      return DynArray_getLength(n->file), childID;
   else
      return NULL;

}
/*
   Returns the parent DirNode of n, if it exists, otherwise returns NULL
*/
DirNode DirNode_getParent(DirNode n){
   assert(n != NULL);

   return n->parent;
}

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
int DirNode_linkChild(DirNode parent, DirNode child){
   size_t i;
   char *rest;
   
   assert(parent != NULL);
   assert(child != NULL);

   if(Node_hasChild(parent, child->path, NULL))
      return ALREADY_IN_TREE;
   i = strlen(parent->path);
   if(strncmp(child->path, parent->path, i))
      return PARENT_CHILD_ERROR;
   rest = child->path + i;
   if(strlen(child->path) >= i && rest[0] != '/')
      return PARENT_CHILD_ERROR;
   rest++;
   if(strstr(rest, "/") != NULL)
      return PARENT_CHILD_ERROR;

   child->parent = parent;

   if(DynArray_bsearch(parent->children, child, &i,
                       (int (*)(const void*, const void*)) DirNode_compare) == 1)
      return ALREADY_IN_TREE;

   if(DynArray_addAt(parent->children, i, child) == TRUE)
      return SUCCESS;
   else
      return PARENT_CHILD_ERROR;
}

int DirNode_linkFileChild(DirNode parent, FileNode child){
   size_t i;
   char *rest;

   assert(parent != NULL);
   assert(child != NULL);

   if(Node_hasChild(parent, child->path, NULL))
      return ALREADY_IN_TREE;
   i = strlen(parent->path);
   if(strncmp(child->path, parent->path, i))
      return PARENT_CHILD_ERROR;
   rest = child->path + i;
   if(strlen(child->path) >= i && rest[0] != '/')
      return PARENT_CHILD_ERROR;
   rest++;
   if(strstr(rest, "/") != NULL)
      return PARENT_CHILD_ERROR;

   if(DynArray_bsearch(parent->children, child, &i,
                       (int (*)(const void*, const void*)) FileNode_compare) == 1)
      return ALREADY_IN_TREE;

   if(DynArray_addAt(parent->file, i, child) == TRUE)
      return SUCCESS;
   else
      return PARENT_CHILD_ERROR;

}
/*
  Unlinks DirNode parent from its child DirNode child, leaving the
  child DirNode unchanged.

  Returns PARENT_CHILD_ERROR if child is not a child of parent,
  and SUCCESS otherwise.
*/
int DirNode_unlinkDirChild(DirNode parent, DirNode child){
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   if(DynArray_bsearch(parent->children, child, &i,
                       (int (*)(const void*, const void*)) DirNode_compare) == 0)
      return PARENT_CHILD_ERROR;

   (void) DynArray_removeAt(parent->children, i);
   return SUCCESS;
}

int DirNode_unlinkFileChild(DirNode parent, FileNode child){
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   if(DynArray_bsearch(parent->file, child, &i,
                       (int (*)(const void*, const void*)) FileNode_compare) == 0)
      return PARENT_CHILD_ERROR;

   (void) DynArray_removeAt(parent->children, i);
   return SUCCESS;
}

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
int DirNode_addDirChild(DirNode parent, const char* dir){
   DirNode new;
   int result;

   assert(parent != NULL);
   assert(dir != NULL);

   new = DirNode_create(dir, parent);
   if(new == NULL)
      return PARENT_CHILD_ERROR;

   result = DirNode_linkChild(parent, new);
   if(result != SUCCESS)
      (void) DirNode_destroy(new);

   return result;
}

/*
  Returns a string representation n, or NULL if there is an allocation
  error.

  Allocates memory for the returned string,
  which is then owned by client!
*/
char* DirNode_toString(DirNode n){
   char *path;
   
   assert(n != NULL);

   path = malloc(strlen(n)+1);
   if(path == NULL)
      return NULL;

   return strcpy(path, n->path);
}


/*
  Compares node1 and node2 based on their paths.
  Returns <0, 0, or >0 if node1 is less than,
  equal to, or greater than node2, respectively.
*/
static int DirNode_compare(DirNode node1, DirNode node2){
   assert(node1 != NULL);
   assert(node2 != NULL);

   return strcmp(node1->path, node2->path);
}
/*
   Returns 1 if n has a child directory with path,
   0 if it does not have such a child, and -1 if
   there is an allocation error during search.

   If n does have such a child, and childID is not NULL, store the
   child's identifier in *childID. If n does not have such a child,
   store the identifier that such a child would have in *childID.
*/
static int DirNode_hasChild(DirNode n, const char* path, size_t* childID){
   size_t index;
   int result;
   Node checker;

   assert(n != NULL);
   assert(path != NULL);

   checker = DirNode_create(path, NULL);
   if(checker == NULL)
      return -1;
   result = DynArray_bsearch(n->children, checker, &index,
                             (int (*)(const void*, const void*)) DirNode_compare);
   (void) DirNode_destroy(checker);

   if(childID != NULL)
      *childID = index;
   return result;
}

#endif
