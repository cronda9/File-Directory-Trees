/*--------------------------------------------------------------------*/
/* dirNode.c                                                          */
/* Author: Ben Herber and Christian Ronda                             */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "a4def.h"
#include "dirNode.h"
#include "dynarray.h"

/*--------------------------------------------------------------------*/
/*
   a DirNode is an object that contains a path payload and references to
   the DirNode's parent (if it exists) and directory children
   (if they exist) as well as any files stored in the directory
   (if they exist).
*/
struct pDirNode {
   /* Sting describing tree path to DirNode. */
   char *path;

   /* Parent of DirNode in tree hierarchy. */
   DirNode parent;

   /* DirNode children of current DirNode in tree hierarchy. */
   DynArray_T dirChildren;

   /* FileNode children of current DirNode in tree hierarchy. */
   DynArray_T fileChildren;
};

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
DirNode DirNode_create(const char *dir, DirNode parent) {
   DirNode dirnode;

   assert(dir != NULL);

   dirnode = malloc(sizeof(struct pDirNode));
   if (dirnode == NULL)
      return NULL;
   
   /* Memory Allocation. */
   if(parent == NULL)
      dirnode->path = malloc(strlen(dir)+1);
   else
      dirnode->path = malloc(strlen(parent->path) + 1 + strlen(dir) + 1);
   if (dirnode->path == NULL) {
      free(dirnode);
      return NULL;
   }

   /* Copy and/or concatenate path. */
   if (parent != NULL) {
      strcpy(dirnode->path, parent->path);
      strcat(dirnode->path, "/");
      strcat(dirnode->path, dir);
   }
   else
      strcpy(dirnode->path, dir);

   dirnode->parent = parent;
   dirnode->dirChildren = DynArray_new(0);
   dirnode->fileChildren = DynArray_new(0);

   if (dirnode->dirChildren == NULL) {
      free(dirnode->path);
      free(dirnode);
      return NULL;
   } /* Check fileChildren Validity too. */

   return dirnode;
}

/*--------------------------------------------------------------------*/
/*
  Destroys the entire hierarchy of DirNodes and FileNodes rooted at n,
  including n itself.

  Returns the number of Nodes destroyed.
*/
size_t DirNode_destroy(DirNode n) {
   size_t i;
   size_t j;
   size_t count = 0;
   DirNode c;

   assert(n != NULL);

   /* Destroy File children. */
   for (j = 0; j < DynArray_getLength(n->fileChildren); j++) {
      FileNode_destroy(DynArray_get(n->fileChildren, j));
      count++;
   }

   /* Destroy Directory children. */
   for (i = 0; i < DynArray_getLength(n->dirChildren); i++) {
      c = DynArray_get(n->dirChildren, i);
      count += DirNode_destroy(c);
   }

   DynArray_free(n->fileChildren);
   DynArray_free(n->dirChildren);
   free(n->path);
   free(n);
   count++;

   return count;
}

/*--------------------------------------------------------------------*/
/*
  Compares node1 and node2 based on their paths.
  Returns <0, 0, or >0 if node1 is less than,
  equal to, or greater than node2, respectively.
*/
static int DirNode_compare(DirNode node1, DirNode node2) {
   assert(node1 != NULL);
   assert(node2 != NULL);

   return strcmp(node1->path, node2->path);
}

/*--------------------------------------------------------------------*/
int DirNode_hasDirChild(DirNode n, const char *path, size_t *childID) {
   size_t index;
   int result;
   DirNode checker;

   assert(n != NULL);
   assert(path != NULL);

   fprintf(stderr,"%s\n",path);
   checker = DirNode_create(path, NULL);
   fprintf(stderr,"%s\n",DirNode_getPath(checker));
   if (checker == NULL)
      return -1;
   result = DynArray_bsearch(
       n->dirChildren, checker, &index,
       (int (*)(const void *, const void *))DirNode_compare);
   (void)DirNode_destroy(checker);

   if (childID != NULL)
      *childID = index;
   return result;
}

/*--------------------------------------------------------------------*/
int DirNode_hasFileChild(DirNode n, const char *path, size_t *childID) {
   size_t index;
   int result;
   FileNode checker;

   assert(n != NULL);
   assert(path != NULL);

   checker = FileNode_create(path, NULL, 0);
   if (checker == NULL)
      return -1;
   result = DynArray_bsearch(
       n->fileChildren, checker, &index,
       (int (*)(const void *, const void *))FileNode_compare);
   (void)FileNode_destroy(checker);

   if (childID != NULL)
      *childID = index;
   return result;
}

