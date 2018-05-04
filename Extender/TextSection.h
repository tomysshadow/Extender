#ifndef _TEXT_SECTION_H_
#define _TEXT_SECTION_H_

#include <windows.h>

class TextSection {
	private:
	LPVOID lpAddress = NULL;
	SIZE_T dwSize = 0;
	HANDLE currentProcess = INVALID_HANDLE_VALUE;
	DWORD lpflOldProtect = NULL;
	bool lpflOldProtectSet = false;
	bool flushed = false;

	public:
	bool unprotect();
	bool protect();
	bool flush();
	bool test(void* tested, DWORD testedAddress);
	bool write(DWORD writtenAddress);
	bool write(void* written, DWORD writtenAddress, bool call = false);
	TextSection(LPVOID lpAddress, SIZE_T dwSize, HANDLE currentProcess);
	~TextSection();
};

#endif