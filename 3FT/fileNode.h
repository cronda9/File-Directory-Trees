/*--------------------------------------------------------------------*/
/* fileNode.h                                                         */
/* Author: Christopher Ronda & Benjamin Herber                        */
/*--------------------------------------------------------------------*/

#ifndef FILE_NODE_INCLUDED
#define FILE_NODE_INCLUDED

#include "a4def.h"
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*
    FileNode is an object that stores the contents and some metadata
    (path, length) associated with a given file.
*/
typedef struct pFileNode *FileNode;

/*--------------------------------------------------------------------*/
/*
    Create a FileNode with given string path, generic pointer to the
    file's contents and the size_t length of the given contents.
    Creates a defensive copy of given path. Upon failure to create
    node to memory allocation or otherwise, return NULL. Return new
    FileNode upon success.
*/
FileNode FileNode_create(const char *path, void *contents,
                         size_t length);

/*--------------------------------------------------------------------*/
/*
    Take a FileNode n as argument and free associated memory used.
*/
void FileNode_destroy(FileNode n);

/*--------------------------------------------------------------------*/
/*
    Take an existing FileNode n as argument and return stored
    contents associated with that file.
*/
void *FileNode_getContents(FileNode n);

/*--------------------------------------------------------------------*/
/*
    Take an existing FileNode n and updates its contents with
    the generic pointer newContents and stored length with newLength.
    Return old contents upon success and NULL upon failure.
*/
void *FileNode_update(FileNode n, void *newContents, size_t newLength);

/*--------------------------------------------------------------------*/
/*
    Take an existing FileNode n and tries to retrieve the metadata
    length associated with it. Return length stored in node's metadata.
*/
size_t FileNode_getStats(FileNode n);

/*--------------------------------------------------------------------*/
/*
    Take an existing FileNode n as argument and return associated path
    if successful, otherwise return NULL.
*/
char *FileNode_getPath(FileNode n);

/*--------------------------------------------------------------------*/
/*
    Take two FileNodes n1 and n2 as argument and compare their paths
    lexigraphically.

    Return < 0 if n1 < n2
    Return > 0 if n1 > n2
    Return = 0 if n1 = n2
*/
int FileNode_compare(FileNode n1, FileNode n2);

#endif
