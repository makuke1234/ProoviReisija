#include "iniFile.h"
#include "fileHelper.h"

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




bool IniString_init(IniString_t * restrict istr, const char * restrict str, intptr_t length)
{
	assert(istr != NULL);
	if (str == NULL)
	{
		str = "";
	}

	uint8_t realLen = (length == -1) ? strlen(str) : strnlen_s(str, (size_t)length);

	if ((istr->str = malloc(sizeof(char) * (realLen + 1))) == NULL)
	{
		return false;
	}

	memcpy(istr->str, str, sizeof(char) * realLen);
	istr->str[realLen] = '\0';
	istr->len = realLen;
	return true;
}
IniString_t * IniString_make(const char * restrict str, intptr_t length)
{
	IniString_t * mem = malloc(sizeof(IniString_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!IniString_init(mem, str, length))
	{
		free(mem);
		return NULL;
	}
	return mem;
}
bool IniString_initEscape(IniString_t * restrict istr, const char * restrict str, intptr_t length)
{
	assert(istr != NULL);
	if (str == NULL)
	{
		str = "";
	}

	if ((istr->str = ini_escapeStr_s(str, length, &istr->len)) == NULL)
	{
		return false;
	}
	--istr->len;

	return true;
}
IniString_t * IniString_makeEscape(const char * restrict str, intptr_t length)
{
	IniString_t * mem = malloc(sizeof(IniString_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!IniString_initEscape(istr, str, length))
	{
		free(mem);
		return NULL;
	}
	return mem;
}

void IniString_destroy(IniString_t * restrict istr)
{
	assert(istr != NULL);
	if (istr->str != NULL)
	{
		free(istr->str);
		istr->str = NULL;
	}
}
void IniString_free(IniString_t * restrict istr)
{
	assert(istr != NULL);
	IniString_destroy(istr);
	free(istr);
}



char * ini_escapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize)
{
	assert(string != NULL);
	assert(psize  != NULL);

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

	*psize = estrSize;
	return estr;
}
char * ini_escapeStr(const char * restrict string, intptr_t length)
{
	size_t sz;
	return ini_escapeStr_s(string, length, &sz);
}
char * ini_unescapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize)
{
	assert(string != NULL);
	assert(psize  != NULL);

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
	
	*psize = estrSize;
	return estr;
}
char * ini_unescapeStr(const char * restrict string, intptr_t length)
{
	size_t sz;
	return ini_unescapeStr_s(string, length, &sz);
}


IniE_t ini_checkData(const char * restrict string, intptr_t length)
{
	assert(string != NULL);

	size_t realLen = (length == -1) ? strlen(string) : strnlen_s(string, (size_t)length);

	for (const char * end = string + realLen; string != end;)
	{
		if ((*string == ' ') || (*string == '\t') || (*string == '\n') || (*string == '\r'))
		{
			++string;
			continue;
		}
		else if ((*string == ';') || (*string == '#'))
		{
			// Skip till end of line
			++string;
			for (; string != end; ++string)
			{
				if ((*string == '\n') || (*string == '\r'))
				{
					++string;
					break;
				}
			}
		}
		else if (*string == '[')
		{
			// Section
			++string;
			bool found = false;
			for (; string != end; ++string)
			{
				if (*string == '\\')
				{
					++string;
					if (string == end)
					{
						return IniE_ESCAPE;
					}
					continue;
				}
				else if (*string == ']')
				{
					++string;
					found = true;
					break;
				}
			}
			if (!found)
			{
				return IniE_SECTION;
			}
			for (; string != end; ++string)
			{
				if ((*string == '\n') || (*string == '\r'))
				{
					++string;
					break;
				}
			}
		}
		else
		{
			// Key-value pair
			for (; string != end; ++string)
			{
				if (*string == '\\')
				{
					++string;
					if (string == end)
					{
						return IniE_ESCAPE;
					}
					continue;
				}
				else if ((*string == ' ') || (*string == '\t') || (*string == '=') || (*string == ':'))
				{
					break;
				}
			}
			bool found = false;
			for (; string != end; ++string)
			{
				if ((*string == ' ') || (*string == '\t'))
				{
					continue;
				}
				else if ((*string == '=') || (*string == ':'))
				{
					++string;
					found = true;
					break;
				}
				else
				{
					return IniE_VALUE;
				}
			}
			if (found == false)
			{
				return IniE_VALUE;
			}
			for (; string != end; ++string)
			{
				if ((*string == ' ') || (*string == '\t'))
				{
					continue;
				}
				else
				{
					break;
				}
			}
			for (; string != end; ++string)
			{
				if ((*string == '\n') || (*string == '\r'))
				{
					++string;
					break;
				}
			}
		}
	}
	
	return IniE_OK;
}
IniE_t ini_checkFile(const char * restrict fileName)
{
	assert(fileName != NULL);
	
	size_t sz;
	char * str = fhelper_read(fileName, &sz);
	
	if (str == NULL)
	{
		return IniE_MEM;
	}
	IniE_t code = ini_checkData(str, (intptr_t)(sz - 1));
	free(str);
	return code;
}

IniE_t ini_parseData(const char * restrict string, intptr_t length, IniData_t * restrict pini)
{
	assert(string != NULL);
	assert(pini != NULL);
	
	size_t realLen = (length == -1) ? strlen(string) : strnlen_s(string, (size_t)length);

	// Check
	IniE_t code = ini_checkData(string, (intptr_t)realLen);
	if (code != IniE_OK)
	{
		return code;
	}

	// Parse
	for (const char * end = string + realLen; string != end;)
	{

	}

	return IniE_OK;
}
IniE_t ini_parseFile(const char * restrict fileName, IniData_t * restrict pini)
{
	assert(fileName != NULL);
	assert(pini != NULL);
	
	size_t sz;
	char * str = fhelper_read(fileName, &sz);
	
	if (str == NULL)
	{
		return IniE_MEM;
	}
	IniE_t code = ini_parseData(str, (intptr_t)(sz - 1), pini);
	free(str);
	return code;
}

