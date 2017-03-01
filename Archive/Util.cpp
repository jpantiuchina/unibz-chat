#include "Util.h"

#include <ctype.h>
#include <algorithm>


using namespace std;

static char tolower_wrapper(char ch) // Required because tolower might is inline
// Wrote my own because tolower didn't work in g++: http://stackoverflow.com/questions/19621831/undefined-symbol-toupper-in-macports-gcc-4-7-os-x-mavericks-10-9-c11
{
	if (!('A' <= ch && ch <= 'Z'))
		return ch;
	else
		return ch - ('A' - 'a');
}


void toLowerCase(string& str)
{
	transform(str.begin(), str.end(), str.begin(), tolower_wrapper);
}


