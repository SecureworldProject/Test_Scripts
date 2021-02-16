#pragma once
#include <windows.h>
#include <tchar.h>

#define BUFFSIZE 1024       // Mapping examination size (not full size)
#define FILE_MAP_START 0    // Offset from the the file start to the mapping start

const TCHAR* lpcTheFile = TEXT("fmtest.txt");   // The file to be manipulated
