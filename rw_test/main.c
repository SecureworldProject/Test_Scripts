/////  FILE INCLUDES  /////
#include <windows.h>
#include <stdio.h>
#include <inttypes.h>
#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib")




/////  DEFINITIONS  /////

#define NOOP ((void)0)
#define ENABLE_PRINTS 0					// Affects the PRINT() functions. If 0 does not print anything. If 1 traces are printed.
#define PRINT(...) do { if (ENABLE_PRINTS) printf(__VA_ARGS__); else NOOP;} while (0)
#define PRINT_HEX(BUF, BUF_SIZE) do { if (ENABLE_PRINTS) print_hex(#BUF, BUF, BUF_SIZE); else NOOP;} while (0)

#define MAX_INPUT_LENGTH 500



/////  FUNCTION PROTOTYPES  /////
DWORD print_hex(char* buf_name, void* buf, size_t size);
DWORD getFileSize(uint64_t * file_size, HANDLE handle, WCHAR * file_path);
DWORD readMenu();
DWORD writeMenu();
DWORD getDataToWrite(byte * write_buffer, DWORD write_length);


/////  FUNCTION DEFINITIONS  /////

DWORD print_hex(char* buf_name, void* buf, size_t size) {
	printf("First %llu bytes of %s contain:\n", size, buf_name);

	char* full_str = NULL;
	char* target_str = NULL;
	size_t written_bytes = 0;
	size_t full_str_size = 0;

	// Size of string will consist on:
	//   (size*3)			 - 3 characters for every byte (2 hex characters plus 1 space). Space changed for '\n' every 32 bytes
	//   (size/8 - size/32)	 - Every 8 bytes another space is added after the space (if it is not multiple of 32, which already has '\n' instead)
	//   (1 + 1)			 - A '\n' and a '\0' is added at the end
	full_str_size = (size * 3) + (size / 8 - size / 32) + (1+1);
	full_str = malloc(full_str_size * sizeof(char));
	if (full_str == NULL) {
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	target_str = full_str;

	for (int i = 0; i < size; i++) {
		if ((i + 1) % 32 == 0) {
			written_bytes += (size_t)sprintf_s(target_str, full_str_size - written_bytes, "%02hhX\n", ((byte*)buf)[i]);
		} else if ((i + 1) % 8 == 0) {
			written_bytes += (size_t)sprintf_s(target_str, full_str_size - written_bytes, "%02hhX  ", ((byte*)buf)[i]);
		} else {
			written_bytes += (size_t)sprintf_s(target_str, full_str_size - written_bytes, "%02hhX ", ((byte*)buf)[i]);
		}
		target_str = full_str + written_bytes;

	}
	sprintf_s(target_str, full_str_size - written_bytes, "\n");
	printf(full_str);
	free(full_str);

	return ERROR_SUCCESS;
}

DWORD getFileSize(uint64_t* file_size, HANDLE handle, WCHAR* file_path) {
	BOOL opened = FALSE;
	DWORD error_code = ERROR_SUCCESS;

	// Ensure handle is valid (reopen the file if necessary)
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		PRINT("Invalid file handle\n");
		handle = CreateFile(file_path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			error_code = GetLastError();
			PRINT("\tERROR creating handle to get file size (%d)\n", error_code);
			return error_code;
		}
		opened = TRUE;
	}

	// Using GetFileSizeEx() and passing directly the file_size pointer.
	// Maybe should check file_size > 0 (although that would mean that file_size > 8 EiB = 2^63 Bytes)
	if (!GetFileSizeEx(handle, (PLARGE_INTEGER)file_size)) {
		error_code = GetLastError();
		PRINT("\tcan not get a file size error = %d\n", error_code);
		if (opened)
			CloseHandle(handle);
		return error_code;
	};

	return error_code;
}

