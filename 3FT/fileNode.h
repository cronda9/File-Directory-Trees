/*--------------------------------------------------------------------*/
/* fileNode.h                                                         */
/* Author: Christopher Ronda & Benjamin Herber                        */
/*--------------------------------------------------------------------*/

#ifndef FILE_NODE_INCLUDED
#define FILE_NODE_INCLUDED

#include <stdlib.h>
#include "a4def.h"

typedef struct pFileNode* FileNode;

FileNode FileNode_create(char* path, void *contents, size_t length);

void FileNode_destroy(FileNode n);

void *FileNode_update(FileNode n, void *newContents, size_t newLength);

int FileNode_stats(FileNode n, boolean* type, size_t* length);

char *FileNode_getPath(FileNode n);

#endif
