#include <windows.h>
#include "main.h"

EXTENDED_CODE_ADDRESS extendedCode_returnAddress = 0x00000000;

__declspec(naked) void extendedCode() {
	__asm {
		// set this to your Extension Code
		
		// epilogue
		jmp [extendedCode_returnAddress]
	}
}

bool extender() {
	// set this to your Error Caption
	LPCTSTR errorCaption = "Extender Error";

	// get Module Handle
	HMODULE moduleHandle = GetModuleHandle(NULL);

	if (!moduleHandle) {
		MessageBox(NULL, "Failed to get Module Handle", errorCaption, MB_OK | MB_ICONERROR);
		return false;
	}

	// add your Extension Code Addresses
	// (any addresses you want to use within your Extension Code as a variable)
	extendedCode_returnAddress = createExtendedCodeAddress(moduleHandle, 0x00001000);

	// test it
	const size_t EXAMPLE_TEST_CODE_SIZE = 4;
	unsigned char exampleTestCode[EXAMPLE_TEST_CODE_SIZE] = {0x00, 0x00, 0x00, 0x00};

	if (!testCode(errorCaption, moduleHandle, 0x00001000, EXAMPLE_TEST_CODE_SIZE, exampleTestCode)) {
		MessageBox(NULL, "Failed to Test Code", errorCaption, MB_OK | MB_ICONERROR);
		return false;
	}

	// extend it
	if (!extendCode(errorCaption, moduleHandle, 0x00001000, extendedCode)) {
		MessageBox(NULL, "Failed to Extend Code", errorCaption, MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}