#pragma once
#ifndef __RBTIMER_H__
#define __RBTIMER_H__

#ifdef _WIN32
#include "windows.h"
#include <ctime>
#else
#include <sys/time.h>
#endif


//支持镶嵌计时
namespace rb
{
	class rb_timer_t
	{
	public:
		static const int max_stack_n = 32;
		void init();
		double time() const;
		void begin();
		double end();
	private:
		int _cur_start_count_index;
	#ifdef _WIN32
		
		LARGE_INTEGER _end_count;
		LARGE_INTEGER _freqency;
		LARGE_INTEGER _start_count_stack[max_stack_n];


    #else
    timeval _end_count;
	timerval __start_count_stack[max_stack_n];
    #endif

	
	};
}
#endif