/*--------------------------------------------------------------------*/
/*
   Returns DirNode n's path.
*/
const char *DirNode_getPath(DirNode n) {
   assert(n != NULL);

   return n->path;
}

/*--------------------------------------------------------------------*/
size_t DirNode_getNumFiles(DirNode n) {
   assert(n != NULL);

   return DynArray_getLength(n->fileChildren);
}

/*--------------------------------------------------------------------*/
size_t DirNode_getNumDirs(DirNode n) {
   assert(n != NULL);

   return DynArray_getLength(n->dirChildren);
}

/*--------------------------------------------------------------------*/
DirNode DirNode_getDirChild(DirNode n, size_t childID) {
   assert(n != NULL);

   if (DynArray_getLength(n->dirChildren) > childID)
      return DynArray_get(n->dirChildren, childID);
   else
      return NULL;
}

/*--------------------------------------------------------------------*/
FileNode DirNode_getFileChild(DirNode n, size_t childID) {
   assert(n != NULL);

   if (DynArray_getLength(n->fileChildren) > childID)
      return DynArray_get(n->fileChildren, childID);
   else
      return NULL;
}

/*--------------------------------------------------------------------*/
DirNode DirNode_getParent(DirNode n) {
   assert(n != NULL);

   return n->parent;
}

/*--------------------------------------------------------------------*/
int DirNode_linkDirChild(DirNode parent, DirNode child) {
   size_t i;
   char *rest;

   assert(parent != NULL);
   assert(child != NULL);

   if (DirNode_hasDirChild(parent, child->path, NULL))
      return ALREADY_IN_TREE;
   if (DirNode_hasFileChild(parent, child->path, NULL))
      return ALREADY_IN_TREE;
   i = strlen(parent->path);
   if (strncmp(child->path, parent->path, i))
      return PARENT_CHILD_ERROR;
   rest = child->path + i;
   if (strlen(child->path) >= i && rest[0] != '/')
      return PARENT_CHILD_ERROR;
   rest++;
   if (strstr(rest, "/") != NULL)
      return PARENT_CHILD_ERROR;

   child->parent = parent;

   if (DynArray_bsearch(
           parent->dirChildren, child, &i,
           (int (*)(const void *, const void *))DirNode_compare) == 1)
      return ALREADY_IN_TREE;

   if (DynArray_addAt(parent->dirChildren, i, child) == TRUE)
      return SUCCESS;
   else
      return PARENT_CHILD_ERROR;
}

/*--------------------------------------------------------------------*/
int DirNode_linkFileChild(DirNode parent, FileNode child) {
   size_t i;
   char *childPath;
   char *rest;

   assert(parent != NULL);
   assert(child != NULL);

   childPath = FileNode_getPath(child);

   if (DirNode_hasDirChild(parent, childPath, NULL))
      return ALREADY_IN_TREE;
   if (DirNode_hasFileChild(parent, childPath, NULL))
      return ALREADY_IN_TREE;
   i = strlen(parent->path);
   if (strncmp(childPath, parent->path, i))
      return PARENT_CHILD_ERROR;
   rest = childPath + i;
   if (strlen(childPath) >= i && rest[0] != '/')
      return PARENT_CHILD_ERROR;
   rest++;
   if (strstr(rest, "/") != NULL)
      return PARENT_CHILD_ERROR;

   if (DynArray_bsearch(
           parent->fileChildren, child, &i,
           (int (*)(const void *, const void *))FileNode_compare) == 1)
      return ALREADY_IN_TREE;

   if (DynArray_addAt(parent->fileChildren, i, child) == TRUE)
      return SUCCESS;
   else
      return PARENT_CHILD_ERROR;
}

/*--------------------------------------------------------------------*/
int DirNode_unlinkDirChild(DirNode parent, DirNode child) {
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   if (DynArray_bsearch(
           parent->dirChildren, child, &i,
           (int (*)(const void *, const void *))DirNode_compare) == 0)
      return PARENT_CHILD_ERROR;

   (void)DynArray_removeAt(parent->dirChildren, i);
   return SUCCESS;
}

/*--------------------------------------------------------------------*/
int DirNode_unlinkFileChild(DirNode parent, FileNode child) {
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   if (DynArray_bsearch(
           parent->fileChildren, child, &i,
           (int (*)(const void *, const void *))FileNode_compare) == 0)
      return PARENT_CHILD_ERROR;

   (void)DynArray_removeAt(parent->fileChildren, i);
   return SUCCESS;
}
