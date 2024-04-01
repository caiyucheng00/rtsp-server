#ifndef __LOG__H_
#define __LOG__H_

#include <time.h>
#include <string>
#include <vector>

static std::string getTime() {
	const char* time_fmt = "%Y-%m-%d %H:%M:%S";
	time_t t = time(nullptr);
	char time_str[64];
	strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));

	return time_str;
}

static std::string getFile(std::string file) {
#ifndef WIN32
	std::string pattern = "/";
#else
	std::string pattern = "\\";
#endif // !WIN32

	std::string::size_type pos;
	std::vector<std::string> result;
	file += pattern;//扩展字符串以方便操作
	int size = file.size();
	for (int i = 0; i < size; i++) {
		pos = file.find(pattern, i);
		if (pos < size) {
			std::string s = file.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result.back();
}

#define LOGI(format, ...)  fprintf(stderr,"[INFO]%s [%s:%d] " format "\n", getTime().data(),__FILE__,__LINE__,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"[ERROR]%s [%s:%d] " format "\n",getTime().data(),__FILE__,__LINE__,##__VA_ARGS__)

#endif
