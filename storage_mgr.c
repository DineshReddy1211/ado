#include "storage_mgr.h"
#include "dberror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RC_WRITE_NON_EXISTING_PAGE 2


// Function to get the number of pages in a file
int getNumberOfPages(FILE *file) {
    fseek(file, 0L, SEEK_END);
    int numPages = ftell(file) / PAGE_SIZE;
    rewind(file);
    return numPages;
}

void initStorageManager(void) {
    // Initialization code if needed
}

RC createPageFile(char *fileName) {
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Initialize the first page with '\0' bytes
    SM_PageHandle emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    fwrite(emptyPage, sizeof(char), PAGE_SIZE, file);
    free(emptyPage);

    fclose(file);
    return RC_OK;
}

RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    FILE *file = fopen(fileName, "r+");
    if (file == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    // Initialize fHandle fields based on the opened file
    fHandle->fileName = strdup(fileName);
    fHandle->totalNumPages = getNumberOfPages(file);
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = file;

    return RC_OK;
}

RC closePageFile(SM_FileHandle *fHandle) {
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    fclose((FILE *)fHandle->mgmtInfo);
    free(fHandle->fileName);

    return RC_OK;
}

RC destroyPageFile(char *fileName) {
    if (remove(fileName) != 0) {
        return RC_FILE_NOT_FOUND;
    }

    return RC_OK;
}

RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    FILE *file = (FILE *)fHandle->mgmtInfo;
    fseek(file, pageNum * PAGE_SIZE, SEEK_SET);
    fread(memPage, sizeof(char), PAGE_SIZE, file);

    fHandle->curPagePos = pageNum;
    return RC_OK;
}

int getBlockPos(SM_FileHandle *fHandle) {
    return fHandle->curPagePos;
}

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage);
}

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
}

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_WRITE_NON_EXISTING_PAGE;
    }

    FILE *file = (FILE *)fHandle->mgmtInfo;
    fseek(file, pageNum * PAGE_SIZE, SEEK_SET);
    fwrite(memPage, sizeof(char), PAGE_SIZE, file);

    fHandle->curPagePos = pageNum;
    return RC_OK;
}

RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

RC appendEmptyBlock(SM_FileHandle *fHandle) {
    FILE *file = (FILE *)fHandle->mgmtInfo;

    // Move to the end of the file
    fseek(file, 0L, SEEK_END);

    // Append an empty page filled with '\0' bytes
    SM_PageHandle emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    fwrite(emptyPage, sizeof(char), PAGE_SIZE, file);
    free(emptyPage);

    fHandle->totalNumPages++;
    fHandle->curPagePos = fHandle->totalNumPages - 1;
    return RC_OK;
}

RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    if (numberOfPages > fHandle->totalNumPages) {
        int numPagesToAdd = numberOfPages - fHandle->totalNumPages;
        for (int i = 0; i < numPagesToAdd; i++) {
            appendEmptyBlock(fHandle);
        }
    }

    return RC_OK;
}

