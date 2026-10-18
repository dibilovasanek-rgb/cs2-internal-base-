#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <fstream>
#include "stack_trace.h"
#include "../dependencies/config/Include/config.h"
#include "../core/hooks/hooks.h"
#pragma comment(lib, "dbghelp.lib")

void c_stack_trace::write_mini_dump(EXCEPTION_POINTERS* pExceptionInfo) {
    std::string crashDir = "C:\\crash_AIRFLOW\\";
    CreateFolder(crashDir); 

    std::string dumpPath = crashDir + "crash_dump.dmp";
    HANDLE hFile = CreateFileA(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION minidumpInfo;
        minidumpInfo.ThreadId = GetCurrentThreadId();
        minidumpInfo.ExceptionPointers = pExceptionInfo;
        minidumpInfo.ClientPointers = FALSE;

        BOOL result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, &minidumpInfo, nullptr, nullptr);
        CloseHandle(hFile);

        if (!result) {
            std::cerr << "[!] Failed to write dump to file " << dumpPath << "\n";
        }
        else {
            std::cerr << "[!] Crash dump saved to " << dumpPath << "\n";
        }
    }
    else {
        std::cerr << "[!] Failed to create dump file at " << dumpPath << "\n";
    }
}

void c_stack_trace::log_stack_trace() {
    std::string crashDir = "C:\\crash_AIRFLOW\\";
    CreateFolder(crashDir);

    std::string logPath = crashDir + "stack_trace.log";
    std::ofstream file = OpenFile(logPath, std::ios::app);

    if (file.is_open()) {
        void* stack[max_trace];
        USHORT frames = CaptureStackBackTrace(0, max_trace, stack, nullptr);

        file << "=== STACK TRACE ===\n";
        for (USHORT i = 0; i < frames; i++) {
            file << "trace stack " << i << ": " << stack[i] << "\n";
            file << "trace dll " << i << ": " << trace_[i] << "\n";
            file << "last hook: " << g_hook->last_hook << "\n";
        }
        file << "===================\n";
        CloseFile(file);

        std::cerr << "[!] Stack trace saved to " << logPath << "\n";
    }
    else {
        std::cerr << "[!] Failed to open stack trace log file at " << logPath << "\n";
    }
}

static LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo) {
    extern std::unique_ptr<c_stack_trace> g_stack_trace;

    std::cerr << "[!] Crash detected! Logging stack trace & writing dump...\n";

    g_stack_trace->log_stack_trace();
    g_stack_trace->write_mini_dump(pExceptionInfo);

    return EXCEPTION_EXECUTE_HANDLER;
}

void c_stack_trace::setup_crash_handler() {
    SetUnhandledExceptionFilter(ExceptionHandler);
    AddVectoredExceptionHandler(false, ExceptionHandler);
}