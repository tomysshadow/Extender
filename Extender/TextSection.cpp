#include <windows.h>
#include "TextSection.h"

bool TextSection::unprotect() {
	if (!VirtualProtect(lpAddress, dwSize, PAGE_EXECUTE_READWRITE, &lpflOldProtect) || !lpAddress || !dwSize) {
		return false;
	}
	lpflOldProtectSet = true;
	return true;
}

bool TextSection::protect() {
	if (!lpflOldProtectSet || !VirtualProtect(lpAddress, dwSize, lpflOldProtect, &lpflOldProtect)) {
		return false;
	}
	return true;
}

bool TextSection::flush() {
	if (!flushed) {
		flushed = true;
		if (!FlushInstructionCache(currentProcess, lpAddress, dwSize)) {
			flushed = false;
		}
	}
	return flushed;
}

bool TextSection::test(void* tested, size_t sizeofTested, DWORD testedAddress) {
	// set Test Position absolutely
	testedAddress = (DWORD)lpAddress + testedAddress;
	if (testedAddress + sizeofTested > (DWORD)lpAddress + dwSize) {
		return false;
	}

	// get Basic Memory Information
	MEMORY_BASIC_INFORMATION memoryBasicInformation;
	if (VirtualQuery((LPCVOID)testedAddress, &memoryBasicInformation, sizeof(memoryBasicInformation)) != sizeof(memoryBasicInformation)
		|| !memoryBasicInformation.Protect
		|| memoryBasicInformation.Protect & PAGE_NOACCESS
		|| memoryBasicInformation.Protect & PAGE_EXECUTE) {
		return false;
	}
	return !memcmp((const void*)testedAddress, tested, sizeofTested); // false means they are the same
}

bool TextSection::write(DWORD writtenAddress) {
	const BYTE NOP = 0x90;
	const char NOP_LENGTH = 1;

	// set Write Position absolutely
	writtenAddress = (DWORD)lpAddress + writtenAddress;
	if (writtenAddress + NOP_LENGTH > (DWORD)lpAddress + dwSize) {
		return false;
	}

	// write code
	*(BYTE*)writtenAddress = NOP;
	flushed = false;
	return true;
}

bool TextSection::write(void* written, DWORD writtenAddress, bool call) {
	const BYTE JMP = 0xE9;
	const BYTE CALL = 0x58;
	const char LENGTH = 5;

	// set Write Position absolutely
	writtenAddress = (DWORD)lpAddress + writtenAddress;
	if (writtenAddress + LENGTH > (DWORD)lpAddress + dwSize) {
		return false;
	}
	// write code
	// for the sake of simplicity, we do not account for the memory instruction(s) we're replacing being a different length
	// that is to be dealt with on a case by case basis during implementation
	if (!call) {
		*(BYTE*)writtenAddress = JMP;
	} else {
		*(BYTE*)writtenAddress = CALL;
	}
	*(DWORD*)((BYTE*)writtenAddress + 1) = (DWORD)written - writtenAddress - LENGTH;
	flushed = false;
	return true;
}

TextSection::TextSection(LPVOID lpAddress, SIZE_T dwSize, HANDLE currentProcess):
	lpAddress(lpAddress),
	dwSize(dwSize),
	currentProcess(currentProcess) {
}

TextSection::~TextSection() {
	flush();
	protect();
	lpAddress = NULL;
}