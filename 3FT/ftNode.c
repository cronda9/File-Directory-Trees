/*--------------------------------------------------------------------*/
/* fileNode.c                                                         */
/* Author: Christopher Ronda & Benjamin Herber                        */
/*--------------------------------------------------------------------*/

#include <string.h>
#include <assert.h>

#include "dynarray.h"
#include "fileNode.h"

/*
   A pFileNode structure represents a file stored in a directory tree.
*/
struct pFileNode
{
    /* The full path to this file. */
    char *path;

    /* A generic pointer to the contents of this file. */
    void *contents;

    /* Metadata: length of stored file. */
    size_t length;
};

FileNode FileNode_create(const char *path, void *contents, size_t length)
{
    FileNode newFile;
    char *cpyPath;

    assert(path != NULL);
    assert(contents != NULL);
    assert(length != NULL);

    /* Create Defensive copy of path string. */
    cpyPath = (char *)malloc((strlen(path) + 1) * sizeof(char));
    if (cpyPath == NULL)
        return NULL;
    strcpy(cpyPath, path);

    /* Allocate memory and initialize new node. */
    newFile = (FileNode)malloc(sizeof(struct pFileNode));
    if (newFile == NULL) {
        free(cpyPath);
        return NULL;
    }

    newFile->path = cpyPath;
    newFile->contents = contents;
    newFile->length = length;

    return newFile;
}

void FileNode_destroy(FileNode n)
{
    assert(n != NULL);

    free(n->path);
    free(n);
}

void *FileNode_getContents(FileNode n)
{
    assert(n != NULL);
    return (n->contents);
}

void *FileNode_update(FileNode n, void *newContents, size_t newLength)
{
    void *oldContents;

    assert(n != NULL);
    assert(newContents != NULL);
    assert(newLength != NULL);

    oldContents = (n->contents);
    n->contents = newContents;
    n->length = newLength;

    return oldContents;
}

size_t FileNode_stats(FileNode n)
{
    assert(n != NULL);
    return (n->length);
}

char *FileNode_getPath(FileNode n)
{
    assert(n != NULL);
    return (n->path);
}