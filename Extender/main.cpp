#include <windows.h>
#include <Dbghelp.h>
#include "main.h"
#include "TextSection.h"

bool getTextSectionAddressAndSize(LPVOID &textSectionLpAddress, SIZE_T &textSectionDwSize, const char* messageBoxLpCaption) {
	if (moduleHandle == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, "Failed to get Module Handle", messageBoxLpCaption, MB_OK);
		return false;
	}
	PIMAGE_NT_HEADERS imageNtHeader = ImageNtHeader(moduleHandle);
	if (!imageNtHeader) {
		MessageBox(NULL, "Failed to get Image NT Header", messageBoxLpCaption, MB_OK);
		imageNtHeader = 0;
		return false;
	}
	PIMAGE_SECTION_HEADER imageSectionHeader = (PIMAGE_SECTION_HEADER)(imageNtHeader + 1);
	const char IMAGE_SECTION_HEADER_NAME_TEXT[IMAGE_SIZEOF_SHORT_NAME] = ".text\0\0";
	for (WORD i = 0;i<imageNtHeader->FileHeader.NumberOfSections;i++) {
		if (moduleHandle == INVALID_HANDLE_VALUE) {
			MessageBox(NULL, "Failed to get Module Handle", messageBoxLpCaption, MB_OK);
			imageNtHeader = 0;
			imageSectionHeader = 0;
			return false;
		}
		// false means they are the same
		if (!memcmp((const void*)imageSectionHeader->Name, IMAGE_SECTION_HEADER_NAME_TEXT, IMAGE_SIZEOF_SHORT_NAME)) {
			textSectionLpAddress = (BYTE*)moduleHandle + imageSectionHeader->VirtualAddress;
			textSectionDwSize = imageSectionHeader->Misc.VirtualSize;
			imageNtHeader = 0;
			imageSectionHeader = 0;
			return true;
		}
		imageSectionHeader++;
	}
	MessageBox(NULL, "Failed to get Text Section", messageBoxLpCaption, MB_OK);
	imageNtHeader = 0;
	imageSectionHeader = 0;
	return false;
}

bool setupExtender(HANDLE currentProcess) {
	const char* messageBoxLpCaption = "Extender Error";

	// get Module Handle
	moduleHandle = GetModuleHandle(LP_MODULE_NAME);
	LPVOID textSectionLpAddress = NULL;
	SIZE_T textSectionDwSize = 0;

	// get Text Section lpAddress and dwSize
	if (!getTextSectionAddressAndSize(textSectionLpAddress, textSectionDwSize, messageBoxLpCaption)) {
		MessageBox(NULL, "Failed to get Text Section Address and Size", messageBoxLpCaption, MB_OK);
		CloseHandle(moduleHandle);
		return false;
	}

	// create Text Section
	TextSection* textSection = new TextSection(textSectionLpAddress, textSectionDwSize, currentProcess);
	if (!textSection) {
		MessageBox(NULL, "Failed to create Text Section", messageBoxLpCaption, MB_OK);
		CloseHandle(moduleHandle);
		return false;
	}
	if (!textSection->unprotect()) {
		MessageBox(NULL, "Failed to unprotect Text Section", messageBoxLpCaption, MB_OK);
		CloseHandle(moduleHandle);
		delete textSection;
		textSection = 0;
		return false;
	}

	// test it
	if (!textSection->test(tested, TESTED_ADDRESS)) {
		MessageBox(NULL, "Text Section Test failed", messageBoxLpCaption, MB_OK);
		CloseHandle(moduleHandle);
		delete textSection;
		textSection = 0;
		return false;
	}

	// write it
	if (!textSection->write(written, WRITTEN_ADDRESS)) {
		MessageBox(NULL, "Text Section Write failed", messageBoxLpCaption, MB_OK);
		CloseHandle(moduleHandle);
		delete textSection;
		textSection = 0;
		return false;
	}

	// flush it
	if (!textSection->flush()) {
		MessageBox(NULL, "Text Section Flush failed", messageBoxLpCaption, MB_OK);
		CloseHandle(moduleHandle);
		delete textSection;
		textSection = 0;
		return false;
	}

	// cleanup
	delete textSection;
	textSection = 0;
	return true;
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		{
			HANDLE currentProcess = GetCurrentProcess();
			if (!setupExtender(currentProcess)) {
				TerminateProcess(currentProcess, 0);
				return FALSE;
			}
		}
		break;
		case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}