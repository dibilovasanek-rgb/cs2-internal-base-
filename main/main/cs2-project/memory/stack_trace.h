#pragma once
#include <string>

class c_stack_trace
{
	enum
	{
		max_trace = 32
	};
public:
	const char* trace_[max_trace];
	int sequence_number_ = 0;
public:
	void add_trace(const char* trace_name) { trace_[sequence_number_ % max_trace] = trace_name; ++sequence_number_; }
	void write_mini_dump(EXCEPTION_POINTERS* pExceptionInfo);
	void log_stack_trace();
	void setup_crash_handler();
};
inline auto g_stack_trace = std::make_unique<c_stack_trace>();