DWORD readMenu() {
	WCHAR file_path[MAX_PATH] = { 0 };
	WCHAR line[MAX_INPUT_LENGTH] = { 0 };
	BOOL opened = FALSE;
	HANDLE handle = INVALID_HANDLE_VALUE;
	size_t file_size = 0;
	size_t read_offset = 0;
	DWORD read_length = 0;
	DWORD bytes_read = 0;
	byte* read_buffer = NULL;
	BOOL valid_value = FALSE;
	DWORD error_code = ERROR_SUCCESS;

	//printf("\nYou have entered the READ MENU.\n");
	printf("\n\n");
	printf("       _____                _ _______        _    \n");
	printf("      |  __ \\              | |__   __|      | |   \n");
	printf("      | |__) |___  __ _  __| |  | | ___  ___| |_  \n");
	printf("      |  _  // _ \\/ _` |/ _` |  | |/ _ \\/ __| __| \n");
	printf("      | | \\ \\  __/ (_| | (_| |  | |  __/\\__ \\ |_  \n");
	printf("      |_|  \\_\\___|\\__,_|\\__,_|  |_|\\___||___/\\__| \n");
	printf("\n\n");

	/*
	It looks like this (but needs to duplicate backslashes to scape them)
	  _____                _ _______        _
	 |  __ \              | |__   __|      | |
	 | |__) |___  __ _  __| |  | | ___  ___| |_
	 |  _  // _ \/ _` |/ _` |  | |/ _ \/ __| __|
	 | | \ \  __/ (_| | (_| |  | |  __/\__ \ |_
	 |_|  \_\___|\__,_|\__,_|  |_|\___||___/\__|

	*/


	// Get the path
	valid_value = FALSE;
	printf("\nEnter the full path of the file from which you want to read.\n");
	do {
		printf("--> ");
		if (fgetws(file_path, MAX_PATH, stdin)) {				// fgets() ensures that string ends with '\0'
			file_path[wcscspn(file_path, L"\n")] = '\0';		// Remove trailing '\n'

			// Validation checks
			if (!PathFileExistsW(file_path)) {
				printf("\tThe specified path ('%ws') does not exist.\n\n", file_path);
				valid_value = FALSE;
				continue;
				//error_code = ERROR_PATH_NOT_FOUND;
				//goto CLEANUP_READ;
			}
			if (PathIsDirectoryW(file_path)) {
				printf("\tThe specified path ('%ws') matches a directory not a file.\n\n", file_path);
				valid_value = FALSE;
				continue;
				//error_code = ERROR_FILE_NOT_FOUND;
				//goto CLEANUP_READ;
			}

			handle = CreateFileW(
				file_path,				// Name of the file
				GENERIC_READ,			// Open for reading
				0,						// Do not share
				NULL,					// Default security
				OPEN_EXISTING,			// Open existing file only
				FILE_ATTRIBUTE_NORMAL,	// Normal file
				NULL);					// No attr. template

			if (handle == INVALID_HANDLE_VALUE) {
				printf("\tThe specified file ('%ws') cannot be opened.\n\n", file_path);
				valid_value = FALSE;
				continue;
				//error_code = ERROR_OPEN_FAILED;
				//goto CLEANUP_READ;
			}
			opened = TRUE;
		}
	} while (wcslen(file_path) <= 0 || !opened);

	// Check file size
	error_code = getFileSize(&file_size, handle, file_path);
	if (error_code != ERROR_SUCCESS) {
		printf("\nError obtaining file size\n");
		goto CLEANUP_READ;
	}
	printf("\nThe detected file size is %llu\n", file_size);
	if (file_size == 0) {
		printf("\nCannot read anything from an empty file...\n");
		goto CLEANUP_READ;
	}

	// Get the read offset
	valid_value = FALSE;
	printf("\nEnter the position (as byte offset) you want to start reading from.\n");
	do {
		printf("--> ");
		line[0] = '\0';
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {			// fgets() ensures that string ends with '\0'
			if (1 == swscanf_s(line, L"%llu", &read_offset)) {
				valid_value = TRUE;
				// Validation checks
				if (!(read_offset < file_size)) {
					valid_value = FALSE;
					printf("\t[Incorrect: 'read_offset' must be lower than 'file_size']\n\n");
				}
				if (!(read_offset >= 0)) {						// Useless, swscanf_s ensures conversion and size_t is unsigned
					valid_value = FALSE;
					printf("\t[Incorrect: 'read_offset' must be bigger than zero]\n\n");
				}
			}
		}
	} while (!valid_value);

	// Get the read length
	valid_value = FALSE;
	printf("\nEnter the length of the read.\n");
	do {
		printf("--> ");
		line[0] = '\0';
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {			// fgets() ensures that string ends with '\0'
			if (1 == swscanf_s(line, L"%lu", &read_length)) {
				// Validation checks
				valid_value = TRUE;
				if (!(read_offset + read_length <= file_size)) {
					//valid_value = FALSE;
					read_length = file_size - read_offset;
					printf("\t[Warning: 'read_offset' + 'read_length' should be lower than 'file_size'. Value of 'read_length' adjusted to %lu.]\n\n", read_length);
				}
				if (!(read_length > 0)) {						// Useless, swscanf_s ensures conversion and DWORD is unsigned
					valid_value = FALSE;
					printf("\t[Incorrect: 'read_length' must be positive]\n\n");
				}
			}
		}
	} while (!valid_value);

	// Set handle offset
	LARGE_INTEGER large_int_read_offset = { 0 };
	large_int_read_offset.QuadPart = read_offset;
	if (!SetFilePointerEx(handle, large_int_read_offset, NULL, FILE_BEGIN)) {
		printf("\nSetFilePointerEx error (%lu), seeking to offset = %lld\n", GetLastError(), read_offset);
	} else {
		PRINT("\nSetFilePointerEx worked\n");
	}

	// Allocate memory for the buffer
	read_buffer = malloc(read_length * sizeof(byte));
	if (read_buffer == NULL) {
		printf("\nCould not allocate memory (%lu bytes) for the read_buffer\n", read_length);
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CLEANUP_READ;
	} else {
		PRINT("\nMemory allocated (%lu bytes) for the read_buffer\n", read_length);
	}

	PRINT("Reading file...\n");
	// Do the read operation requested
	valid_value = FALSE;
	valid_value = ReadFile(
		handle,			// Open file handle
		read_buffer,	// Pointer to the beginning of data to read
		read_length,	// Number of bytes to read
		&bytes_read,	// Number of bytes that were read
		NULL			// No overlap
	);
	PRINT("Read ended (%lu bytes read)\n", bytes_read);

	if (valid_value) {
		printf("\n_________________________________________ READ OPERATION _________________________________________\n");
		printf("file_path: '%ws'\n", file_path);
		printf("read_offset: '%llu'\n", read_offset);
		printf("read_length: '%lu'\n", read_length);
		printf("bytes_read: '%lu'\n", bytes_read);
		printf("\n");
		print_hex("read_buffer", read_buffer, (size_t)bytes_read);
		printf("\n");
		printf("As string (stops on first '\\0'): '%.*s'\n", (unsigned int)bytes_read, read_buffer);
		printf("\n__________________________________________________________________________________________________\n");
	} else {
		printf("\nError reading file (%lu)\n", GetLastError());
		error_code = ERROR_READ_FAULT;
	}

	CLEANUP_READ:
	if (opened) {
		CloseHandle(handle);
	}
	if (read_buffer != NULL) {
		free(read_buffer);
	}

	return error_code;
}

