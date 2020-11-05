/*--------------------------------------------------------------------*/
/* node.c                                                             */
/* Author: Christian Ronda & Benjamin Herber                          */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dynarray.h"
#include "node.h"

/*--------------------------------------------------------------------*/
/*
   A node structure represents a node in a file directory tree. The node
   can be either of type FIL which has associated contents and length
   but no children or of type DIR which has associated children Nodes.
*/
struct node {
   /* The full path of this node (FIL or DIR). */
   char *path;

   /* Parent DIR of this node (FIL or DIR). */
   Node parent;

   /* Boolean enum specifying if Node is file or directory. */
   int type;

   /* Contents of stored file (invalid for DIR). */
   void *contents;

   /* Length of stored file (invalid for DIR). */
   size_t length;

   /* Sub-Dirs (lexigraphically sorted) of node (invalid for FIL). */
   DynArray_T children;
};

/*--------------------------------------------------------------------*/
/*
  returns a path with contents
  n->path/dir
  or NULL if there is an allocation error.

  Allocates memory for the returned string,
  which is then owened by the caller!
*/
static char *Node_buildPath(Node n, const char *dir) {
   char *path;

   assert(dir != NULL);

   if (n == NULL)
      path = malloc(strlen(dir) + 1);
   else
      path = malloc(strlen(n->path) + 1 + strlen(dir) + 1);

   if (path == NULL)
      return NULL;
   *path = '\0';

   if (n != NULL) {
      strcpy(path, n->path);
      strcat(path, "/");
   }
   strcat(path, dir);

   return path;
}

/*--------------------------------------------------------------------*/
void *Node_getFileContents(Node n) {

   assert(n != NULL);
   assert(n->type == FIL);

   return n->contents;
}

void *Node_replaceFileContents(Node n, void *newContents,
                               size_t newLength) {
   void *oldContents;

   assert(n != NULL);

   if (n->type == DIR)
      return NULL;

   oldContents = n->contents;
   n->contents = newContents;
   n->length = newLength;

   return oldContents;
}

/*--------------------------------------------------------------------*/
Node Node_create(const char *dir, Node parent) {

   Node new;

   assert(dir != NULL);

   /* Create defensive copy of path for Node. */
   new = malloc(sizeof(struct node));
   if (new == NULL)
      return NULL;
   new->path = Node_buildPath(parent, dir);

   if (new->path == NULL) {
      free(new);
      return NULL;
   }

   /* Set-up fields of Node struct. */
   new->parent = parent;
   new->contents = NULL;
   new->type = DIR;
   new->children = DynArray_new(0);
   if (new->children == NULL) {
      free(new->path);
      free(new);
      return NULL;
   }

   return new;
}

/*--------------------------------------------------------------------*/
Node Node_createFile(Node existingNode, void *contents, size_t length) {

   assert(existingNode != NULL);

   /* Allocates memory for size of new contents */
   existingNode->contents = contents;
   existingNode->type = FIL;
   existingNode->length = length;

   DynArray_free(existingNode->children);

   return existingNode;
}

/*--------------------------------------------------------------------*/
size_t Node_destroy(Node n) {
   size_t i;
   size_t count = 0;
   Node c;

   assert(n != NULL);

   /* Handle FIL type. */
   if (n->type == FIL) {
      free(n->path);
      free(n);
      count++;
      return count;
   }

   /* Destroy each sub dir. */
   for (i = 0; i < DynArray_getLength(n->children); i++) {
      c = DynArray_get(n->children, i);
      count += Node_destroy(c);
   }
   DynArray_free(n->children);

   free(n->path);
   free(n);
   count++;

   return count;
}

/*--------------------------------------------------------------------*/
int Node_compare(Node node1, Node node2) {
   assert(node1 != NULL);
   assert(node2 != NULL);

   /* Compare when the two nodes are of the same type */
   if (node1->type == node2->type)
      return strcmp(node1->path, node2->path);

   /* FILEs are less than DIRs. */
   if (node1->type == FIL)
      return 1;
   return -1;
}

