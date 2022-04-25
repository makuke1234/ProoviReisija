#include "iniFile.h"
#include "fileHelper.h"
#include "logger.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


bool g_ini_strAppendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch)
{
	assert(pstr  != NULL);
	assert(psize != NULL);
	assert(pcap  != NULL);
	
	// Kontrollib, äkki praegune sõnemassiiv vajab suurendamist
	if ((*psize) >= (*pcap))
	{
		// Uus suurus on praegusest suurusest vähemalt 2 korda suurem, väldib ka olukorda, et muidu 0 puhul ei suurendataks
		size_t newcap = ((*psize) + 1) * 2;
		// Proovib mälu juurde allokeerida, kontrollib õnnestumist
		char * newmem = realloc(*pstr, sizeof(char) * newcap);
		if (newmem == NULL)
		{
			return false;
		}
		*pstr = newmem;
		*pcap = newcap;
	}

	// Lisab soovitud märgendi sõne lõppu
	(*pstr)[*psize] = ch;
	++(*psize);

	return true;
}
bool g_ini_strAppendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint)
{
	assert(pstr  != NULL);
	assert(psize != NULL);
	assert(pcap  != NULL);

	// Unicode Code Pointi lisamisel arvestan, et eelnev Unicode'iga tegelev funktsioon
	// võis ebaõnnestuda, sel juhul tagastab selline funktsioon "keelatud" väärtuse, milleks
	// on UTF-süsteemis mingi "non-character" märgend, näiteks 0xFFFF
	if (codePoint == FORBIDDEN_CODEPOINT)
	{
		return false;
	}

	// Tegu on 1-baidisega UTF-8 süsteemis
	if (codePoint <= 0x7F)
	{
		return g_ini_strAppendCh(pstr, psize, pcap, (char)codePoint);
	}
	// Tegu on 2-baidisega UTF-8 süsteemis
	else if (codePoint <= 0x07FF)
	{
		if (!g_ini_strAppendCh(pstr, psize, pcap, (char)(0xC0 | ((codePoint >> 6) & 0x1F))))
		{
			return false;
		}
		return g_ini_strAppendCh(pstr, psize, pcap, (char)(0x80 | (codePoint & 0x3F)));
	}
	// Tegu on 3-baidisega UTF-8 süsteemis, kuna minu code point on 16-bitine,
	// siis on alati garanteeritud, et code point on väiksem kui 0xFFFF
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
	// Kui märgend on vahemikus '0' kuni '9' kaasaarvatud, siis tuleb lahutada '0' väärtus
	// Kui märgend on vahemikus 'A' kuni 'F' kaasaarvatud, siis tuleb lahutada 'A' väärtus ning lisada 10
	// Vastasel juhul on tegu "keelatud" sisendiga, mis ei ole hex, tagastatakse viga näitav väärtus 0xFF
	return ((ch >= '0') && (ch <= '9')) ? (uint8_t)(ch - '0') : ((ch >= 'A') && (ch <= 'F')) ? (uint8_t)(ch - 'A' + 10) : 0xFF;
}
uint16_t g_ini_strCPFromStr(const char * restrict str)
{
	assert(str != NULL);

	uint16_t cp = 0x0000;
	for (size_t i = 0; i < 4; ++i)
	{
		// Siiamaani saadud arvu nihutatakse 4 biti võrra vasakule,
		// uusi märgendeid loetakse ning kirjutatakse vasakult-paremale
		cp <<= 4;
		// Rakendatakse bitmask, et olla kindel, et alumised 4 bitti on "puhtad"
		cp &= 0xFFF0;
		uint16_t res = (uint16_t)g_ini_strHexToNum(str[i]);
		// Kui märgendit ei õnnestunud hexi konverteerida, siis järelikult pole tegu hex-iga,
		// tagastatakse veakood
		if (res > 0x0F)
		{
			return FORBIDDEN_CODEPOINT;
		}
		// Saadud hex märgend lisatakse arvu
		cp |= res;
	}

	return cp;
}