DWORD writeMenu() {
	WCHAR file_path[MAX_PATH] = { 0 };
	WCHAR line[MAX_INPUT_LENGTH] = { 0 };
	BOOL opened = FALSE;
	HANDLE handle = INVALID_HANDLE_VALUE;
	size_t file_size = 0;
	size_t write_offset = 0;
	DWORD write_length = 0;
	DWORD bytes_written = 0;
	byte* write_buffer = NULL;
	BOOL valid_value = FALSE;
	DWORD error_code = ERROR_SUCCESS;

	//printf("\nYou have entered the WRITE MENU.\n");
	printf("\n\n");
	printf("      __          __   _ _    _______        _    \n");
	printf("      \\ \\        / /  (_) |  |__   __|      | |   \n");
	printf("       \\ \\  /\\  / / __ _| |_ ___| | ___  ___| |_  \n");
	printf("        \\ \\/  \\/ / '__| | __/ _ \\ |/ _ \\/ __| __| \n");
	printf("         \\  /\\  /| |  | | ||  __/ |  __/\\__ \\ |_  \n");
	printf("          \\/  \\/ |_|  |_|\\__\\___|_|\\___||___/\\__| \n");
	printf("\n\n");

	/*
	It looks like this (but needs to duplicate backslashes to scape them)
	 __          __   _ _    _______        _
	 \ \        / /  (_) |  |__   __|      | |
	  \ \  /\  / / __ _| |_ ___| | ___  ___| |_
	   \ \/  \/ / '__| | __/ _ \ |/ _ \/ __| __|
		\  /\  /| |  | | ||  __/ |  __/\__ \ |_
		 \/  \/ |_|  |_|\__\___|_|\___||___/\__|
	*/

	// Get the path
	valid_value = FALSE;
	printf("\nEnter the full path of the file in which you want to write.\n");
	do {
		printf("--> ");
		if (fgetws(file_path, MAX_PATH, stdin)) {				// fgets() ensures that string ends with '\0'
			file_path[wcscspn(file_path, L"\n")] = '\0';		// Remove trailing '\n'

			// Validation checks
			if (!PathFileExistsW(file_path)) {
				printf("\tThe specified path ('%ws') does not exist.\n\n", file_path);
				valid_value = FALSE;
				continue;
				//error_code = ERROR_PATH_NOT_FOUND;
				//goto CLEANUP_WRITE;
			}
			if (PathIsDirectoryW(file_path)) {
				printf("\tThe specified path ('%ws') matches a directory not a file.\n\n", file_path);
				valid_value = FALSE;
				continue;
				//error_code = ERROR_FILE_NOT_FOUND;
				//goto CLEANUP_WRITE;
			}

			handle = CreateFileW(
				file_path,				// Name of the file
				GENERIC_WRITE,			// Open for writing
				0,						// Do not share
				NULL,					// Default security
				OPEN_EXISTING,			// Open existing file only
				FILE_ATTRIBUTE_NORMAL,	// Normal file
				NULL);					// No attr. template

			if (handle == INVALID_HANDLE_VALUE) {
				printf("\tThe specified file ('%ws') cannot be opened.\n\n", file_path);
				valid_value = FALSE;
				continue;
				//error_code = ERROR_OPEN_FAILED;
				//goto CLEANUP_WRITE;
			}
			opened = TRUE;
		}
	} while (wcslen(file_path) <= 0 || !opened);

	// Check file size
	error_code = getFileSize(&file_size, handle, file_path);
	if (error_code != ERROR_SUCCESS) {
		printf("Error obtaining file size\n");
		goto CLEANUP_WRITE;
	}
	printf("\nThe detected file size is %llu\n", file_size);

	// Get the write offset
	valid_value = FALSE;
	printf("\nEnter the position (as byte offset) you want to start writing into.\n");
	do {
		printf("--> ");
		line[0] = '\0';
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {			// fgets() ensures that string ends with '\0'
			if (1 == swscanf_s(line, L"%llu", &write_offset)) {
				valid_value = TRUE;
				// Validation checks
				/*if (!(write_offset < file_size)) {
					valid_value = FALSE;
					printf("\t[Incorrect: 'write_offset' must be lower than 'file_size']\n");
				}*/
				if (!(write_offset >= 0)) {						// Useless, swscanf_s ensures conversion and size_t is unsigned
					valid_value = FALSE;
					printf("\t[Incorrect: 'write_offset' must be positive]\n\n");
				}
			}
		}
	} while (!valid_value);

	// Get the bytes to write
	valid_value = FALSE;
	printf("\nEnter the length of the write.\n");
	do {
		printf("--> ");
		line[0] = '\0';
		if (fgetws(line, MAX_INPUT_LENGTH, stdin)) {			// fgets() ensures that string ends with '\0'
			if (1 == swscanf_s(line, L"%lu", &write_length)) {
				// Validation checks
				valid_value = TRUE;
				/*if (!(write_offset + write_length <= file_size)) {
					valid_value = FALSE;
					printf("\t[Incorrect: 'write_offset' + 'write_length' must be lower than 'file_size']\n");
				}*/
				if (!(write_length > 0)) {						// Useless, swscanf_s ensures conversion and DWORD is unsigned
					valid_value = FALSE;
					printf("\t[Incorrect: 'write_length' must be positive]\n\n");
				}
			}
		}
	} while (!valid_value);

	// Set handle offset
	LARGE_INTEGER large_int_read_offset = { 0 };
	large_int_read_offset.QuadPart = write_offset;
	if (!SetFilePointerEx(handle, large_int_read_offset, NULL, FILE_BEGIN)) {
		printf("\nSetFilePointerEx error (%lu), seeking to offset = %lld\n", GetLastError(), write_offset);
		error_code = -1;
		goto CLEANUP_WRITE;
	} else {
		PRINT("\nSetFilePointerEx worked\n");
	}

	// Allocate memory for the buffer
	write_buffer = malloc(write_length * sizeof(byte));
	if (write_buffer == NULL) {
		printf("\nCould not allocate memory (%lu bytes) for the write_buffer\n", write_length);
		error_code = ERROR_NOT_ENOUGH_MEMORY;
		goto CLEANUP_WRITE;
	} else {
		PRINT("\nMemory allocated (%lu bytes) for the write_buffer\n", write_length);
	}

	// Get the data to write
	error_code = getDataToWrite(write_buffer, write_length);
	if (error_code != ERROR_SUCCESS) {
		goto CLEANUP_WRITE;
	}

	PRINT("Writing file...\n");
	// Do the write operation requested
	valid_value = FALSE;
	valid_value = WriteFile(
		handle,			// Open file handle
		write_buffer,	// Pointer to the beginning of data to write
		write_length,	// Number of bytes to write
		&bytes_written,	// Number of bytes that were written
		NULL			// No overlap
	);
	PRINT("Write ended (%lu bytes written)\n", bytes_written);

	if (valid_value) {
		printf("\n________________________________________ WRITE OPERATION _________________________________________\n");
		printf("file_path: '%ws'\n", file_path);
		printf("write_offset: '%llu'\n", write_offset);
		printf("write_length: '%lu'\n", write_length);
		printf("bytes_written: '%lu'\n", bytes_written);
		printf("\n");
		print_hex("write_buffer", write_buffer, (size_t)bytes_written);
		printf("\n");
		printf("As string (stops on first '\\0'): '%.*s'\n", (unsigned int)write_length, write_buffer);
		printf("\n__________________________________________________________________________________________________\n");
	} else {
		printf("\nError reading file (%lu)\n", GetLastError());
		error_code = ERROR_READ_FAULT;
	}

	CLEANUP_WRITE:
	if (opened) {
		CloseHandle(handle);
	}
	if (write_buffer != NULL) {
		free(write_buffer);
	}

	return error_code;
}

