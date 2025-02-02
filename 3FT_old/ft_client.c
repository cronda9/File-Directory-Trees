/*--------------------------------------------------------------------*/
/* ft_client.c                                                        */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ft.h"

/* Tests the FT implementation with an assortment of checks.
   Prints the status of the data structure along the way to stderr.
   Returns 0. */
int main(void) {
  char* temp;
  boolean b;
  size_t l;

  /* Before the data structure is initialized, insert*, remove*,
     and destroy operations should return INITIALIZATION_ERROR, and
     containsDir should return FALSE for any non-NULL argument, and
     toString should return NULL. */
  assert(FT_insertDir("a/b/c") == INITIALIZATION_ERROR);
  assert(FT_containsDir("a/b/c") == FALSE);
  assert(FT_insertFile("a/b/c/D",NULL,0) == INITIALIZATION_ERROR);
  assert(FT_containsDir("a/b/c/D") == FALSE);
  assert((temp = FT_toString()) == NULL);

  /* After initialization, the data structure is empty, so
     contains* should still return FALSE for any non-NULL string,
     and toString should return the empty string.*/
  assert(FT_init() == SUCCESS);
  assert(FT_containsDir("a/b/c") == FALSE);
  assert(FT_containsDir("a/b/c/D") == FALSE);
  assert((temp = FT_toString()) != NULL);
  assert(!strcmp(temp,""));
  free(temp);

  /* After insertion, the data structure should contain every prefix
     of the inserted path, toString should return a string with these
     prefixes, trying to insert it again should return
     ALREADY_IN_TREE, and trying to insert some other root should
     return CONFLICTING_PATH. */
  assert(FT_insertDir("a/b/c") == SUCCESS);
  assert(FT_containsDir("a") == TRUE);
  assert(FT_containsFile("a") == FALSE);
  assert(FT_containsDir("a/b") == TRUE);
  assert(FT_containsFile("a/b") == FALSE);
  assert(FT_containsDir("a/b/c") == TRUE);
  assert(FT_containsFile("a/b/c") == FALSE);
  assert(FT_insertFile("a/d/A", "hello, world!", 14) == SUCCESS);
  assert(FT_containsDir("a/d") == TRUE);
  assert(FT_containsFile("a/d") == FALSE);
  assert(FT_containsDir("a/d/A") == FALSE);
  assert(FT_containsFile("a/d/A") == TRUE);
  assert((temp = FT_toString()) != NULL);
  free(temp);
  assert(FT_insertDir("a/b/c") == ALREADY_IN_TREE);
  assert(FT_insertFile("a/d/A", NULL, 0) == ALREADY_IN_TREE);
  assert(FT_insertDir("d/e/f") == CONFLICTING_PATH);
  assert(FT_insertFile("d/D", NULL, 0) == CONFLICTING_PATH);

  /* Trying to insert a third child should succeed, unlike in BFT*/
  assert(FT_insertDir("a/g") == SUCCESS);
  assert(FT_containsDir("a/g") == TRUE);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);

  /* Children must be unique, but individual directories or files
     in different paths needn't be */
  assert(FT_insertFile("a/b/c", NULL, 0) == ALREADY_IN_TREE);
  assert(FT_insertDir("a/d/A") == ALREADY_IN_TREE);
  assert(FT_insertDir("a/b/d/e") == SUCCESS);
  assert(FT_containsDir("a/b/d/e") == TRUE);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);
  assert(FT_insertDir("a/b/d/e/e") == SUCCESS);
  assert(FT_insertFile("a/b/d/e/e/A", "goodbye, world!", 16) == SUCCESS);
  assert(FT_containsDir("a/b/d/e/e") == TRUE);
  assert(FT_containsFile("a/b/d/e/e/A") == TRUE);
  assert(FT_containsDir("a/b/d/e/f") == FALSE);
  assert(FT_containsDir("a/b/d/e/e/e") == FALSE);
  assert(FT_containsDir("a/b/d/e/e/f") == FALSE);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);

  /* calling rm* on a path that doesn't exist should return
     NO_SUCH_PATH, but on a path that does exist should return
     SUCCESS and remove entire subtree rooted at that path  */
  assert(FT_containsDir("a/b/d/e") == TRUE);
  assert(FT_containsDir("a/b/d/e/e") == TRUE);
  assert(FT_containsFile("a/b/d/e/e/A") == TRUE);
  assert(FT_containsDir("a/b/d/e/f") == FALSE);
  assert(FT_rmDir("a/b/d/e/f") == NO_SUCH_PATH);
  assert(FT_rmDir("a/b/d/e/e/A") == NOT_A_DIRECTORY);
  assert(FT_rmFile("a/b/d/e") == NOT_A_FILE);
  assert(FT_rmDir("a/b/d/e") == SUCCESS);
  assert(FT_containsDir("a/b/d") == TRUE);
  assert(FT_containsDir("a/b/d/e") == FALSE);
  assert(FT_containsDir("a/b/d/e/e") == FALSE);
  assert(FT_containsFile("a/b/d/e/e/A") == FALSE);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);

  /* removing the root doesn't uninitialize the structure */
  assert(FT_rmDir("a") == SUCCESS);
  assert(FT_rmDir("a") == NO_SUCH_PATH);

  /* putting a file at the root is legal, but limiting */
  assert(FT_insertFile("A",NULL,0) == SUCCESS);
  assert(FT_insertFile("B",NULL,0) == CONFLICTING_PATH);
  assert(FT_insertDir("a") == CONFLICTING_PATH);
  assert(FT_insertFile("b/B",NULL,0) == CONFLICTING_PATH);

  /* file contents work as expected */
  assert(FT_getFileContents("A") == NULL);
  assert(FT_stat("A", &b, &l) == SUCCESS);
  assert(b == TRUE);
  assert(l == 0);
  assert(FT_replaceFileContents("A","Kernighan",10) == NULL);
  assert(!strcmp((char *)FT_getFileContents("A"), "Kernighan"));
  assert(FT_stat("A", &b, &l) == SUCCESS);
  assert(b == TRUE);
  assert(l == 10);
  assert(!strcmp(FT_replaceFileContents("A",calloc(1000,1),1000), "Kernighan"));
  assert(!strcmp((char*)FT_getFileContents("A"),""));
  assert(FT_stat("A", &b, &l) == SUCCESS);
  assert(b == TRUE);
  assert(l == 1000);
  assert(FT_rmFile("A") == SUCCESS);
  assert(FT_insertDir("a/z") == SUCCESS);
  assert(FT_stat("a/z", &b, &l) == SUCCESS);
  assert(b == FALSE);
  assert(l == 1000);

  /* children should be printed in lexicographic order,
     depth first, file children before directory children */
  assert(FT_insertDir("a/y") == SUCCESS);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);
  assert(FT_insertDir("a/x") == SUCCESS);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);
  assert(FT_insertFile("a/x/C", "Ritchie", 8) == SUCCESS);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);
  assert(FT_insertFile("a/x/B", "Thompson", 9) == SUCCESS);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);
  assert(FT_insertDir("a/y/CHILD1DIR") == SUCCESS);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);
  assert(FT_insertDir("a/y/CHILD2DIR") == SUCCESS);
  assert(FT_containsDir("a/y/CHILD2DIR"));
  assert(FT_insertFile("a/y/CHILD2FILE", NULL, 0) == SUCCESS); /* DynArray pblm. */
  assert(FT_insertDir("a/y/CHILD3DIR") == SUCCESS);
  assert(FT_insertFile("a/y/CHILD1FILE", NULL, 0) == SUCCESS);
  assert(FT_insertDir("a/y/CHILD2DIR/CHILD4DIR") == SUCCESS);
  assert((temp = FT_toString()) != NULL);
  fprintf(stderr, "%s\n", temp);
  free(temp);

  assert(FT_destroy() == SUCCESS);
  assert(FT_destroy() == INITIALIZATION_ERROR);
  assert(FT_containsDir("a") == FALSE);
  assert(FT_containsFile("a") == FALSE);

  return 0;
}
