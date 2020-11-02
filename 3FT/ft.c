/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Christopher Ronda & Benjamin Herber                        */
/* (Adapted from dtGood.c)                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "dynarray.h"
#include "ft.h"
#include "dirNode.h"
#include "fileNode.h"

/*--------------------------------------------------------------------*/
/*
   A File Tree is an Abstract Object with 4 state variables:
*/
/* flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;

/* pointer to the root directory DirNode in the hierarchy */
static DirNode root;

/* Special case scenario where a file is the root, so no sub-dirs. */
static FileNode fileRoot;

/* counter of the number of DirNodes and FileNodes in the hierarchy */
static size_t count;

/*--------------------------------------------------------------------*/
/*
   Starting at the parameter curr, traverses as far down
   the hierarchy as possible while still matching the path
   parameter.

   Returns a pointer to the farthest matching DirNode down that path,
   or NULL if there is no node in curr's hierarchy that matches
   a prefix of the path
*/
static DirNode FT_traversePathFrom(char *path, DirNode curr)
{
   DirNode found;
   size_t i;

   assert(path != NULL);

   /* Base case #1: exhausted path. */
   if (curr == NULL)
      return NULL;

   /* Base case #2: exhausted entire path. */
   else if (!strcmp(path, DirNode_getPath(curr)))
      return curr;

   /* Matching prefix but different paths so recur. */
   else if (!strncmp(path, DirNode_getPath(curr), strlen(DirNode_getPath(curr))))
   {
      for (i = 0; i < Node_getNumDirs(curr); i++)
      {
         found = FT_traversePathFrom(path, Node_getDirChild(curr, i));
         if (found != NULL)
            return found;
      }
      return curr;
   }

   return NULL;
}

/*--------------------------------------------------------------------*/
/*
   Returns the farthest DirNode reachable from the root following a
   given path, or NULL if there is no Node in the hierarchy that
   matches a prefix of the path.
*/
static DirNode FT_traversePath(char *path)
{
   assert(path != NULL);
   return FT_traversePathFrom(path, root);
}

/*--------------------------------------------------------------------*/
/*
   Destroys the entire hierarchy of DirNodes and FileNodes rooted at
   curr, including curr itself.
*/
static void FT_removePathFrom(DirNode curr)
{
   if (curr != NULL)
   {
      count -= DirNode_destroy(curr);
   }
}