DWORD getDataToWrite(byte* write_buffer, DWORD write_length) {
	//printf("TO DO: more elaborated than filling with 't'???\n");
	//memset(write_buffer, 't', write_length);

	char input_buffer[MAX_INPUT_LENGTH+1] = { 0 };
	size_t input_length = 0;
	BOOL valid_value = FALSE;

	printf("\nType in the input to be written in the file.\n");
	printf("NOTES:\n");
	printf(" - Input is limited to 500 characters\n");
	printf(" - Ending '\\n' and '\\0' characters are not included\n");
	printf(" - If the written sequence is too short, it is extended repeating itself periodically to match the write length\n");
	printf(" - If the written sequence is too long, it is cut out to fit the write length\n");
	do {
		printf("--> ");
		if (fgets(input_buffer, MAX_INPUT_LENGTH+1, stdin)) {				// fgets() ensures that string ends with '\0'
			input_buffer[strcspn(input_buffer, L"\n")] = '\0';						// Remove trailing '\n'

			// Check positive length
			input_length = strlen(input_buffer);
			if (input_length > 0) {
				valid_value = TRUE;

				// Copy the input (cutting or extending it) to the write buffer
				size_t write_idx;
				size_t input_idx;
				for (write_idx = input_idx = 0; write_idx < write_length; write_idx++, input_idx++, input_idx %= input_length) {
					write_buffer[write_idx] = input_buffer[input_idx];
				}
			}
		}
	} while (!valid_value);

	//print_hex("write_buffer", write_buffer, (size_t)write_length);
	//printf("As string (stops on first '\\0'): '%.*s'\n", (unsigned int)write_length, write_buffer);

	return ERROR_SUCCESS;
}


