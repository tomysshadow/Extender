#ifndef _MAIN_H_
#define _MAIN_H_

#include <windows.h>
#include <Dbghelp.h>

inline size_t stringSize(const char* string) {
	return strlen(string) + 1;
}

inline bool stringsEqual(const char* leftHandSide, const char* rightHandSide) {
	return !strcmp(leftHandSide, rightHandSide);
}

inline bool memoryEqual(const void* buffer, const void* buffer2, size_t bufferSize) {
	return !memcmp(buffer, buffer2, bufferSize);
}

bool shiftMemory(size_t bufferSize, const void* buffer, size_t sourceSize, const void* source, unsigned int shift, bool direction) {
	if (source < buffer || (char*)source + sourceSize >(char*)buffer + bufferSize) {
		return false;
	}

	size_t destinationSize = (char*)buffer + bufferSize - source;
	char* destination = (char*)source;

	if (!direction) {
		destination -= shift;
	} else {
		destination += shift;
	}

	if (destination < buffer || destination + sourceSize >(char*)buffer + bufferSize) {
		return false;
	}
	return !memmove_s(destination, destinationSize, source, sourceSize);
}

bool extender();

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);

		{
			if (!extender()) {
				TerminateProcess(GetCurrentProcess(), -1);
				return FALSE;
			}
		}
	}
	return TRUE;
}

inline DWORD createExtendedCodeAddress(HMODULE moduleHandle, DWORD address) {
	return (DWORD)moduleHandle + address;
}

bool getSectionAddressAndSize(LPCTSTR errorLpCaption, HMODULE moduleHandle, DWORD virtualAddress, DWORD virtualSize) {
	if (!moduleHandle) {
		MessageBox(NULL, "Failed to Get Module Handle", errorLpCaption, MB_OK | MB_ICONERROR);
		return false;
	}

	PIMAGE_NT_HEADERS imageNtHeader = ImageNtHeader(moduleHandle);

	if (!imageNtHeader) {
		MessageBox(NULL, "Failed to Get Image NT Header", errorLpCaption, MB_OK | MB_ICONERROR);
		return false;
	}

	PIMAGE_SECTION_HEADER imageSectionHeader = (PIMAGE_SECTION_HEADER)(imageNtHeader + 1);

	if (!imageSectionHeader) {
		imageNtHeader = NULL;
		MessageBox(NULL, "Failed to Get Image Section Header", errorLpCaption, MB_OK | MB_ICONERROR);
		return false;
	}

	for (WORD i = 0;i < imageNtHeader->FileHeader.NumberOfSections;i++) {
		if (virtualAddress >= (DWORD)moduleHandle + imageSectionHeader->VirtualAddress && virtualAddress + virtualSize <= (DWORD)moduleHandle + imageSectionHeader->VirtualAddress + imageSectionHeader->Misc.VirtualSize) {
			imageNtHeader = NULL;
			imageSectionHeader = NULL;
			return true;
		}

		imageSectionHeader++;
	}

	imageNtHeader = NULL;
	imageSectionHeader = NULL;
	MessageBox(NULL, "Failed to Get Section Address And Size", errorLpCaption, MB_OK | MB_ICONERROR);
	return false;
}

bool unprotectCode(LPCTSTR errorLpCaption, HMODULE moduleHandle, DWORD virtualAddress, DWORD virtualSize, DWORD &lpflOldProtect) {
	if (!getSectionAddressAndSize(errorLpCaption, moduleHandle, virtualAddress, virtualSize)) {
		return false;
	}

	if (!VirtualProtect((LPVOID)virtualAddress, virtualSize, PAGE_EXECUTE_READWRITE, &lpflOldProtect) || !virtualAddress || !virtualSize) {
		MessageBox(NULL, "Failed to Unprotect Code", errorLpCaption, MB_OK | MB_ICONERROR);
		return false;
	}

	// get Basic Memory Information
	MEMORY_BASIC_INFORMATION memoryBasicInformation;
	if (VirtualQuery((LPCVOID)virtualAddress, &memoryBasicInformation, sizeof(memoryBasicInformation)) != sizeof(memoryBasicInformation)
		|| !memoryBasicInformation.Protect
		|| memoryBasicInformation.Protect & PAGE_NOACCESS
		|| memoryBasicInformation.Protect & PAGE_EXECUTE) {
		return false;
	}
	return true;
}

bool protectCode(LPCTSTR errorLpCaption, HMODULE moduleHandle, DWORD virtualAddress, DWORD virtualSize, DWORD &lpflOldProtect) {
	if (!getSectionAddressAndSize(errorLpCaption, moduleHandle, virtualAddress, virtualSize)) {
		return false;
	}

	if (!lpflOldProtect || !VirtualProtect((LPVOID)virtualAddress, virtualSize, lpflOldProtect, &lpflOldProtect)) {
		MessageBox(NULL, "Failed to Protect Code", errorLpCaption, MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

bool flushCode(LPCTSTR errorLpCaption, HMODULE moduleHandle, DWORD virtualAddress, DWORD virtualSize) {
	if (!getSectionAddressAndSize(errorLpCaption, moduleHandle, virtualAddress, virtualSize)) {
		return false;
	}

	if (!FlushInstructionCache(GetCurrentProcess(), (LPCVOID)virtualAddress, virtualSize)) {
		MessageBox(NULL, "Failed to Flush Code", errorLpCaption, MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}

bool testCode(LPCTSTR errorLpCaption, HMODULE moduleHandle, DWORD relativeVirtualAddress, DWORD virtualSize, unsigned char code[]) {
	if (!moduleHandle) {
		return false;
	}

	DWORD virtualAddress = (DWORD)moduleHandle + relativeVirtualAddress;
	DWORD lpflOldProtect = 0;

	if (!unprotectCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize, lpflOldProtect)) {
		return false;
	}

	bool result = memoryEqual((const void*)virtualAddress, (const void*)code, virtualSize);

	if (!protectCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize, lpflOldProtect)) {
		return false;
	}
	return result;
}

bool extendCode(LPCTSTR errorLpCaption, HMODULE moduleHandle, DWORD relativeVirtualAddress, void* code, bool call = false) {
	if (!moduleHandle) {
		return false;
	}

	DWORD virtualAddress = (DWORD)moduleHandle + relativeVirtualAddress;
	DWORD virtualSize = 5;
	DWORD lpflOldProtect = 0;

	if (!unprotectCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize, lpflOldProtect)) {
		return false;
	}

	if (!call) {
		*(PBYTE)virtualAddress = 0xE9;
	} else {
		*(PBYTE)virtualAddress = 0x58;
	}

	*(PDWORD)((PBYTE)virtualAddress + 1) = (DWORD)code - virtualAddress - virtualSize;

	if (!flushCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize)) {
		return false;
	}

	if (!protectCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize, lpflOldProtect)) {
		return false;
	}
	return true;
}

bool extendCode(LPCTSTR errorLpCaption, HMODULE moduleHandle, DWORD relativeVirtualAddress) {
	if (!moduleHandle) {
		return false;
	}

	DWORD virtualAddress = (DWORD)moduleHandle + relativeVirtualAddress;
	DWORD virtualSize = 1;
	DWORD lpflOldProtect = 0;

	if (!unprotectCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize, lpflOldProtect)) {
		return false;
	}

	*(PBYTE)virtualAddress = 0x90;

	if (!flushCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize)) {
		return false;
	}

	if (!protectCode(errorLpCaption, moduleHandle, virtualAddress, virtualSize, lpflOldProtect)) {
		return false;
	}
	return true;
}

#endif