/*--------------------------------------------------------------------*/
/*
   Given a prospective parent and child Node,
   adds child to parent's children list, if possible

   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS.
*/
static int FT_linkParentToChild(DirNode parent, DirNode child)
{

   assert(parent != NULL);

   if (DirNode_linkChild(parent, child) != SUCCESS)
   {
      (void)DirNode_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/*--------------------------------------------------------------------*/
/*
   Inserts a new path into the tree rooted at parent, or, if
   parent is NULL, as the root of the data structure. Only insert
   as far as the path prefix of the given path. Store last prefix
   dirNode in memory pointer to by DirNode lastDir and last str token
   in lastToken as well as first newly created dir at newest. lastToken
   can be NULL and if so, it will be ignored.

   If a Node representing path already exists, returns ALREADY_IN_TREE

   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR

   If there is an error linking any of the new nodes,
   returns PARENT_CHID_ERROR

   Otherwise, returns SUCCESS
*/
static int FT_insertRestOfPrefix(char *path, DirNode parent, DirNode *lastDir,
                                 DirNode *firstDir, char **lastToken)
{

   DirNode curr = parent;
   DirNode firstNew = NULL;
   DirNode new;
   char *copyPath;
   char *restPath = path;
   char *dirToken;
   char *nextToken;
   int result;
   size_t newCount = 0;

   assert(path != NULL);
   assert(lastDir != NULL);
   assert(firstDir != NULL);

   /* Check if dir has conflicting path or already in tree. */
   if ((curr == NULL) && (root != NULL))
      return CONFLICTING_PATH;
   else if (!strcmp(path, DirNode_getPath(curr)))
      return ALREADY_IN_TREE;
   else
      restPath += (strlen(DirNode_getPath(curr)) + 1);

   /* Set up tonkenized path iterations. */
   copyPath = malloc(strlen(restPath) + 1);
   if (copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   nextToken = strtok(copyPath, "/");

   /* Create all sub-dirs up until, not including, last child. */
   while (nextToken != NULL)
   {
      dirToken = nextToken;
      nextToken = strtok(NULL, "/");
      new = DirNode_create(dirToken, curr);
      newCount++;

      if (firstNew == NULL)
         firstNew = new;
      else
      {
         result = FT_linkParentToChild(curr, new);
         if (result != SUCCESS)
         {
            (void)Node_destroy(new);
            (void)Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }

      if (new == NULL)
      {
         (void)Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }

      curr = new;
   }

   /* Store last and first created DirNode as well as last token. */
   if (lastToken != NULL)
   {
      *lastToken = (char *)malloc(strlen(dirToken) + 1);
      if (*lastToken == NULL)
         return MEMORY_ERROR;
      strcpy(*lastToken, dirToken);
   }
   *lastDir = curr;
   *firstDir = firstNew;
   free(copyPath);

   count += newCount;
   return SUCCESS;
}

/*--------------------------------------------------------------------*/
int FT_insertDir(char *path)
{
   size_t oldCount;
   DirNode curr;
   DirNode parent = curr;
   DirNode lastInsert;
   DirNode firstInsert;
   char *lastToken;
   int result;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* Check if limited by file root. */
   if (fileRoot != NULL)
      return CONFLICTING_PATH;

   oldCount = count;

   /* Check and insert prefix. */
   curr = FT_traversePath(path);
   result = FT_insertRestOfPrefix(path, curr, &lastInsert, &firstInsert, &lastToken);
   if (result != SUCCESS)
      return result;

   /* Insert last directory. */
   curr = DirNode_create(lastToken, lastInsert);
   if (curr == NULL)
   {
      DirNode_destroy(firstInsert);
      count = oldCount;
   }
   count++;
   result = FT_linkParentToChild(lastInsert, curr);
   if (result != SUCCESS)
   {
      DirNode_destroy(firstInsert);
      DirNode_destroy(curr);
      count = oldCount;
   }

   /* Do final link. */
   if (parent == NULL)
   {
      root = firstInsert;
      return SUCCESS;
   }
   else
   {
      result = FT_linkParentToChild(parent, firstInsert);
      if (result != SUCCESS)
      {
         (void)Node_destroy(firstInsert);
         count = oldCount;
      }

      return result;
   }
}

/*--------------------------------------------------------------------*/
boolean FT_containsDir(char *path)
{
   DirNode curr;
   boolean result;

   assert(path != NULL);

   if (!isInitialized)
      return FALSE;

   /* Address if file is the root. */
   if (fileRoot != NULL)
      return FALSE;

   /* Traverse as far down dir path as possible. */
   curr = FT_traversePath(path);

   /* Root conflict. */
   if (curr == NULL)
      return FALSE;
   
   /* Check if paths are equal. */
   return (strcmp(path, DirNode_getPath(curr)) == 0);
}

/*--------------------------------------------------------------------*/
/*
  Removes the directory hierarchy rooted at path starting from Node
  curr. If curr is the data structure's root, root becomes NULL.

  Returns NO_SUCH_PATH if curr is not the Node for path,
  and SUCCESS otherwise.
 */
static int FT_rmPathAt(char *path, DirNode curr)
{

   DirNode parent;

   assert(path != NULL);
   assert(curr != NULL);

   parent = Node_getParent(curr);

   if (!strcmp(path, Node_getPath(curr)))
   {
      if (parent == NULL)
         root = NULL;
      else
         Node_unlinkChild(parent, curr);

      FT_removePathFrom(curr);

      return SUCCESS;
   }
   else
      return NO_SUCH_PATH;
}

/*--------------------------------------------------------------------*/
int FT_rmDir(char *path)
{
   DirNode curr;
   int result;

   assert(path != NULL);

   if (!isInitialized)
      return INITIALIZATION_ERROR;

   /* See if trying to traverse over file if root. */
   if (fileRoot && strncmp(path, FileNode_getPath(fileRoot), strlen(fileRoot)))
      return NOT_A_DIRECTORY;

   /* Traverse prefix as far as possible. */
   curr = FT_traversePath(path);
   if (curr == NULL)
      return NO_SUCH_PATH;

   /* Test if trying to go through file instead of dir. */
   if (DirNode_hasFileChild(curr, path, NULL) == TRUE) /* !! NEEDS TO ALSO HANDLE IF FILE IS IN MIDDLE OF PATH!! */
      return NOT_A_DIRECTORY;

   result = FT_rmPathAt(path, curr);

   return result;
}

/*--------------------------------------------------------------------*/
int FT_init(void)
{
   if (isInitialized)
      return INITIALIZATION_ERROR;

   isInitialized = TRUE;
   root = NULL;
   fileRoot = NULL;
   count = 0;
   return SUCCESS;
}

/*--------------------------------------------------------------------*/
int FT_destroy(void)
{
   if (!isInitialized)
      return INITIALIZATION_ERROR;

   FT_removePathFrom(root);

   root = NULL;
   fileRoot = NULL;
   isInitialized = FALSE;
   return SUCCESS;
}

/*--------------------------------------------------------------------*/
/*
   Performs a pre-order traversal of the tree rooted at n,
   inserting each payload to DynArray_T d beginning at index i.
   Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(DirNode n, DynArray_T d, size_t i)
{
   size_t c;
   FileNode fileChild;

   assert(d != NULL);

   if (n != NULL)
   {
      (void)DynArray_set(d, i, DirNode_getPath(n));
      i++;

      /* Traverse File children first. */
      for (c = 0; c < DirNode_getNumFiles(n); c++, i++)
      {
         fileChild = DirNode_getFileChild(n, c);
         (void)DynArray_set(d, i, FileNode_getPath(fileChild));
      }

      /* Traverse Dir children last. */
      for (c = 0; c < DieNode_getNumDirs(n); c++)
         i = FT_preOrderTraversal(DirNode_getDirChild(n, c), d, i);
   }
   return i;
}

/*--------------------------------------------------------------------*/
/*
   Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length.
*/
static void FT_strlenAccumulate(char *str, size_t *pAcc)
{
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
static void FT_strcatAccumulate(char *str, char *acc)
{
   assert(acc != NULL);

   if (str != NULL)
      strcat(acc, str);
   strcat(acc, "\n");
}

/*--------------------------------------------------------------------*/
char *FT_toString(void)
{
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char *result = NULL;

   if (!isInitialized)
      return NULL;

   /* Check for file root. */
   if (fileRoot != NULL)
   {
      char *tmpStr = FileNode_getPath(fileRoot);
      result = (char *)malloc(strlen(tmpStr) + 2);
      if (result == NULL)
         return NULL;
      result = stpcpy(result, tmpStr);
      *result = '\n';
      *(result + 1) = '\0';
      return result;
   }

   nodes = DynArray_new(count);
   (void)FT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void *))FT_strlenAccumulate, (void *)&totalStrlen);

   result = malloc(totalStrlen);
   if (result == NULL)
   {
      DynArray_free(nodes);
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void *))FT_strcatAccumulate, (void *)result);

   DynArray_free(nodes);
   return result;
}
