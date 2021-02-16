/*
   Maps the created test file in memory and printsits contents.
*/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "../mmap_test_constants.h"


int main(void) {

    ///////////////////////
    /////  VARIABLES  /////
    ///////////////////////

    HANDLE hMapFile;        // handle for the file's memory-mapped region
    HANDLE hFile;           // the file handle
    BOOL bFlag;             // a result holder
    //DWORD dBytesWritten;    // number of bytes written
    //DWORD dwFileSize;       // temporary storage for file sizes
    DWORD dwFileMapSize;    // size of the file mapping
    DWORD dwMapViewSize;    // the size of the view
    DWORD dwFileMapStart;   // where to start the file map view
    DWORD dwSysGran;        // system allocation granularity
    SYSTEM_INFO SysInfo;    // system information; used to get granularity
    LPVOID lpMapAddress;    // pointer to the base address of the memory-mapped region

    char* pData;            // pointer to the data
    //UINT8 letter;           // Letter to fill the file with
    //int i;                  // loop counter
    int iViewDelta;         // the offset into the view where the data shows up



    /////////////////////////////////////
    /////  OPEN EXISTING TEST FILE  /////
    /////////////////////////////////////

    // Create the test file. Open it with "Create Always" to overwrite any existing file.
    hFile = CreateFile(
        lpcTheFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    // Check no errors
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("hFile is NULL\n"));
        _tprintf(TEXT("Target file is %s\n"), lpcTheFile);
        return 4;
    }



    /////////////////////////////////
    /////  CALCULATE VARIABLES  /////
    /////////////////////////////////

    // Get the system allocation granularity.
    GetSystemInfo(&SysInfo);
    dwSysGran = SysInfo.dwAllocationGranularity;

    // Now calculate a few variables. Calculate the file offsets as 64-bit values, and then get the low-order 32 bits for the function calls.

    // To calculate where to start the file mapping, round down the offset of the data into the file to the nearest multiple of the system allocation granularity.
    dwFileMapStart = (FILE_MAP_START / dwSysGran) * dwSysGran;
    _tprintf(TEXT("The file map view starts at %ld bytes into the file.\n"), dwFileMapStart);

    // Calculate the size of the file mapping view.
    dwMapViewSize = (FILE_MAP_START % dwSysGran) + BUFFSIZE;
    _tprintf(TEXT("The file map view is %ld bytes large.\n"), dwMapViewSize);

    // How large will the file mapping object be?
    dwFileMapSize = FILE_MAP_START + BUFFSIZE;
    _tprintf(TEXT("The file mapping object is %ld bytes large.\n"), dwFileMapSize);

    // The data of interest isn't at the beginning of the view, so determine how far into the view to set the pointer.
    iViewDelta = FILE_MAP_START - dwFileMapStart;
    _tprintf(TEXT("The data is %d bytes into the view.\n"), iViewDelta);


    /*
    /////////////////////////////
    /////  FILE POPULATION  /////
    /////////////////////////////

    // Write a file with data suitable for experimentation.
    // Note that this code does not check for storage medium overflow or other errors, which production code should do.
        // If the 1st "WriteFile" is used, provides unique int (4-byte) offsets in the file for easy visual inspection.
        // If the 2nd "WriteFile" is used, provides a file full of the letter provided in the "letter" variable.
    _tprintf(TEXT("CREATING FILE...\n"));
    letter = 'B';
    for (i = 0; i < (int)dwSysGran; i++) {
        //WriteFile(hFile, &i, sizeof(i), &dBytesWritten, NULL);              // Fill with unique ints
        WriteFile(hFile, &letter, sizeof(letter), &dBytesWritten, NULL);    // Fill with provided letter
    }

    // Prints the file size that was written.
    dwFileSize = GetFileSize(hFile, NULL);
    _tprintf(TEXT("hFile size: %10d\n"), dwFileSize);*/



    //////////////////////////
    /////  FILE MAPPING  /////
    //////////////////////////

    // Create a file mapping object for the file. Note that it is a good idea to ensure the file size is not zero
    hMapFile = CreateFileMapping(
        hFile,          // current file handle
        NULL,           // default security
        PAGE_READONLY,  // read only permission
        0,              // size of mapping object, high
        dwFileMapSize,  // size of mapping object, low
        NULL            // name of mapping object
    );
    // Check no errors
    if (hMapFile == NULL) {
        _tprintf(TEXT("hMapFile is NULL: last error: %d\n"), GetLastError());
        return (2);
    }

    // Map the view and test the results.
    lpMapAddress = MapViewOfFile(
        hMapFile,            // handle to mapping object
        FILE_MAP_READ,       // read only
        0,                   // high-order 32 bits of file offset
        dwFileMapStart,      // low-order 32 bits of file offset
        dwMapViewSize        // number of bytes to map
    );
    // Check no errors
    if (lpMapAddress == NULL) {
        _tprintf(TEXT("lpMapAddress is NULL: last error: %d\n"), GetLastError());
        return 3;
    }



    /////////////////////////////
    /////  PRINT TEST FILE  /////
    /////////////////////////////

    // Calculate the pointer to the data.
    pData = (char*)lpMapAddress + iViewDelta;

    // Print data as text
    for (size_t j = 0; j < BUFFSIZE; j++) {
        printf("%c", pData[j]);
        if ((j + 1) % 10 == 0) {
            if (j >= 59) {
                break;
            }
            Sleep(4000);    // in ms (4s)
            printf("\n");
        }
    }


    /////////////////////////////
    /////  CLOSE & CLEANUP  /////
    /////////////////////////////

    // Close the file mapping object
    bFlag = UnmapViewOfFile(lpMapAddress);
    bFlag = CloseHandle(hMapFile); // close the file mapping object

    if (!bFlag) {
        _tprintf(TEXT("\nError %ld occurred closing the mapping object!"), GetLastError());
    }

    // Close the file handle
    bFlag = CloseHandle(hFile);

    if (!bFlag) {
        _tprintf(TEXT("\nError %ld occurred closing the file!"), GetLastError());
    }

    return 0;
}