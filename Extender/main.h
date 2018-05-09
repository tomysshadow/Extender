#ifndef _MAIN_H_
#define _MAIN_H_

#include <windows.h>

const LPCTSTR  LP_MODULE_NAME = NULL;                 // set this to the module's name (leave NULL for calling process)
const DWORD   WRITTEN_ADDRESS = 0x00000000;           // set this to the address that will be written, relative to the beginning of the module's .text section
const DWORD    TESTED_ADDRESS = 0x00000000;           // set this to the address that will be  tested, relative to the beginning of the module's .text section
                                                      // don't set this to the written address or the test won't succeed after writing to it
const DWORD WRITTEN_CODE_RETURN_ADDRESS = 0x00001000; // set this to the return address of the code to be written, relative to the beginning of the module
													  // it is relative to the beginning of the module rather than it's .text section,
													  // because whereas the written code may only be written to the module's .text section,
													  // the written code itself may need access to other sections
HANDLE moduleHandle = INVALID_HANDLE_VALUE;           // leave alone, we will get the Module Handle when setting up the Extender
HANDLE moduleHandleWrittenCodeReturnAddress = INVALID_HANDLE_VALUE;
const size_t SIZEOF_TESTED = 4;
unsigned char tested[SIZEOF_TESTED] = {0x00, 0x00, 0x00, 0x00};    // set this to the buffer to be tested within the .text section
bool testedSet = false;
// set this to the code to be written within the .text section
__declspec(naked) void written() {
	__asm {
		// set this to the original code
		
		// set this to your extended code

		// leave alone, epilogue code
		jmp [moduleHandleWrittenCodeReturnAddress]
	}
}

#endif