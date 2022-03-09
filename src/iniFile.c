#include "iniFile.h"
#include "fileHelper.h"

#include <stdlib.h>
#include <string.h>


bool g_ini_strAppendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch)
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
bool g_ini_strAppendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint)
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
		return g_ini_strAppendCh(pstr, psize, pcap, (char)codePoint);
	}
	else if (codePoint <= 0x07FF)
	{
		if (!g_ini_strAppendCh(pstr, psize, pcap, (char)(0xC0 | ((codePoint >> 6) & 0x1F))))
		{
			return false;
		}
		return g_ini_strAppendCh(pstr, psize, pcap, (char)(0x80 | (codePoint & 0x3F)));
	}
	else /*if (codePoint <= 0xFFFF)*/
	{
		if (!g_ini_strAppendCh(pstr, psize, pcap, (char)(0xE0 | ((codePoint >> 12) & 0x0F))))
		{
			return false;
		}
		if (!g_ini_strAppendCh(pstr, psize, pcap, (char)(0x80 | ((codePoint >> 6) & 0x3F))))
		{
			return false;
		}
		return g_ini_strAppendCh(pstr, psize, pcap, (char)(0x80 | (codePoint & 0x3F)));
	}
}

uint8_t g_ini_strHexToNum(char ch)
{
	return ((ch >= '0') && (ch <= '9')) ? (uint8_t)(ch - '0') : ((ch >= 'A') && (ch <= 'F')) ? (uint8_t)(ch - 'A' + 10) : 0xFF;
}
uint16_t g_ini_strCPFromStr(const char * restrict str)
{
	assert(str != NULL);

	uint16_t cp = 0x0000;
	for (size_t i = 0; i < 4; ++i)
	{
		cp <<= 4;
		cp &= 0xFFF0;
		uint16_t res = (uint16_t)g_ini_strHexToNum(str[i]);
		if (res > 0x0F)
		{
			return FORBIDDEN_CODEPOINT;
		}
		cp |= res;
	}

	return cp;
}

char * g_ini_escapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize)
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
						if (!g_ini_strAppendCP(&estr, &estrSize, &estrCap, g_ini_strCPFromStr(string)))
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
					if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, appendable))
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
			if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, *string))
			{
				free(estr);
				return NULL;
			}
			++string;
		}
	}

	// Lisab null-terminaatori sõne lõppu
	if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, '\0'))
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
char * g_ini_escapeStr(const char * restrict string, intptr_t length)
{
	size_t sz;
	return g_ini_escapeStr_s(string, length, &sz);
}
char * g_ini_unescapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize)
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
			if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, '\\') ||
				!g_ini_strAppendCh(&estr, &estrSize, &estrCap, appendable))
			{
				free(estr);
				return NULL;
			}
		}
		else
		{
			if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, *string))
			{
				free(estr);
				return NULL;
			}
		}
	}

	// Lisab null-terminaatori sõne lõppu
	if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, '\0'))
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
char * g_ini_unescapeStr(const char * restrict string, intptr_t length)
{
	size_t sz;
	return g_ini_unescapeStr_s(string, length, &sz);
}




