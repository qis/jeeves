// Copyright (c) 2012, Stephen Fewer of Harmony Security (www.harmonysecurity.com)
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted 
// provided that the following conditions are met:
// 
//     * Redistributions of source code must retain the above copyright notice, this list of 
// conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above copyright notice, this list of 
// conditions and the following disclaimer in the documentation and/or other materials provided 
// with the distribution.
// 
//     * Neither the name of Harmony Security nor the names of its contributors may be used to
// endorse or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.

#include "module.h"
#include <csgo/xorstr.h>
#include <stdio.h>

DWORD Rva2Offset(DWORD dwRva, UINT_PTR uiBaseAddress)
{
  WORD wIndex = 0;
  PIMAGE_SECTION_HEADER pSectionHeader = nullptr;
  PIMAGE_NT_HEADERS pNtHeaders = nullptr;

  pNtHeaders = (PIMAGE_NT_HEADERS)(uiBaseAddress + ((PIMAGE_DOS_HEADER)uiBaseAddress)->e_lfanew);

  pSectionHeader = (PIMAGE_SECTION_HEADER)((UINT_PTR)(&pNtHeaders->OptionalHeader) + pNtHeaders->FileHeader.SizeOfOptionalHeader);

  if (dwRva < pSectionHeader[0].PointerToRawData)
    return dwRva;

  for (wIndex = 0; wIndex < pNtHeaders->FileHeader.NumberOfSections; wIndex++) {
    if (dwRva >= pSectionHeader[wIndex].VirtualAddress && dwRva < (pSectionHeader[wIndex].VirtualAddress + pSectionHeader[wIndex].SizeOfRawData))
      return (dwRva - pSectionHeader[wIndex].VirtualAddress + pSectionHeader[wIndex].PointerToRawData);
  }

  return 0;
}

DWORD GetReflectiveLoaderOffset(VOID * lpReflectiveDllBuffer)
{
  UINT_PTR uiBaseAddress = 0;
  UINT_PTR uiExportDir = 0;
  UINT_PTR uiNameArray = 0;
  UINT_PTR uiAddressArray = 0;
  UINT_PTR uiNameOrdinals = 0;
  DWORD dwCounter = 0;
#ifdef WIN_X64
  DWORD dwCompiledArch = 2;
#else
  // This will catch Win32 and WinRT.
  DWORD dwCompiledArch = 1;
#endif

  uiBaseAddress = (UINT_PTR)lpReflectiveDllBuffer;

  // get the File Offset of the modules NT Header
  uiExportDir = uiBaseAddress + ((PIMAGE_DOS_HEADER)uiBaseAddress)->e_lfanew;

  // currenlty we can only process a PE file which is the same type as the one this fuction has  
  // been compiled as, due to various offset in the PE structures being defined at compile time.
  if (((PIMAGE_NT_HEADERS)uiExportDir)->OptionalHeader.Magic == 0x010B) // PE32
  {
    if (dwCompiledArch != 1)
      return 0;
  } else if (((PIMAGE_NT_HEADERS)uiExportDir)->OptionalHeader.Magic == 0x020B) // PE64
  {
    if (dwCompiledArch != 2)
      return 0;
  } else {
    return 0;
  }

  // uiNameArray = the address of the modules export directory entry
  uiNameArray = (UINT_PTR)&((PIMAGE_NT_HEADERS)uiExportDir)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

  // get the File Offset of the export directory
  uiExportDir = uiBaseAddress + Rva2Offset(((PIMAGE_DATA_DIRECTORY)uiNameArray)->VirtualAddress, uiBaseAddress);

  // get the File Offset for the array of name pointers
  uiNameArray = uiBaseAddress + Rva2Offset(((PIMAGE_EXPORT_DIRECTORY)uiExportDir)->AddressOfNames, uiBaseAddress);

  // get the File Offset for the array of addresses
  uiAddressArray = uiBaseAddress + Rva2Offset(((PIMAGE_EXPORT_DIRECTORY)uiExportDir)->AddressOfFunctions, uiBaseAddress);

  // get the File Offset for the array of name ordinals
  uiNameOrdinals = uiBaseAddress + Rva2Offset(((PIMAGE_EXPORT_DIRECTORY)uiExportDir)->AddressOfNameOrdinals, uiBaseAddress);

  // get a counter for the number of exported functions...
  dwCounter = ((PIMAGE_EXPORT_DIRECTORY)uiExportDir)->NumberOfNames;

  // loop through all the exported functions to find the ReflectiveLoader
  while (dwCounter--) {
    char * cpExportedFunctionName = (char *)(uiBaseAddress + Rva2Offset(DEREF_32(uiNameArray), uiBaseAddress));

    if (strstr(cpExportedFunctionName, xorstr("ReflectiveLoader"))) {
      // get the File Offset for the array of addresses
      uiAddressArray = uiBaseAddress + Rva2Offset(((PIMAGE_EXPORT_DIRECTORY)uiExportDir)->AddressOfFunctions, uiBaseAddress);

      // use the functions name ordinal as an index into the array of name pointers
      uiAddressArray += (DEREF_16(uiNameOrdinals) * sizeof(DWORD));

      // return the File Offset to the ReflectiveLoader() functions code...
      return Rva2Offset(DEREF_32(uiAddressArray), uiBaseAddress);
    }
    // get the next exported function name
    uiNameArray += sizeof(DWORD);

    // get the next exported function name ordinal
    uiNameOrdinals += sizeof(WORD);
  }

  return 0;
}

