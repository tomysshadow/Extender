#include "shared.h"
#include "Extender.h"
#include <windows.h>

EXTENDED_CODE_ADDRESS extendedCode_returnAddress = 0x00000000;

__declspec(naked) void exampleExtendedCode() {
	__asm {
		// set this to your Extension Code
		
		// epilogue
		jmp [extendedCode_returnAddress]
	}
}

bool extender() {
	HMODULE moduleHandle = GetModuleHandle(NULL);

	if (!moduleHandle) {
		showLastError("Failed to Get Module Handle");
		return false;
	}

	// add your Extended Code Addresses
	// (any addresses you want to use within your Extended Code as a variable)
	extendedCode_returnAddress = makeExtendedCodeAddress(moduleHandle, 0x00001000);

	// test it
	const VIRTUAL_SIZE EXAMPLE_TESTED_CODE_SIZE = 4;
	CODE1 exampleTestedCode[EXAMPLE_TESTED_CODE_SIZE] = {0x00, 0x00, 0x00, 0x00};

	if (!testCode(moduleHandle, 0x00001000, EXAMPLE_TESTED_CODE_SIZE, exampleTestedCode)) {
		showLastError("Failed to Test Code");
		return false;
	}

	// extend it
	if (!extendCode(moduleHandle, 0x00001000, exampleExtendedCode)) {
		showLastError("Failed to Extend Code");
		return false;
	}
	return true;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		if (!DisableThreadLibraryCalls(instance)) {
			showLastError("Failed to Disable Thread Library Calls");
			return FALSE;
		}

		if (!extender()) {
			terminateCurrentProcess();
			return FALSE;
		}
	}
	return TRUE;
}