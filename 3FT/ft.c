/*--------------------------------------------------------------------*/
/* dt.c                                                               */
/* Author: Christian Ronda & Benjamin Herber                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dynarray.h"
#include "ft.h"
#include "node.h"

/* Equality enum to clarify if comparisons. */
enum { EQUAL };

/*--------------------------------------------------------------------*/
/* A Directory Tree is an Abstract Object that stores both directories
   and files with 3 state variables:
*/
/* A flag for if it is in an initialized state (TRUE) or not (FALSE). */
static boolean isInitialized;

/* A pointer to the root Node in the hierarchy (either DIR or FIL). */
static Node root;

/* A counter of the number of Nodes in the hierarchy */
static size_t count;

/*--------------------------------------------------------------------*/
/*
   Starting at the parameter curr, traverses as far down
   the hierarchy as possible while still matching the path
   parameter.

   Returns a pointer to the farthest matching Node down that path,
   or NULL if there is no node in curr's hierarchy that matches
   a prefix of the path
*/
static Node FT_traversePathFrom(char *path, Node curr) {
   Node found;
   size_t i;

   assert(path != NULL);

   if (curr == NULL)
      return NULL;

   else if (!strcmp(path, Node_getPath(curr)))
      return curr;

   else if (!strncmp(path, Node_getPath(curr),
                     strlen(Node_getPath(curr)))) {
      for (i = 0; i < Node_getNumChildren(curr); i++) {
         found = FT_traversePathFrom(path, Node_getChild(curr, i));
         if (found != NULL)
            return found;
      }
      return curr;
   }
   return NULL;
}

/*--------------------------------------------------------------------*/
/*
   Returns the farthest Node reachable from the root following a given
   path, or NULL if there is no Node in the hierarchy that matches a
   prefix of the path.
*/
static Node FT_traversePath(char *path) {
   assert(path != NULL);

   /* Root Failure. */
   if (root == NULL)
      return NULL;

   /* Check if file is at root. */
   if (Node_getType(root) == FIL) {
      if (strcmp(path, Node_getPath(root)) != EQUAL)
         return NULL;
      return root;
   }

   return FT_traversePathFrom(path, root);
}

/*
   Destroys the entire hierarchy of Nodes rooted at curr,
   including curr itself.
*/
static void FT_removePathFrom(Node curr) {
   if (curr != NULL) {
      count -= Node_destroy(curr);
   }
}