int main(){

	printf("\n\n\n");
	printf("       _____                ___          __   _ _    _______        _    \n");
	printf("      |  __ \\              | \\ \\        / /  (_) |  |__   __|      | |   \n");
	printf("      | |__) |___  __ _  __| |\\ \\  /\\  / / __ _| |_ ___| | ___  ___| |_  \n");
	printf("      |  _  // _ \\/ _` |/ _` | \\ \\/  \\/ / '__| | __/ _ \\ |/ _ \\/ __| __| \n");
	printf("      | | \\ \\  __/ (_| | (_| |  \\  /\\  /| |  | | ||  __/ |  __/\\__ \\ |_  \n");
	printf("      |_|  \\_\\___|\\__,_|\\__,_|   \\/  \\/ |_|  |_|\\__\\___|_|\\___||___/\\__| \n");
	printf("\n\n\n");

	/*
	It looks like this (but needs to duplicate backslashes to scape them)
	  _____                ___          __   _ _    _______        _
	 |  __ \              | \ \        / /  (_) |  |__   __|      | |
	 | |__) |___  __ _  __| |\ \  /\  / / __ _| |_ ___| | ___  ___| |_
	 |  _  // _ \/ _` |/ _` | \ \/  \/ / '__| | __/ _ \ |/ _ \/ __| __|
	 | | \ \  __/ (_| | (_| |  \  /\  /| |  | | ||  __/ |  __/\__ \ |_
	 |_|  \_\___|\__,_|\__,_|   \/  \/ |_|  |_|\__\___|_|\___||___/\__|

	*/

	char line[MAX_INPUT_LENGTH] = { 0 };
	int choice = 0;
	BOOL quit_menu = FALSE;

	printf("This is a test for checking reading and writting through the securemirror application\n");
	do {
		printf("\n");
		printf("Select an option:\n");
		printf("  1) Read\n");
		printf("  2) Write\n");
		printf("  0) Exit\n");
		printf("--> ");
		if (fgets(line, MAX_INPUT_LENGTH, stdin)) {
			if (1 == sscanf_s(line, "%d", &choice)) {
				switch (choice) {
					case 1:
						readMenu();
						break;
					case 2:
						writeMenu();
						break;
					case 0:
						printf("Exitting...\n");
						quit_menu = TRUE;
						break;
					default:
						printf("Invalid option, try again.\n");
						break;
				}
			}
		}
	} while (!quit_menu);

	return 0;
}
