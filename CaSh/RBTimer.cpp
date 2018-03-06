#include "RBTimer.h"

namespace rb
{
	void rb_timer_t::init()
	{
	#ifdef _WIN32
		QueryPerformanceFrequency(&_freqency);
		_cur_start_count_index = 0;
		#else

		#endif
	}

	double rb_timer_t::time() const
	{
	#ifdef _WIN32
		LARGE_INTEGER atime;
		QueryPerformanceCounter(&atime);
		return static_cast<double>(atime.QuadPart/_freqency.QuadPart);
		#else
		gettimeofday(&_start_count,0);
		return (_start_count.tv_sec*1000000 + _start_count.tv_usec )/ 1000;
		#endif // WIN32
	}

	void rb_timer_t::begin()
	{
	#ifdef _WIN32
		LARGE_INTEGER _start_count;
		QueryPerformanceCounter(&_start_count);
		
		#else
		timeval _start_count;
		gettimeofday(&_start_count,0);
		#endif // WIN32

		_start_count_stack[_cur_start_count_index++] = _start_count;
	}

	double rb_timer_t::end()
	{
	#ifdef _WIN32
		LARGE_INTEGER _start_count;
		QueryPerformanceCounter(&_end_count);

		#else
		timeval _start_count;
		gettimeofday(&_end_count,0);
		#endif // WIN32

		_start_count = _start_count_stack[--_cur_start_count_index];
#ifdef _WIN32
		double elapsed = (static_cast<double>(_end_count.QuadPart - _start_count.QuadPart) / static_cast<double>(_freqency.QuadPart) * 1000);
		return elapsed;
#else
		//double elapsed = static_cast<double>((_end_count.tv_usec - _start_count.tv_usec))/static_cast<double>(1000);
		int us = ((_end_count.tv_usec - _start_count.tv_usec));
		int s2us = (_end_count.tv_sec - _start_count.tv_sec) * 1000000;
		double elapsed = (us + s2us) / 1000.f;
		return elapsed;
#endif // _WIN32
	}


}