/*--------------------------------------------------------------------*/
const char *Node_getPath(Node n) {

   assert(n != NULL);
   return n->path;
}

/*--------------------------------------------------------------------*/
size_t Node_getNumChildren(Node n) {
   assert(n != NULL);

   /* If n is a file. */
   if (n->type == FIL)
      return 0;

   return DynArray_getLength(n->children);
}

/*--------------------------------------------------------------------*/
int Node_hasChild(Node n, const char *path, size_t *childID) {
   size_t index;
   int result;
   Node checker;

   assert(n != NULL);
   assert(path != NULL);

   /* checks if file */
   if (n->type == FIL)
      return 0;

   checker = Node_create(path, NULL);
   if (checker == NULL)
      return -1;
   result = DynArray_bsearch(
       n->children, checker, &index,
       (int (*)(const void *, const void *))Node_compare);
   (void)Node_destroy(checker);

   if (childID != NULL)
      *childID = index;
   return result;
}

/*--------------------------------------------------------------------*/
Node Node_getChild(Node n, size_t childID) {
   assert(n != NULL);

   /* checks if file */
   if (n->type == FIL)
      return NULL;

   if (DynArray_getLength(n->children) > childID)
      return DynArray_get(n->children, childID);
   else
      return NULL;
}

/*--------------------------------------------------------------------*/
Node Node_getParent(Node n) {
   assert(n != NULL);

   return n->parent;
}

/*--------------------------------------------------------------------*/
int Node_linkChild(Node parent, Node child) {
   size_t i;
   char *rest;

   assert(parent != NULL);
   assert(child != NULL);

   /* FILEs cannot have children. */
   if (parent->type == FIL)
      return PARENT_CHILD_ERROR;

   /* Child already in DynArray. */
   if (Node_hasChild(parent, child->path, NULL))
      return ALREADY_IN_TREE;

   /* Prefixes don't match. */
   i = strlen(parent->path);
   if (strncmp(child->path, parent->path, i))
      return PARENT_CHILD_ERROR;

   /* Improper path format yet slips through prefix check. */
   rest = child->path + i;
   if (strlen(child->path) >= i && rest[0] != '/')
      return PARENT_CHILD_ERROR;

   /* Proper path structure. */
   rest++;
   if (strstr(rest, "/") != NULL)
      return PARENT_CHILD_ERROR;

   child->parent = parent;

   if (DynArray_bsearch(
           parent->children, child, &i,
           (int (*)(const void *, const void *))Node_compare) == 1)
      return ALREADY_IN_TREE;

   if (DynArray_addAt(parent->children, i, child) == TRUE)
      return SUCCESS;
   else
      return PARENT_CHILD_ERROR;
}

/*--------------------------------------------------------------------*/
int Node_unlinkChild(Node parent, Node child) {
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   /* Find node. */
   if (DynArray_bsearch(
           parent->children, child, &i,
           (int (*)(const void *, const void *))Node_compare) == 0)
      return PARENT_CHILD_ERROR;

   /* Remove it. */
   (void)DynArray_removeAt(parent->children, i);
   return SUCCESS;
}

/*--------------------------------------------------------------------*/
int Node_addChild(Node parent, const char *dir) {
   Node new;
   int result;

   assert(parent != NULL);
   assert(dir != NULL);

   /* FILEs cannot have children. */
   if (parent->type == FIL)
      return NOT_A_DIRECTORY;

   new = Node_create(dir, parent);
   if (new == NULL)
      return MEMORY_ERROR;

   result = Node_linkChild(parent, new);
   if (result != SUCCESS)
      (void)Node_destroy(new);

   return result;
}

/*--------------------------------------------------------------------*/
char *Node_toString(Node n) {
   char *copyPath;

   assert(n != NULL);

   copyPath = malloc(strlen(n->path) + 1);
   if (copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, n->path);
}

/*--------------------------------------------------------------------*/
int Node_getType(Node n) {
   assert(n != NULL);

   return n->type;
}

/*--------------------------------------------------------------------*/
size_t Node_getLength(Node n) {
   assert(n != NULL);

   return (n->length);
}