/*--------------------------------------------------------------------*/
/*
   Given a prospective parent and child Node,
   adds child to parent's children list, if possible

   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS.
*/
static int FT_linkParentToChild(Node parent, Node child) {

   assert(parent != NULL);

   if (Node_linkChild(parent, child) != SUCCESS) {
      (void)Node_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/*--------------------------------------------------------------------*/
/*
   Inserts a new path into the tree rooted at parent, or, if
   parent is NULL, as the root of the data structure.

   If a Node representing path already exists, returns ALREADY_IN_TREE

   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR

   If there is an error linking any of the new nodes,
   returns PARENT_CHID_ERROR

   Otherwise, returns SUCCESS
*/
static int FT_insertRestOfPath(char *path, Node parent, Node *first,
                               Node *last) {

   Node curr = parent;
   Node firstNew = NULL;
   Node new;
   char *copyPath;
   char *restPath = path;
   char *dirToken;
   int result;
   size_t newCount = 0;

   assert(path != NULL);

   /* Test root case and if already exists or get rest of path. */
   if (curr == NULL) {
      if (root != NULL) {
         return CONFLICTING_PATH;
      }
   } else {
      if (!strcmp(path, Node_getPath(curr)))
         return ALREADY_IN_TREE;

      restPath += (strlen(Node_getPath(curr)) + 1);
   }

   /* Set up tokenizing for inserting path. */
   copyPath = malloc(strlen(restPath) + 1);
   if (copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");

   /* Create necessary new nodes and link. */
   while (dirToken != NULL) {
      new = Node_create(dirToken, curr);
      newCount++;

      /* Test if first in chain or link successively. */
      if (firstNew == NULL)
         firstNew = new;
      else {
         result = FT_linkParentToChild(curr, new);
         if (result != SUCCESS) {
            (void)Node_destroy(new);
            (void)Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }

      if (new == NULL) {
         (void)Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }

      curr = new;
      dirToken = strtok(NULL, "/");
   }

   /* If requested: set last mem to last and first inserted nodes. */
   if ((last != NULL) && (first != NULL)) {
      *last = curr;
      *first = firstNew;
   }
   free(copyPath);

   /* Finish linking process to given prefix. */
   if (parent == NULL) {
      root = firstNew;
      count = newCount;
      return SUCCESS;
   } else {
      result = FT_linkParentToChild(parent, firstNew);
      if (result == SUCCESS)
         count += newCount;
      else
         (void)Node_destroy(firstNew);

      return result;
   }
}

/*--------------------------------------------------------------------*/
int FT_insertDir(char *path) {
   Node curr;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* Go down as far as possible on prefix. */
   curr = FT_traversePath(path);

   /* Insert the directory and all other paths not in tree. */
   return FT_insertRestOfPath(path, curr, NULL, NULL);
}

/*--------------------------------------------------------------------*/
int FT_insertFile(char *path, void *contents, size_t length) {
   Node curr;
   Node last;
   Node first;
   int result;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* Go down as far as possible on prefix. */
   curr = FT_traversePath(path);

   /* Test if it's parent is a file. */
   if ((curr != NULL) && (Node_getType(curr) == FIL) &&
       (strcmp(path, Node_getPath(curr)) != EQUAL))
      return NOT_A_DIRECTORY;

   /* Insert the Node(s) and all other paths not in tree. */
   result = FT_insertRestOfPath(path, curr, &first, &last);
   if (result != SUCCESS) {
      return result;
   }

   /* Update final Node to the FIL type and associated data. */
   if (Node_createFile(last, contents, length) == NULL) {
      Node_unlinkChild(curr, first);
      Node_destroy(first);
      return MEMORY_ERROR;
   }

   return SUCCESS;
}

/*--------------------------------------------------------------------*/
boolean FT_containsDir(char *path) {
   Node curr;

   assert(path != NULL);

   if (!isInitialized)
      return FALSE;

   /* Try to reach node. */
   curr = FT_traversePath(path);

   /* Root Failure. */
   if (curr == NULL)
      return FALSE;

   /* Mismatch Failure. */
   if (strcmp(path, Node_getPath(curr)) != EQUAL)
      return FALSE;

   /* File Failure. */
   if (Node_getType(curr) == FIL)
      return FALSE;

   return TRUE;
}

/*--------------------------------------------------------------------*/
boolean FT_containsFile(char *path) {
   Node curr;

   assert(path != NULL);

   if (!isInitialized)
      return FALSE;

   /* Try to reach node. */
   curr = FT_traversePath(path);

   /* Root Failure. */
   if (curr == NULL)
      return FALSE;

   /* Mismatch Failure. */
   if (strcmp(path, Node_getPath(curr)) != EQUAL)
      return FALSE;

   /* Dir Failure. */
   if (Node_getType(curr) == DIR)
      return FALSE;

   return TRUE;
}

/*--------------------------------------------------------------------*/
void *FT_getFileContents(char *path) {
   Node curr;

   assert(path != NULL);

   if (!isInitialized)
      return NULL;

   /* Try to reach node. */
   curr = FT_traversePath(path);

   /* Root Failure. */
   if (curr == NULL)
      return NULL;

   /* Mismatch Failure. */
   if (strcmp(path, Node_getPath(curr)) != EQUAL)
      return NULL;

   /* Dir Failure. */
   if (Node_getType(curr) == DIR)
      return NULL;

   return Node_getFileContents(curr);
}

/*--------------------------------------------------------------------*/
void *FT_replaceFileContents(char *path, void *newContents,
                             size_t newLength) {
   Node curr;

   assert(path != NULL);

   if (!isInitialized)
      return NULL;

   /* Try to reach node. */
   curr = FT_traversePath(path);

   /* Root Failure. */
   if (curr == NULL)
      return NULL;

   /* Mismatch Failure. */
   if (strcmp(path, Node_getPath(curr)) != EQUAL)
      return NULL;

   /* Dir Failure. */
   if (Node_getType(curr) == DIR)
      return NULL;

   return Node_replaceFileContents(curr, newContents, newLength);
}

/*--------------------------------------------------------------------*/
/*
  Removes the directory hierarchy rooted at path starting from Node
  curr. If curr is the data structure's root, root becomes NULL.

  Returns NO_SUCH_PATH if curr is not the Node for path,
  and SUCCESS otherwise.
 */
static int FT_rmPathAt(char *path, Node curr) {

   Node parent;

   assert(path != NULL);
   assert(curr != NULL);

   parent = Node_getParent(curr);

   if (!strcmp(path, Node_getPath(curr))) {
      if (parent == NULL)
         root = NULL;
      else
         Node_unlinkChild(parent, curr);

      FT_removePathFrom(curr);

      return SUCCESS;
   } else
      return NO_SUCH_PATH;
}

/*--------------------------------------------------------------------*/
int FT_rmDir(char *path) {
   Node curr;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* Traverse to requested node. */
   curr = FT_traversePath(path);

   /* Root Failure. */
   if (curr == NULL)
      return NO_SUCH_PATH;

   /* File Failure. */
   if (Node_getType(curr) == FIL)
      return NOT_A_DIRECTORY;

   return FT_rmPathAt(path, curr);
   /* Check if root, remove accordingly. */
   if (!strcmp(path, Node_getPath(curr))) {
      if (Node_getParent(curr) == NULL)
         root = NULL;

      count -= Node_destroy(curr);

      return SUCCESS;
   } else
      return NO_SUCH_PATH;
}

/*--------------------------------------------------------------------*/
int FT_rmFile(char *path) {
   Node curr;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* Traverse to requested node. */
   curr = FT_traversePath(path);

   /* Root Failure. */
   if (curr == NULL)
      return NO_SUCH_PATH;

   /* Dir Failure. */
   if (Node_getType(curr) == DIR)
      return NOT_A_FILE;

   return FT_rmPathAt(path, curr);
   /* Check if root, remove accordingly. */
   if (!strcmp(path, Node_getPath(curr))) {
      if (Node_getParent(curr) == NULL)
         root = NULL;

      count -= Node_destroy(curr);

      return SUCCESS;
   } else
      return NO_SUCH_PATH;
}

/*--------------------------------------------------------------------*/
int FT_init(void) {

   if (isInitialized)
      return INITIALIZATION_ERROR;

   /* Set up AO. */
   isInitialized = 1;
   root = NULL;
   count = 0;

   return SUCCESS;
}

/*--------------------------------------------------------------------*/
int FT_destroy(void) {

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* Destroy tree and reset AO. */
   FT_removePathFrom(root);
   root = NULL;
   isInitialized = 0;

   return SUCCESS;
}

/*--------------------------------------------------------------------*/
/*
   Performs a pre-order traversal of the tree rooted at n,
   inserting each payload to DynArray_T d beginning at index i.
   Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(Node n, DynArray_T d, size_t i) {
   size_t c;

   assert(d != NULL);

   if (n != NULL) {
      (void)DynArray_set(d, i, Node_getPath(n));
      i++;
      for (c = 0; c < Node_getNumChildren(n); c++)
         i = FT_preOrderTraversal(Node_getChild(n, c), d, i);
   }
   return i;
}

/*--------------------------------------------------------------------*/
/*
   Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length.
*/
static void FT_strlenAccumulate(char *str, size_t *pAcc) {
   assert(pAcc != NULL);

   if (str != NULL)
      *pAcc += (strlen(str) + 1);
}

/*--------------------------------------------------------------------*/
/*
   Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string.
*/
static void FT_strcatAccumulate(char *str, char *acc) {
   assert(acc != NULL);

   if (str != NULL)
      strcat(acc, str);
   strcat(acc, "\n");
}

int FT_stat(char *path, boolean *type, size_t *length) {
   Node curr;
   int t;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* Traverse to requested node. */
   curr = FT_traversePath(path);

   /* Root Failure. */
   if (curr == NULL)
      return NO_SUCH_PATH;

   t = Node_getType(curr);
   if (t == DIR) {
      *type = FALSE;
      return SUCCESS;
   }

   *type = TRUE;
   *length = Node_getLength(curr);
   return SUCCESS;
}

/*--------------------------------------------------------------------*/
char *FT_toString(void) {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char *result = NULL;

   if (!isInitialized)
      return NULL;

   nodes = DynArray_new(count);
   (void)FT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void *))FT_strlenAccumulate,
                (void *)&totalStrlen);

   result = malloc(totalStrlen);
   if (result == NULL) {
      DynArray_free(nodes);

      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void *))FT_strcatAccumulate,
                (void *)result);

   DynArray_free(nodes);

   return result;
}