// Loads a DLL image from memory via its exported ReflectiveLoader function
HMODULE WINAPI LoadLibraryR(LPVOID lpBuffer, DWORD dwLength)
{
  HMODULE hResult = nullptr;
  DWORD dwReflectiveLoaderOffset = 0;
  DWORD dwOldProtect1 = 0;
  DWORD dwOldProtect2 = 0;
  REFLECTIVELOADER pReflectiveLoader = nullptr;
  DLLMAIN pDllMain = nullptr;

  if (lpBuffer == nullptr || dwLength == 0)
    return nullptr;

  __try {
    // check if the library has a ReflectiveLoader...
    dwReflectiveLoaderOffset = GetReflectiveLoaderOffset(lpBuffer);
    if (dwReflectiveLoaderOffset) {
      pReflectiveLoader = (REFLECTIVELOADER)((UINT_PTR)lpBuffer + dwReflectiveLoaderOffset);

      // we must VirtualProtect the buffer to RWX so we can execute the ReflectiveLoader...
      // this assumes lpBuffer is the base address of the region of pages and dwLength the size of the region
      if (VirtualProtect(lpBuffer, dwLength, PAGE_EXECUTE_READWRITE, &dwOldProtect1)) {
        // call the librarys ReflectiveLoader...
        pDllMain = (DLLMAIN)pReflectiveLoader();
        if (pDllMain) {
          // call the loaded librarys DllMain to get its HMODULE
          if (!pDllMain(nullptr, DLL_QUERY_HMODULE, &hResult))
            hResult = nullptr;
        }
        // revert to the previous protection flags...
        VirtualProtect(lpBuffer, dwLength, dwOldProtect1, &dwOldProtect2);
      }
    }
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    hResult = nullptr;
  }

  return hResult;
}

