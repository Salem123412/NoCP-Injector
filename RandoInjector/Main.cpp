#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <string>

void closeProgram(int time, std::string msg) {
	std::cout << "A problem occured..." << std::endl;

	Sleep(time);
}

inline bool exists(const std::string& DLLNAME) {
	struct stat buffer;

	return (stat(DLLNAME.c_str(), &buffer) == 0);
}

DWORD openHandle(char* processName) {
	// Create our Handle
	auto Handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	// Get our ProcessEntry
	PROCESSENTRY32 processEntry;

	// Get our Process Size
	processEntry.dwSize = sizeof(processEntry);

	// While there are multiple Processes, loop through them
	while (Process32Next(Handle, &processEntry)) {
		// Check if our Processes match
		if (!strcmp(processEntry.szExeFile, processName)) {
			// Define our Process ID
			auto processID = processEntry.th32ProcessID;

			// Close our Handle
			CloseHandle(Handle);

			// Return our Process ID
			return processID;
		}
	}

	// If we're here, our process isn't running
	if (!Process32Next(Handle, &processEntry)) {
		// Call our Close Program Function
		closeProgram(1000, "Process isn't running!");
	}
}

int main() {
	// Define our DLL Name
	std::string fileName = "NoCPv2.dll";
	// Define our Process Name
	char* processName = "csgo.exe";

	// Create a buffer for the path to our DLL
	char Dll[MAX_PATH];

	// Fill the buffer with our full path to our DLL
	GetFullPathName(fileName.c_str(), MAX_PATH, Dll, 0);

	// Define our Process as csgo.exe
	auto process = openHandle(processName);

	// Check if our DLL exists
	if (!exists(fileName)) {
		// Print in console that our DLL couldn't be found
		std::cout << fileName << " couldn't be found!" << std::endl;
		
		closeProgram(1000, "File doesn't exist!");
	}

	// Open a new Handle to our Process
	auto hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, process);

	// Allocate sizeof(Dll) memory to our process
	auto allocatedMemory = VirtualAllocEx(hProcess, NULL, sizeof(Dll), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// Write our Dll to our process
	WriteProcessMemory(hProcess, allocatedMemory, Dll, sizeof(Dll), NULL);

	// Inject the DLL
	CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, allocatedMemory, 0, 0);

	// Close the Handle we opened
	if (CloseHandle(hProcess)) {
		closeProgram(1000, "Successfully Injected!");
	}
}