char * g_ini_escapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize)
{
	assert(string != NULL);
	assert(psize  != NULL);

	// leitakse reaalne sõne pikkus
	size_t realLen = (length == -1) ? strlen(string) : strnlen_s(string, (size_t)length);
	
	size_t estrSize = 0, estrCap = realLen + 1;
	char * estr = malloc(sizeof(char) * estrCap);
	if (estr == NULL)
	{
		return NULL;
	}

	for (const char * end = string + realLen; string != end;)
	{
		// Kui leitakse tagurpidi kaldkriips, siis minnake järgmise märgendi juurde
		if (*string == '\\')
		{
			++string;
			// Kui ei jõutud lõppu
			if (string != end)
			{
				// Muutuja, mis tähistab lisatavat märgendit
				char appendable = 0;
				// Muutuja, mis tähistab seda, kas märgend tuleb lisada või mitte
				bool append = false;
				switch (*string)
				{
				// Tavaliselt lisatavad märgendid
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
					// Unicode'i hex code point
					// Liigub järgmise märgendi juurde, nüüd peaks olema 4 märgendit, mis tähistavad hex-koodi
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
					// Ei olnud nelja märgendit :(, programm vabastab mälu ning tagastab viga näitava NULLi
					/* fall through */
				default:
					free(estr);
					return NULL;
				}

				// Tuleb lisada märgend, s.o ei ole veel lisatud

				if (append)
				{
					// Ei ole vee välja mõeldud, mida lisada :)
					if (appendable == 0)
					{
						switch (*string)
						{
						// "erilised" märgendid
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
					// Liigub järgmise märgendi juurde
					++string;
				}
			}
			// Lõppu jõudmise korral järgmine for-loopi tsükkel katkestav töö
		}
		else
		{
			// Tavaline märgend, kontrollitakse lisamise õnnestumist
			if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, *string))
			{
				free(estr);
				return NULL;
			}
			// Minnakse järgmise märgendi juurde
			++string;
		}
	}

	// Lisab null-terminaatori sõne lõppu
	if (!g_ini_strAppendCh(&estr, &estrSize, &estrCap, '\0'))
	{
		free(estr);
		return NULL;
	}
	
	// Kui ära kasutatud ruum on väiksem allokeeritud mälumahust, siis optimeerib mälukasutust
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
	// Allokeerib mälu tulemuse jaoks
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

	// Optimeerib võimalusel mälukasutust
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




void iniString_zero(iniString_t * restrict pstr)
{
	assert(pstr != NULL);

	*pstr = (iniString_t){
		.str = NULL,
		.len = 0
	};
}
bool iniString_init(iniString_t * restrict pstr, const char * restrict str, intptr_t length)
{
	assert(pstr != NULL);

	// Ka NULL on aktsepteeritud sõnena, siis tehakse tühi sõne lihtsalt
	if (str == NULL)
	{
		str = "";
	}

	size_t realLen = (length == -1) ? strlen(str) : strnlen_s(str, (size_t)length);

	if ((pstr->str = malloc(sizeof(char) * (realLen + 1))) == NULL)
	{
		return false;
	}

	memcpy(pstr->str, str, sizeof(char) * realLen);
	pstr->str[realLen] = '\0';
	pstr->len = realLen;
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
bool iniString_initEscape(iniString_t * restrict pstr, const char * restrict str, intptr_t length)
{
	assert(pstr != NULL);
	if (str == NULL)
	{
		str = "";
	}

	if ((pstr->str = g_ini_escapeStr_s(str, length, &pstr->len)) == NULL)
	{
		return false;
	}
	// Null-terminaator lahutatakse maha
	--pstr->len;

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
bool iniString_initEscapeLower(iniString_t * restrict pstr, const char * restrict str, intptr_t length)
{
	assert(pstr != NULL);

	if (!iniString_initEscape(pstr, str, length))
	{
		return false;
	}

	// Sõne konverteeritakse väiketähtedeks pärast
	for (char * it = pstr->str, * end = pstr->str + pstr->len; it != end; ++it)
	{
		*it = (char)tolower(*it);
	}

	return true;
}
iniString_t * iniString_makeEscapeLower(const char * restrict str, intptr_t length)
{
	iniString_t * mem = malloc(sizeof(iniString_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!iniString_initEscapeLower(mem, str, length))
	{
		free(mem);
		return NULL;
	}
	return mem;
}

bool iniString_initCopy(iniString_t * restrict pstr, const iniString_t * restrict src)
{
	assert(pstr != NULL);
	assert(src  != NULL);
	assert(pstr != src);

	pstr->str = malloc(sizeof(char) * (src->len + 1));
	if (pstr->str == NULL)
	{
		return false;
	}
	memcpy(pstr->str, src->str, sizeof(char) * (src->len + 1));
	pstr->len = src->len;
	
	return true;
}
iniString_t * iniString_makeCopy(const iniString_t * restrict src)
{
	assert(src != NULL);

	iniString_t * mem = malloc(sizeof(iniString_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!iniString_initCopy(mem, src))
	{
		free(mem);
		return NULL;
	}

	return mem;
}

void iniString_destroy(iniString_t * restrict pstr)
{
	assert(pstr != NULL);
	// Mälu vabastatakse
	if (pstr->str != NULL)
	{
		free(pstr->str);
		pstr->str = NULL;
	}
}
void iniString_free(iniString_t * restrict pstr)
{
	assert(pstr != NULL);
	iniString_destroy(pstr);
	// Vabastatakse ka dünaamiliselt allokeeritud struktuuri mälu
	free(pstr);
}


bool iniValue_init(
	iniValue_t * restrict pval,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
)
{
	assert(pval != NULL);
	
	// INI-failis olevad "võtmed" ei tohiks olla suurtähetundlikud ...
	if (!iniString_initEscapeLower(&pval->key, keystr, keylen))
	{
		return false;
	}
	// ... Väärtused aga on
	else if (!iniString_initEscape(&pval->value, valstr, vallen))
	{
		iniString_destroy(&pval->key);
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

void iniValue_destroy(iniValue_t * restrict pval)
{
	assert(pval != NULL);

	iniString_destroy(&pval->key);
	iniString_destroy(&pval->value);
}
void iniValue_free(iniValue_t * restrict pval)
{
	assert(pval != NULL);

	iniValue_destroy(pval);
	free(pval);
}


bool iniSection_init(iniSection_t * restrict psect, const char * sectname, intptr_t sectnameLen)
{
	assert(psect != NULL);

	// INI-failis olev sektsiooni-identifikaator ei tohiks olla tõstutundlik
	if (!iniString_initEscapeLower(&psect->section, sectname, sectnameLen))
	{
		return false;
	}

	psect->values    = NULL;
	psect->numValues = 0;
	psect->maxValues = 0;

	// Initsialiseeritakse räsitabel konkreetse sektsiooni võtmeväärtuste jaoks
	if (!hashMapCK_init(&psect->valueMap, 1))
	{
		// Kui see ei õnnestunud, siis vabastatakse ka sektsiooni enda struktuur
		iniString_destroy(&psect->section);
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
	iniSection_t * restrict psect,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
)
{
	assert(psect != NULL);

	// Vajadusel allokeeritakse mälu väärtuste massiivi jaoks juurde
	if (psect->numValues >= psect->maxValues)
	{
		size_t newcap = (psect->numValues + 1) * 2;
		iniValue_t ** newmem = realloc(psect->values, sizeof(iniValue_t *) * newcap);
		if (newmem == NULL)
		{
			return false;
		}

		// "Värskelt" allokeeritud mälu nullitakse ära, memset ei saa kasutada, sest
		// NULL ei ole garanteeritud olema väärtusega 0
		for (size_t i = psect->maxValues; i < newcap; ++i)
		{
			newmem[i] = NULL;
		}

		psect->values    = newmem;
		psect->maxValues = newcap;
	}

	// Initsialiseeritakse uus väärtus
	iniValue_t * val = iniValue_make(keystr, keylen, valstr, vallen);
	if (val == NULL)
	{
		return false;
	}

	// Väärtus lisatakse räsitabelisse
	if (!hashMapCK_insert(&psect->valueMap, val->key.str, val))
	{
		iniValue_free(val);
		return false;
	}

	// Väärtus lisatakse ka väärtuste massiivi, et oleks võimalik väärtuste järjekord
	// säilitada konstantse keerukuse juures
	psect->values[psect->numValues] = val;
	val->idx = psect->numValues;

	++psect->numValues;

	return true;
}
iniValue_t * iniSection_getValue(iniSection_t * restrict psect, const char * restrict keystr)
{
	assert(psect  != NULL);
	assert(keystr != NULL);

	hashNodeCK_t * node = hashMapCK_get(&psect->valueMap, keystr);
	if (node == NULL)
	{
		return NULL;
	}
	return node->value;
}
bool iniSection_removeValue(iniSection_t * restrict psect, const char * restrict keystr)
{
	assert(psect  != NULL);
	assert(keystr != NULL);

	// Esmalt proovitakse väärtus eemaldada räsitabelist
	iniValue_t * val = hashMapCK_remove(&psect->valueMap, keystr);
	if (val == NULL)
	{
		return false;
	}

	psect->values[val->idx] = NULL;
	// Kui tegu oli viimase väärtusega massiivis, siis saab vähendada praegu käigus olevate
	// pesade arvu
	if (psect->numValues == (val->idx + 1))
	{
		--psect->numValues;
	}
	// Vabastatakse väärtuse struktuuri mälu
	iniValue_free(val);
	return true;
}

void iniSection_destroy(iniSection_t * restrict psect)
{
	assert(psect != NULL);

	// Vabastatakse sektsiooni nime mälu
	iniString_destroy(&psect->section);
	// Võimalusel vabastatakse väärtuste mälu
	if (psect->values != NULL)
	{
		for (size_t i = 0; i < psect->numValues; ++i)
		{
			if (psect->values[i] != NULL)
			{
				iniValue_free(psect->values[i]);
			}
		}
		free(psect->values);
		psect->values = NULL;
	}
	hashMapCK_destroy(&psect->valueMap);
}
void iniSection_free(iniSection_t * restrict psect)
{
	assert(psect != NULL);
	iniSection_destroy(psect);
	free(psect);
}


bool ini_init(ini_t * restrict pini)
{
	assert(pini != NULL);

	pini->sections    = NULL;
	pini->numSections = 0;
	pini->maxSections = 0;

	if (!hashMapCK_init(&pini->sectionMap, 1))
	{
		return false;
	}

	if (!ini_addSection(pini, "", 0))
	{
		ini_destroy(pini);
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

bool ini_addSection(ini_t * restrict pini, const char * restrict secstr, intptr_t seclen)
{
	assert(pini != NULL);

	// Vajadusel allokeerib mälu juurde
	if (pini->numSections >= pini->maxSections)
	{
		size_t newcap = (pini->numSections + 1) * 2;
		iniSection_t ** newmem = realloc(pini->sections, sizeof(iniSection_t *) * newcap);
		if (newmem == NULL)
		{
			return false;
		}

		for (size_t i = pini->maxSections; i < newcap; ++i)
		{
			newmem[i] = NULL;
		}

		pini->sections    = newmem;
		pini->maxSections = newcap;
	}

	// Teeb uue sektsiooni
	iniSection_t * sec = iniSection_make(secstr, seclen);
	if (sec == NULL)
	{
		return false;
	}

	// Lisab uue sektsiooni nii räsitabelisse kui ka massiivi, hoiab järjekorda meeles

	if (!hashMapCK_insert(&pini->sectionMap, sec->section.str, sec))
	{
		iniSection_free(sec);
		return false;
	}

	sec->idx = pini->numSections;
	pini->sections[pini->numSections] = sec;
	++pini->numSections;

	return true;
}
iniSection_t * ini_getSection(ini_t * restrict pini, const char * restrict secstr)
{
	assert(pini   != NULL);
	assert(secstr != NULL);

	hashNodeCK_t * node = hashMapCK_get(&pini->sectionMap, secstr);
	if (node == NULL)
	{
		return NULL;
	}
	return node->value;
}
bool ini_removeSection(ini_t * restrict pini, const char * restrict secstr)
{
	assert(pini  != NULL);
	assert(secstr != NULL);

	iniSection_t * val = ini_getSection(pini, secstr);
	if (val == NULL)
	{
		return false;
	}

	pini->sections[val->idx] = NULL;
	// Kui tegu oli viimase sektsiooniga, siis saab vähendada käigus olevate pesade arvu
	if (pini->numSections == (val->idx + 1))
	{
		--pini->numSections;
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
			// Ignoreerib whitespace märgendeid
			++string;
			continue;
		}
		else if ((*string == ';') || (*string == '#'))
		{
			// Ignoreerib kommentaare INI-failis
			// Liigub rea lõppu
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
			// Leidis sektsiooni
			++string;
			bool found = false;
			// Otsib üles sektsiooni nime lõpu
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

iniErr_t ini_initData(ini_t * restrict pini, const char * restrict string, intptr_t length)
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
iniErr_t ini_initFile(ini_t * restrict pini, const char * restrict fileName)
{
	assert(fileName != NULL);
	assert(pini != NULL);
	
	size_t sz;
	char * str = fhelper_read(fileName, &sz);
	if (str == NULL)
	{
		return inieMEM;
	}

	writeLogger("File contents: %s", str);

	iniErr_t code = ini_initData(pini, str, (intptr_t)(sz - 1));
	free(str);
	return code;
}

void ini_destroy(ini_t * restrict pini)
{
	if (pini->sections != NULL)
	{
		// Vabastab sektsioonidele kuuluva mälu ...
		for (size_t i = 0; i < pini->numSections; ++i)
		{
			if (pini->sections[i] != NULL)
			{
				iniSection_free(pini->sections[i]);
			}
		}
		// ... ning massiivi enda
		free(pini->sections);
		pini->sections = NULL;
	}
	hashMapCK_destroy(&pini->sectionMap);
}
void ini_free(ini_t * restrict pini)
{
	assert(pini != NULL);
	ini_destroy(pini);
	free(pini);
}

