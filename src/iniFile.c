#include "iniFile.h"

#include <string.h>
#include <stdbool.h>


static inline bool s_appendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch)
{
	assert(pstr  != NULL);
	assert(psize != NULL);
	assert(pcap  != NULL);
	
	if (*psize >= *pcap)
	{
		size_t newcap = (*psize + 1) * 2; 
		char * newmem = realloc(*pstr, sizeof(char) * newcap);
		if (newmem == NULL)
		{
			return false;
		}
		*pstr = newmem;
		*pcap = newcap;
	}

	(*pstr)[*psize] = ch;
	++(*psize);

	return true;
}
static inline bool s_appendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint)
{
	assert(pstr  != NULL);
	assert(psize != NULL);
	assert(pcap  != NULL);
	if (codePoint == FORBIDDEN_CODEPOINT)
	{
		return false;
	}

	if (codePoint <= 0x7F)
	{
		return s_appendCh(pstr, psize, pcap, (char)codePoint);
	}
	else if (codePoint <= 0x07FF)
	{
		if (!s_appendCh(pstr, psize, pcap,   0xC0 | ((codePoint >> 6) & 0x1F)))
		{
			return false;
		}
		return s_appendCh(pstr, psize, pcap, 0x80 | (codePoint        & 0x3F));
	}
	else /*if (codePoint <= 0xFFFF)*/
	{
		if (!s_appendCh(pstr, psize, pcap,   0xE0 | ((codePoint >> 12) & 0x0F)))
		{
			return false;
		}
		if (!s_appendCh(pstr, psize, pcap,   0x80 | ((codePoint >>  6) & 0x3F)))
		{
			return false;
		}
		return s_appendCh(pstr, psize, pcap, 0x80 | (codePoint         & 0x3F));
	}
}
static inline uint16_t s_hexToNum(char ch)
{
	return ((ch >= '0') && (ch < '9')) ? (uint16_t)(ch - '0') : ((ch >= 'A') && (ch <= 'F')) ? (uint16_t)(ch - 'A' + 10) : 0x00FF;
}
static inline uint16_t s_codePointFromStr(const char * restrict str)
{
	uint16_t cp = 0x0000;
	for (size_t i = 0; i < 4; ++i)
	{
		cp <<= 4;
		cp &= 0xFFF0;
		uint16_t res = s_hexToNum(str[i]);
		if (res > 0x000F)
		{
			return FORBIDDEN_CODEPOINT;
		}
		cp |= res;
	}

	return cp;
}



char * ini_escapeStr(const char * restrict string, intptr_t length)
{
	size_t realLen = (length == -1) ? strlen(string) : strnlen_s(string, (size_t)length);
	
	size_t estrSize = 0, estrCap = realLen + 1;
	char * estr = malloc(sizeof(char) * estrCap);
	if (estr == NULL)
	{
		return NULL;
	}

	for (const char * end = string + realLen; string != end;)
	{
		if (*string == '\\')
		{
			++string;
			if (string != end)
			{
				char appendable = 0;
				bool append = false;
				switch (*string)
				{
				case '\\':
				case '\'':
				case '"':
				case ';':
				case '#':
				case '=':
				case ':':
					append = true;
					appendable = *string;
					break;
				case '0':
				case 'a':
				case 'b':
				case 't':
				case 'r':
				case 'n':
					append = true;
					break;
				case 'x':
					// Add unicode hex code-point
					++string;
					if ((string != end) && ((end - string) >= 4))
					{
						if (!s_appendCP(&estr, &estrSize, &estrCap, s_codePointFromStr(string)))
						{
							free(estr);
							return NULL;
						}
						string += 4;
					}
				default:
					free(estr);
					return NULL;
				}

				if (append)
				{
					if (appendable == 0)
					{
						switch (*string)
						{
						case '0':
							appendable = '\0';
							break;
						case 'a':
							appendable = '\a';
							break;
						case 'b':
							appendable = '\b';
							break;
						case 't':
							appendable = '\t';
							break;
						case 'r':
							appendable = '\r';
							break;
						case 'n':
							appendable = '\n';
							break;
						}
					}
					if (!s_appendCh(&estr, &estrSize, &estrCap, appendable))
					{
						free(estr);
						return NULL;
					}
					++string;
				}
			}
		}
		else
		{
			if (!s_appendCh(&estr, &estrSize, &estrCap, *string))
			{
				free(estr);
				return NULL;
			}
			++string;
		}
	}

	if (estrSize < estrCap)
	{
		char * newstr = realloc(estr, sizeof(char) * estrSize);
		if (newstr != NULL)
		{
			estr = newstr;
		}
	}

	return estr;
}
char * ini_unescapeStr(const char * restrict string, intptr_t length)
{
	size_t realLen = (length == -1) ? strlen(string) : strnlen_s(string, (size_t)length);

}
