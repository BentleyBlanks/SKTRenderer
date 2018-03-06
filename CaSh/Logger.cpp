#include "Logger.h"
#include <time.h>
#include <thread>


WIPLogger::WIPLogger()
{
	m_initialized = false;
}
WIPLogger* WIPLogger::get_instance()
{
	static WIPLogger* _instance = nullptr;
	if(!_instance)
		_instance = new WIPLogger();
	return _instance;
}

void WIPLogger::startup(const char * log_path)
{
	m_initialized = true;
	_current_lines = 0;
	_default_path = log_path;
	new_log();
}

void WIPLogger::shutdown()
{
	m_initialized = false;
	outfile<<_buf.data();
	outfile.close();
}

void WIPLogger::flush()
{
	if(!m_initialized)
		return;
	outfile<<_buf.data();
	_buf.clear();
	_current_lines = 0;
}

WIPLogger::~WIPLogger()
{

}

void WIPLogger::parse_flags(unsigned int flag)
{
	time_t tt = time(NULL);
	tm* t= localtime(&tt);
	//there is a '/0'
	char date[20];
	sprintf(date,"%04d-%02d-%02d-%02d:%02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	_current_line.clear();
	//_end_str.clear();
	switch (flag)
	{
	case WIP_INFO:
		{

			_current_line = string(date)+":  [INFO]";
		}
		break;
	case WIP_NOTE:

		_current_line = string(date)+":  [NOTE]";
		break;
	case WIP_WARNING:

		_current_line = string(date)+":  [WARNING]";
		break;
	case WIP_ERROR:
		{

			_current_line = string(date)+":  [ERROR] " __FILE__ "  ";
			char temp[5];
			sprintf(temp,"%04d",__LINE__);
			_current_line += temp;

		}
		break;
	case WIP_RAW:
	{
		_current_line = "";
	}
		break;
	default:
		_current_line = string(date)+":  [INFO]";

		break;
	}
}

void WIPLogger::debug_log(unsigned int flags,const char* buffer,...)
{		
  _mut.lock();
	if(!m_initialized)
		return;
	parse_flags(flags);
	va_list vl;

	char temp[MAX_SINGLE_LINE_CHAR_NUM];
	va_start(vl,buffer);
	vsprintf(temp,buffer,vl);
	va_end(vl);

	_current_line += temp;
	_current_line += '\n';
	_buf += _current_line;
	_current_lines++;

	if(_current_lines>=MAX_DEBUG_LINE_LEN)
	{
		flush();
	}
  _mut.unlock();

}
void WIPLogger::debug_print(unsigned int flags,const char* buffer,...)
{
  _mut.lock();
	parse_flags(flags);
	printf(_current_line.data());
	printf(" ");
	va_list vl;

	va_start(vl,buffer);
	vprintf(buffer,vl);
	va_end(vl);
	printf("\n");

	fflush(stdout);
  _mut.unlock();
}

void WIPLogger::debug_print(const char* buffer,...)
{
  _mut.lock();
	va_list vl;

	va_start(vl,buffer);
	vprintf(buffer,vl);
	va_end(vl);
	printf("\n");

	fflush(stdout);
  _mut.unlock();
}

void WIPLogger::new_log()
{
  if (!m_initialized)
    return;
  _mut.lock();
	time_t tt = time(NULL);
	tm* t= localtime(&tt);
	char date[11];
	sprintf(date,"%04d_%02d_%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday);
	char tim[9];
	sprintf(tim,"%02d:%02d:%02d", t->tm_hour,t->tm_min,t->tm_sec);

	
	if(outfile.is_open())
		outfile.close();
	string timeinfo = string(date)+"-"+tim;
	int n = timeinfo.length();
	for(int i=0;i<n;++i)
	{
		if(timeinfo[i]==' '||timeinfo[i]==':')
			timeinfo[i] = '_';
	}
	string file_path = _default_path + timeinfo + ".log";

	m_initialized = true;
	outfile.open(file_path,ios::out|ios::app);
  _mut.unlock();
}

void WIPLogger::debug( unsigned int flags,const char* buffer,... )
{
  if (!m_initialized)
    return;

  _mut.lock();
	parse_flags(flags);
	printf(_current_line.data());
	printf(" ");
	va_list vl;

	va_start(vl,buffer);
	vprintf(buffer,vl);
	va_end(vl);
	printf("\n");

	
	parse_flags(flags);

	va_list vl1;
	char temp[MAX_SINGLE_LINE_CHAR_NUM];
	va_start(vl1,buffer);
	vsprintf(temp,buffer,vl1);
	va_end(vl1);

	_current_line += temp;
	_current_line += '\n';
	_buf += _current_line;
	_current_lines++;

	if(_current_lines>=MAX_DEBUG_LINE_LEN)
	{
		flush();
	}
  _mut.unlock();
}

WIPLogger* g_logger = WIPLogger::get_instance();