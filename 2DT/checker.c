/*--------------------------------------------------------------------*/
/* checker.c                                                          */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checker.h"


/* see checker.h for specification */
boolean Checker_Node_isValid(Node n) {
   Node parent;
   Node prevNode;
   Node currNode;
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;
   size_t j;
   
   /* Sample check: a NULL pointer is not a valid Node */
   if(n == NULL) {
      fprintf(stderr, "Node is a NULL pointer\n");
      return FALSE;
   }

   if(Node_getPath(n)==NULL){
      fprintf(stderr, "Nodes path is null\n");
      return FALSE;
   }
   
   parent = Node_getParent(n);
   if(parent != NULL) {
      npath = Node_getPath(n);
   
      /* Sample check that parent's path must be prefix of n's path */
      ppath = Node_getPath(parent);
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path\n");
         return FALSE;
      }
      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }
   }

   /* Check if the the path is sorted */
   prevNode = Node_getChild(n,0);
   if(prevNode != NULL)
      for(j = 1; j< Node_getNumChildren(n); j++){
         currNode = Node_getChild(n,j);
         if(Node_compare(prevNode, currNode) >= 0){
            fprintf(stderr, "Children are not in sorted order\n");
            return FALSE;
         }
         prevNode = currNode;
      }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Takes size_t pointer cnt and increments it for each node.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise. 

*/
static boolean Checker_treeCheck(Node n, size_t* cnt) {
   size_t c;

   assert(cnt != NULL);
   
   if(n != NULL) {

      /* Increment num of nodes found */
      (*cnt)++;

      /* Sample check on each non-root Node: Node must be valid */
      /* If not, pass that failure back up immediately */
      if(!Checker_Node_isValid(n))
         return FALSE;


      for(c = 0; c < Node_getNumChildren(n); c++)
      {
         Node child = Node_getChild(n, c);

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!Checker_treeCheck(child, cnt))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checker.h for specification */
boolean Checker_DT_isValid(boolean isInit, Node root, size_t count) {
   size_t cnt = 0;
   boolean traversalResult;

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!isInit){
      if(count != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
   }
   /* Checks if root nodes parent is set to NULL */
   if(root != NULL && Node_getParent(root) != NULL){
      fprintf(stderr, "Roots parent node is not NULL\n");
      return FALSE;
   }

   /* Now checks invariants recursively at each Node from the root. */
   traversalResult = Checker_treeCheck(root, &cnt);
   if(traversalResult == FALSE)
      return FALSE;
   /* Checks if count is correct */
   else if(cnt != count){
      fprintf(stderr, "Incorrect count of nodes\n");
      return FALSE;
   }
   return TRUE;
}
