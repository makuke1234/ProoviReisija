#include "iniFile.h"

#include <stdlib.h>
#include <string.h>


bool ini_str_appendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch)
{
	assert(pstr  != NULL);
	assert(psize != NULL);
	assert(pcap  != NULL);
	
	if ((*psize) >= (*pcap))
	{
		size_t newcap = ((*psize) + 1) * 2; 
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
bool ini_str_appendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint)
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
		return ini_str_appendCh(pstr, psize, pcap, (char)codePoint);
	}
	else if (codePoint <= 0x07FF)
	{
		if (!ini_str_appendCh(pstr, psize, pcap, (char)(0xC0 | ((codePoint >> 6) & 0x1F))))
		{
			return false;
		}
		return ini_str_appendCh(pstr, psize, pcap, (char)(0x80 | (codePoint & 0x3F)));
	}
	else /*if (codePoint <= 0xFFFF)*/
	{
		if (!ini_str_appendCh(pstr, psize, pcap, (char)(0xE0 | ((codePoint >> 12) & 0x0F))))
		{
			return false;
		}
		if (!ini_str_appendCh(pstr, psize, pcap, (char)(0x80 | ((codePoint >> 6) & 0x3F))))
		{
			return false;
		}
		return ini_str_appendCh(pstr, psize, pcap, (char)(0x80 | (codePoint & 0x3F)));
	}
}

uint8_t ini_str_hexToNum(char ch)
{
	return ((ch >= '0') && (ch <= '9')) ? (uint8_t)(ch - '0') : ((ch >= 'A') && (ch <= 'F')) ? (uint8_t)(ch - 'A' + 10) : 0xFF;
}
uint16_t ini_str_codePointFromStr(const char * restrict str)
{
	assert(str != NULL);

	uint16_t cp = 0x0000;
	for (size_t i = 0; i < 4; ++i)
	{
		cp <<= 4;
		cp &= 0xFFF0;
		uint16_t res = (uint16_t)ini_str_hexToNum(str[i]);
		if (res > 0x0F)
		{
			return FORBIDDEN_CODEPOINT;
		}
		cp |= res;
	}

	return cp;
}



char * ini_escapeStr(const char * restrict string, intptr_t length)
{
	assert(string != NULL);

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
					appendable = *string;
					/* fall through */
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
						if (!ini_str_appendCP(&estr, &estrSize, &estrCap, ini_str_codePointFromStr(string)))
						{
							free(estr);
							return NULL;
						}
						string += 4;
						break;
					}
					/* fall through */
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
					if (!ini_str_appendCh(&estr, &estrSize, &estrCap, appendable))
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
			if (!ini_str_appendCh(&estr, &estrSize, &estrCap, *string))
			{
				free(estr);
				return NULL;
			}
			++string;
		}
	}

	// Lisab null-terminaatori sõne lõppu
	if (!ini_str_appendCh(&estr, &estrSize, &estrCap, '\0'))
	{
		free(estr);
		return NULL;
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
	assert(string != NULL);

	size_t realLen = (length == -1) ? strlen(string) : strnlen_s(string, (size_t)length);

	size_t estrSize = 0, estrCap = realLen + 1;
	char * estr = malloc(sizeof(char) * estrCap);
	if (estr == NULL)
	{
		return NULL;
	}

	for (const char * end = string + realLen; string != end; ++string)
	{
		char appendable = 0;
		switch (*string)
		{
		case '\\':
		case '\'':
		case '"':
		case ';':
		case '#':
		case '=':
		case ':':
			appendable = *string;
			break;
		case '\a':
			appendable = 'a';
			break;
		case '\b':
			appendable = 'b';
			break;
		case '\t':
			appendable = 't';
			break;
		case '\r':
			appendable = 'r';
			break;
		case '\n':
			appendable = 'n';
			break;
		}

		if (appendable != 0)
		{
			if (!ini_str_appendCh(&estr, &estrSize, &estrCap, '\\') ||
				!ini_str_appendCh(&estr, &estrSize, &estrCap, appendable))
			{
				free(estr);
				return NULL;
			}
		}
		else
		{
			if (!ini_str_appendCh(&estr, &estrSize, &estrCap, *string))
			{
				free(estr);
				return NULL;
			}
		}
	}

	// Lisab null-terminaatori sõne lõppu
	if (!ini_str_appendCh(&estr, &estrSize, &estrCap, '\0'))
	{
		free(estr);
		return NULL;
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