// Loads a PE image from memory into the address space of a host process via the image's exported ReflectiveLoader function
// Note: You must compile whatever you are injecting with REFLECTIVEDLLINJECTION_VIA_LOADREMOTELIBRARYR 
//       defined in order to use the correct RDI prototypes.
// Note: The hProcess handle must have these access rights: PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
//       PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ
// Note: If you are passing in an lpParameter value, if it is a pointer, remember it is for a different address space.
// Note: This function currently cant inject accross architectures, but only to architectures which are the 
//       same as the arch this function is compiled as, e.g. x86->x86 and x64->x64 but not x64->x86 or x86->x64.
HANDLE WINAPI LoadRemoteLibraryR(HANDLE hProcess, LPVOID lpBuffer, DWORD dwLength, LPVOID lpParameter)
{
  BOOL bSuccess = FALSE;
  LPVOID lpRemoteLibraryBuffer = nullptr;
  LPTHREAD_START_ROUTINE lpReflectiveLoader = nullptr;
  HANDLE hThread = nullptr;
  DWORD dwReflectiveLoaderOffset = 0;
  DWORD dwThreadId = 0;

  __try {
    do {
      if (!hProcess || !lpBuffer || !dwLength)
        break;

      // check if the library has a ReflectiveLoader...
      dwReflectiveLoaderOffset = GetReflectiveLoaderOffset(lpBuffer);
      if (!dwReflectiveLoaderOffset)
        break;

      // alloc memory (RWX) in the host process for the image...
      lpRemoteLibraryBuffer = VirtualAllocEx(hProcess, nullptr, dwLength, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
      if (!lpRemoteLibraryBuffer)
        break;

      // write the image into the host process...
      if (!WriteProcessMemory(hProcess, lpRemoteLibraryBuffer, lpBuffer, dwLength, nullptr))
        break;

      // add the offset to ReflectiveLoader() to the remote library address...
      lpReflectiveLoader = (LPTHREAD_START_ROUTINE)((ULONG_PTR)lpRemoteLibraryBuffer + dwReflectiveLoaderOffset);

      // create a remote thread in the host process to call the ReflectiveLoader!
      hThread = CreateRemoteThread(hProcess, nullptr, 1024 * 1024, lpReflectiveLoader, lpParameter, (DWORD)nullptr, &dwThreadId);

    } while (0);

  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    hThread = nullptr;
  }

  return hThread;
}

// We implement a minimal GetProcAddress to avoid using the native kernel32!GetProcAddress which
// wont be able to resolve exported addresses in reflectivly loaded librarys.
FARPROC WINAPI GetProcAddressR(HANDLE hModule, LPCSTR lpProcName)
{
  UINT_PTR uiLibraryAddress = 0;
  FARPROC fpResult = nullptr;

  if (hModule == nullptr)
    return nullptr;

  // a module handle is really its base address
  uiLibraryAddress = (UINT_PTR)hModule;

  __try {
    UINT_PTR uiAddressArray = 0;
    UINT_PTR uiNameArray = 0;
    UINT_PTR uiNameOrdinals = 0;
    PIMAGE_NT_HEADERS pNtHeaders = nullptr;
    PIMAGE_DATA_DIRECTORY pDataDirectory = nullptr;
    PIMAGE_EXPORT_DIRECTORY pExportDirectory = nullptr;

    // get the VA of the modules NT Header
    pNtHeaders = (PIMAGE_NT_HEADERS)(uiLibraryAddress + ((PIMAGE_DOS_HEADER)uiLibraryAddress)->e_lfanew);

    pDataDirectory = (PIMAGE_DATA_DIRECTORY)&pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    // get the VA of the export directory
    pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(uiLibraryAddress + pDataDirectory->VirtualAddress);

    // get the VA for the array of addresses
    uiAddressArray = (uiLibraryAddress + pExportDirectory->AddressOfFunctions);

    // get the VA for the array of name pointers
    uiNameArray = (uiLibraryAddress + pExportDirectory->AddressOfNames);

    // get the VA for the array of name ordinals
    uiNameOrdinals = (uiLibraryAddress + pExportDirectory->AddressOfNameOrdinals);

    // test if we are importing by name or by ordinal...
    if (((DWORD)lpProcName & 0xFFFF0000) == 0x00000000) {
      // import by ordinal...

      // use the import ordinal (- export ordinal base) as an index into the array of addresses
      uiAddressArray += ((IMAGE_ORDINAL((DWORD)lpProcName) - pExportDirectory->Base) * sizeof(DWORD));

      // resolve the address for this imported function
      fpResult = (FARPROC)(uiLibraryAddress + DEREF_32(uiAddressArray));
    } else {
      // import by name...
      DWORD dwCounter = pExportDirectory->NumberOfNames;
      while (dwCounter--) {
        char * cpExportedFunctionName = (char *)(uiLibraryAddress + DEREF_32(uiNameArray));

        // test if we have a match...
        if (strcmp(cpExportedFunctionName, lpProcName) == 0) {
          // use the functions name ordinal as an index into the array of name pointers
          uiAddressArray += (DEREF_16(uiNameOrdinals) * sizeof(DWORD));

          // calculate the virtual address for the function
          fpResult = (FARPROC)(uiLibraryAddress + DEREF_32(uiAddressArray));

          // finish...
          break;
        }

        // get the next exported function name
        uiNameArray += sizeof(DWORD);

        // get the next exported function name ordinal
        uiNameOrdinals += sizeof(WORD);
      }
    }
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    fpResult = nullptr;
  }

  return fpResult;
}