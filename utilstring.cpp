#include "utilstring.h"

#include <stdlib.h>
#include <algorithm> 
#include <functional> 
#include <sstream>
#include <streambuf>
#include <iomanip>

// http://stackoverflow.com/questions/673240/how-do-i-print-an-unsigned-char-as-hex-in-c-using-ostream
struct HexCharStruct
{
        unsigned char c;
        HexCharStruct(unsigned char _c) : c(_c) { }
};

inline std::ostream& operator<<(std::ostream& o, const HexCharStruct& hs)
{
        return (o << std::setfill('0') << std::setw(2) << std::hex << (int) hs.c);
}

inline HexCharStruct hex(unsigned char c)
{
        return HexCharStruct(c);
}

static void bufferPrintHex(std::ostream &sout, const void* value, size_t size)
{
	if (value == NULL)
		return;
	unsigned char *p = (unsigned char*) value;
	for (size_t i = 0; i < size; i++)
	{
		sout << hex(*p);
		p++;
	}
}

std::string hexString(const void *buffer, size_t size)
{
	std::stringstream r;
	bufferPrintHex(r, buffer, size);
	return r.str();
}

/**
 * Return hex string
 * @param data
 * @return
 */
std::string hexString(const std::string &data)
{
	return hexString((void *) data.c_str(), data.size());
}

static std::string readHex(std::istream &s)
{
	std::stringstream r;
	s >> std::noskipws;
	char c[3] = {0, 0, 0};
	while (s >> c[0])
	{
		if (!(s >> c[1]))
			break;
		unsigned char x = (unsigned char) strtol(c, NULL, 16);
		r << x;
	}
	return r.str();
}

std::string hex2string(const std::string &hex)
{
	std::stringstream ss(hex);
    return readHex(ss);
}

/**
 * @brief Return binary data string
 * @param hex hex string
 * @return binary data string
 */
std::string stringFromHex(const std::string &hex)
{
	std::string r(hex.length() / 2, '\0');
	std::stringstream ss(hex);
	ss >> std::noskipws;
	char c[3] = {0, 0, 0};
	int i = 0;
	while (ss >> c[0]) {
		if (!(ss >> c[1]))
			break;
		unsigned char x = (unsigned char) strtol(c, NULL, 16);
		r[i] = x;
		i++;
	}
	return r;
}