bool iniString_init(iniString_t * restrict istr, const char * restrict str, intptr_t length)
{
	assert(istr != NULL);

	if (str == NULL)
	{
		str = "";
	}

	size_t realLen = (length == -1) ? strlen(str) : strnlen_s(str, (size_t)length);

	if ((istr->str = malloc(sizeof(char) * (realLen + 1))) == NULL)
	{
		return false;
	}

	memcpy(istr->str, str, sizeof(char) * realLen);
	istr->str[realLen] = '\0';
	istr->len = realLen;
	return true;
}
iniString_t * iniString_make(const char * restrict str, intptr_t length)
{
	iniString_t * mem = malloc(sizeof(iniString_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!iniString_init(mem, str, length))
	{
		free(mem);
		return NULL;
	}
	return mem;
}
bool iniString_initEscape(iniString_t * restrict istr, const char * restrict str, intptr_t length)
{
	assert(istr != NULL);
	if (str == NULL)
	{
		str = "";
	}

	if ((istr->str = g_ini_escapeStr_s(str, length, &istr->len)) == NULL)
	{
		return false;
	}
	--istr->len;

	return true;
}
iniString_t * iniString_makeEscape(const char * restrict str, intptr_t length)
{
	iniString_t * mem = malloc(sizeof(iniString_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!iniString_initEscape(mem, str, length))
	{
		free(mem);
		return NULL;
	}
	return mem;
}

void iniString_destroy(iniString_t * restrict istr)
{
	assert(istr != NULL);
	if (istr->str != NULL)
	{
		free(istr->str);
		istr->str = NULL;
	}
}
void iniString_free(iniString_t * restrict istr)
{
	assert(istr != NULL);
	iniString_destroy(istr);
	free(istr);
}


bool iniValue_init(
	iniValue_t * restrict ival,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
)
{
	assert(ival != NULL);
	
	if (!iniString_init(&ival->key, keystr, keylen))
	{
		return false;
	}
	else if (!iniString_init(&ival->value, valstr, vallen))
	{
		iniString_destroy(&ival->key);
		return false;
	}
	return true;
}
iniValue_t * iniValue_make(
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
)
{
	iniValue_t * mem = malloc(sizeof(iniValue_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!iniValue_init(mem, keystr, keylen, valstr, vallen))
	{
		free(mem);
		return NULL;
	}

	return mem;
}

void iniValue_destroy(iniValue_t * restrict ival)
{
	assert(ival != NULL);
	iniString_destroy(&ival->key);
	iniString_destroy(&ival->value);
}
void iniValue_free(iniValue_t * restrict ival)
{
	assert(ival != NULL);
	iniValue_destroy(ival);
	free(ival);
}


bool iniSection_init(iniSection_t * restrict isect, const char * sectname, intptr_t sectnameLen)
{
	assert(isect != NULL);

	if (!iniString_init(&isect->section, sectname, sectnameLen))
	{
		return false;
	}

	isect->values    = NULL;
	isect->numValues = 0;
	isect->maxValues = 0;

	if (!hashMap_init(&isect->valueMap, 1))
	{
		iniString_destroy(&isect->section);
		return false;
	}

	return true;
}
iniSection_t * iniSection_make(const char * sectname, intptr_t sectnameLen)
{
	iniSection_t * mem = malloc(sizeof(iniSection_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!iniSection_init(mem, sectname, sectnameLen))
	{
		free(mem);
		return NULL;
	}

	return mem;
}

bool iniSection_addValue(
	iniSection_t * restrict isect,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
)
{
	assert(isect != NULL);

	if (isect->numValues >= isect->maxValues)
	{
		// Reallocate memory
		size_t newcap = (isect->numValues + 1) * 2;
		iniValue_t ** newmem = realloc(isect->values, sizeof(iniValue_t *) * newcap);
		if (newmem == NULL)
		{
			return false;
		}

		for (size_t i = isect->maxValues; i < newcap; ++i)
		{
			newmem[i] = NULL;
		}

		isect->values    = newmem;
		isect->maxValues = newcap;
	}

	iniValue_t * val = iniValue_make(keystr, keylen, valstr, vallen);
	if (val == NULL)
	{
		return false;
	}

	if (!hashMap_insert(&isect->valueMap, val->key.str, val))
	{
		iniValue_free(val);
		return false;
	}

	isect->values[isect->numValues] = val;
	val->idx = isect->numValues;

	++isect->numValues;

	return true;
}
iniValue_t * iniSection_getValue(iniSection_t * restrict isect, const char * restrict keystr)
{
	assert(isect  != NULL);
	assert(keystr != NULL);

	hashNode_t * node = hashMap_get(&isect->valueMap, keystr);
	if (node == NULL)
	{
		return NULL;
	}
	return node->value;
}
bool iniSection_removeValue(iniSection_t * restrict isect, const char * restrict keystr)
{
	assert(isect  != NULL);
	assert(keystr != NULL);

	iniValue_t * val = hashMap_remove(&isect->valueMap, keystr);
	if (val == NULL)
	{
		return false;
	}

	isect->values[val->idx] = NULL;
	if (isect->numValues == (val->idx + 1))
	{
		--isect->numValues;
	}
	iniValue_free(val);
	return true;
}

void iniSection_destroy(iniSection_t * restrict isect)
{
	assert(isect != NULL);

	iniString_destroy(&isect->section);
	if (isect->values != NULL)
	{
		for (size_t i = 0; i < isect->numValues; ++i)
		{
			if (isect->values[i] != NULL)
			{
				iniValue_free(isect->values[i]);
			}
		}
		free(isect->values);
		isect->values = NULL;
	}
	hashMap_destroy(&isect->valueMap);
}
void iniSection_free(iniSection_t * restrict isect)
{
	assert(isect != NULL);
	iniSection_destroy(isect);
	free(isect);
}


bool ini_init(ini_t * restrict idata)
{
	assert(idata != NULL);

	idata->sections    = NULL;
	idata->numSections = 0;
	idata->maxSections = 0;

	if (!hashMap_init(&idata->sectionMap, 1))
	{
		return false;
	}

	if (!ini_addSection(idata, "", 0))
	{
		ini_destroy(idata);
		return false;
	}

	return true;
}
ini_t * ini_make(void)
{
	ini_t * mem = malloc(sizeof(ini_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!ini_init(mem))
	{
		free(mem);
		return NULL;
	}

	return mem;
}

bool ini_addSection(ini_t * restrict idata, const char * restrict secstr, intptr_t seclen)
{
	assert(idata != NULL);

	if (idata->numSections >= idata->maxSections)
	{
		size_t newcap = (idata->numSections + 1) * 2;
		iniSection_t ** newmem = realloc(idata->sections, sizeof(iniSection_t *) * newcap);
		if (newmem == NULL)
		{
			return false;
		}

		for (size_t i = idata->maxSections; i < newcap; ++i)
		{
			newmem[i] = NULL;
		}

		idata->sections    = newmem;
		idata->maxSections = newcap;
	}

	iniSection_t * sec = iniSection_make(secstr, seclen);
	if (sec == NULL)
	{
		return false;
	}

	if (!hashMap_insert(&idata->sectionMap, sec->section.str, sec))
	{
		iniSection_free(sec);
		return false;
	}

	sec->idx = idata->numSections;
	idata->sections[idata->numSections] = sec;
	++idata->numSections;

	return true;
}
iniSection_t * ini_getSection(ini_t * restrict idata, const char * restrict secstr)
{
	assert(idata  != NULL);
	assert(secstr != NULL);

	hashNode_t * node = hashMap_get(&idata->sectionMap, secstr);
	if (node == NULL)
	{
		return NULL;
	}
	return node->value;
}
bool ini_removeSection(ini_t * restrict idata, const char * restrict secstr)
{
	assert(idata  != NULL);
	assert(secstr != NULL);

	iniSection_t * val = ini_getSection(idata, secstr);
	if (val == NULL)
	{
		return false;
	}

	idata->sections[val->idx] = NULL;
	if (idata->numSections == (val->idx + 1))
	{
		--idata->numSections;
	}
	iniSection_free(val);
	return true;
}

iniErr_t ini_checkData(const char * restrict string, intptr_t length)
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
						return inieESCAPE;
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
				return inieSECTION;
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
						return inieESCAPE;
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
					return inieVALUE;
				}
			}
			if (found == false)
			{
				return inieVALUE;
			}
			for (; string != end; ++string)
			{
				if ((*string != ' ') && (*string != '\t'))
				{
					break;
				}
			}
			if (*string == '"')
			{
				++string;
				found = false;
				for (; string != end; ++string)
				{
					if (*string == '\\')
					{
						++string;
						if (string == end)
						{
							return inieESCAPE;
						}
						continue;
					}
					else if (*string == '"')
					{
						++string;
						found = true;
						break;
					}
				}
				if (found == false)
				{
					return inieQUOTE;
				}
			}
			else
			{
				for (; string != end; ++string)
				{
					if (*string == '\\')
					{
						++string;
						if (string == end)
						{
							return inieESCAPE;
						}
						continue;
					}
					else if ((*string == ';') || (*string == '#') || (*string == '\n') || (*string == '\r'))
					{
						string += (*string != ';') && (*string != '#');
						break;
					}
				}
			}
		}
	}
	
	return inieOK;
}
iniErr_t ini_checkFile(const char * restrict fileName)
{
	assert(fileName != NULL);
	
	size_t sz;
	char * str = fhelper_read(fileName, &sz);
	
	if (str == NULL)
	{
		return inieMEM;
	}
	iniErr_t code = ini_checkData(str, (intptr_t)(sz - 1));
	free(str);
	return code;
}

iniErr_t ini_initData(const char * restrict string, intptr_t length, ini_t * restrict pini)
{
	assert(string != NULL);
	assert(pini != NULL);
	
	size_t realLen = (length == -1) ? strlen(string) : strnlen_s(string, (size_t)length);

	// Check
	iniErr_t code = ini_checkData(string, (intptr_t)realLen);
	if (code != inieOK)
	{
		return code;
	}

	if (!ini_init(pini))
	{
		return inieMEM;
	}

	// Parse
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
			// Add section
			++string;
			for (const char * secend = string; secend != end; ++secend)
			{
				if (*secend == '\\')
				{
					++secend;
					if (secend == end)
					{
						return inieESCAPE;
					}
					continue;
				}
				else if (*secend == ']')
				{
					// Add section to list
					if (!ini_addSection(pini, string, secend - string))
					{
						ini_destroy(pini);
						return inieMEM;
					}

					// Go on
					string = secend;
					++string;
					break;
				}
			}
		}
		else
		{
			const char * keystart = string, * keyend = string + 1;
			// Add key-value pair to last (current) section
			for (; string != end; ++string)
			{
				if (*string == '\\')
				{
					++string;
					if (string == end)
					{
						ini_destroy(pini);
						return inieESCAPE;
					}
					continue;
				}
				else if ((*string == ' ') || (*string == '\t') || (*string == '=') || (*string == ':'))
				{
					keyend = string;
					break;
				}
			}
			for (; string != end; ++string)
			{
				if ((*string == ' ') || (*string == '\t'))
				{
					continue;
				}
				else if ((*string == '=') || (*string == ':'))
				{
					++string;
					break;
				}
			}
			const char * valstart = string, * valend = end;
			for (; string != end; ++string)
			{
				if ((*string != ' ') && (*string != '\t'))
				{
					valstart = string;
					break;
				}
			}
			if (*valstart == '"')
			{
				++valstart;
				for (string = valstart; string != end; ++string)
				{
					if (*string == '\\')
					{
						++string;
						if (string == end)
						{
							ini_destroy(pini);
							return inieESCAPE;
						}
						continue;
					}
					else if (*string == '"')
					{
						valend = string;
						++string;
						break;
					}
				}
			}
			else
			{
				for (; string != end; ++string)
				{
					if (*string == '\\')
					{
						++string;
						if (string == end)
						{
							ini_destroy(pini);
							return inieESCAPE;
						}
						continue;
					}
					else if ((*string == ';') || (*string == '#') || (*string == '\n') || (*string == '\r'))
					{
						valend = string;
						if ((*(valend - 1) == ' ') || (*(valend - 1) == '\t'))
						{
							--valend;
							for (; valend != valstart; --valend)
							{
								if ((*valend != ' ') && (*valend != '\t'))
								{
									break;
								}
							}
							++valend;
						}
						string += (*string != ';') & (*string != '#');
						break;
					}
				}
			}

			// Get current section
			iniSection_t * cursect = pini->sections[pini->numSections - 1];
			if (!iniSection_addValue(cursect, keystart, keyend - keystart, valstart, valend - valstart))
			{
				ini_destroy(pini);
				return inieMEM;
			}
		}
	}

	return inieOK;
}
iniErr_t ini_initFile(const char * restrict fileName, ini_t * restrict pini)
{
	assert(fileName != NULL);
	assert(pini != NULL);
	
	size_t sz;
	char * str = fhelper_read(fileName, &sz);
	if (str == NULL)
	{
		return inieMEM;
	}

	iniErr_t code = ini_initData(str, (intptr_t)(sz - 1), pini);
	free(str);
	return code;
}

void ini_destroy(ini_t * restrict pini)
{
	if (pini->sections != NULL)
	{
		for (size_t i = 0; i < pini->numSections; ++i)
		{
			if (pini->sections[i] != NULL)
			{
				iniSection_free(pini->sections[i]);
			}
		}
		free(pini->sections);
		pini->sections = NULL;
	}
	hashMap_destroy(&pini->sectionMap);
}
void ini_free(ini_t * restrict pini)
{
	assert(pini != NULL);
	ini_destroy(pini);
	free(pini);
}

