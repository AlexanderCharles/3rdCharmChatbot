
#include "String.h"

#include <string.h>
#include <ctype.h>



ssize_t
ScanForToken(const char* i_str, char i_token)
{
	size_t i;
	
	i = 0; 
	
	while (i < strlen(i_str))
	{
		if (i_str[i] == i_token) return(i);
		else if (i_str[i] == '\0') break;
		++i;
	}
	
	return(-1);
}

ssize_t
ReverseScanForToken(const char* i_str, char i_token)
{
	ssize_t i;
	
	i = strlen(i_str) - 1; 
	
	while (i >= 0)
	{
		if (i_str[i] == i_token) return(i);
		else if (i_str[i] == '\0') break;
		--i;
	}
	
	return(-1);
}



ssize_t
ScanRangeForToken(const char* i_str, char i_token,
                  size_t i_stride)
{
	size_t i, s, l;
	
	i = 0;
	l = strlen(i_str);
	s = i_stride < l ? i_stride : l;
	
	while (i < s)
	{
		if (i_str[i] == i_token) return(i);
		else if (i_str[i] == '\0') break;
		++i;
	}
	
	return(-1);
}

ssize_t
ReverseScanRangeForToken(const char* i_str, char i_token,
                         size_t i_stride)
{
	ssize_t i;
	
	i = i_stride;
	
	while (i >= 0)
	{
		if (i_str[i] == i_token) return(i);
		else if (i_str[i] == '\0') break;
		--i;
	}
	
	return(-1);
}



ssize_t
ScanRangeForTokenCount(const char* i_str, char i_token,
                       size_t i_stride)
{
	ssize_t result;
	size_t  i, s, l;
	
	result = i = 0;
	l = strlen(i_str);
	s = i_stride < l ? i_stride : l;
	
	while (i < s)
	{
		if (i_str[i] == i_token)
		{
			result++;
		}
		
		++i;
	}
	
	return(result);
}

ssize_t
ScanForTokenCountIgnoreQuotationScope(const char* i_str, char i_token)
{
	ssize_t result;
	size_t  i, l;
	
	result = i = 0;
	l = strlen(i_str);
	
	while (i < l)
	{
		if (i_str[i] == '"')
		{
			ssize_t stringEnd = ScanForToken(&i_str[i + 1], '"') + 1;
			if (stringEnd == -1 || (size_t) stringEnd >= l)
			{
				return(-1);
			}
			
			i += stringEnd;
			++i;
			continue;
		}
		else if (i_str[i] == i_token)
		{
			result++;
		}
		
		++i;
	}
	
	return(result);
}

ssize_t
ScanRangeForTokenCountIgnoreQuotationScope(const char* i_str, char i_token,
                                           size_t  i_stride)
{
	ssize_t result;
	size_t  i, s, l;
	
	result = i = 0;
	l = strlen(i_str);
	s = i_stride < l ? i_stride : l;
	
	while (i < s)
	{
		if (i_str[i] == '"')
		{
			ssize_t stringEnd = ScanForToken(&i_str[i + 1], '"') + 1;
			if (stringEnd == -1 || (size_t) stringEnd >= l)
			{
				return(-1);
			}
			
			i += stringEnd;
			++i;
			continue;
		}
		else if (i_str[i] == i_token)
		{
			result++;
		}
		
		++i;
	}
	
	return(result);
}



bool
StrIsAlphanumeric(char* i_str)
{
	ssize_t i;
	
	if ((i = strlen(i_str) - 1) <= 0) return(false);
	
	while (i >= 0)
	{
		if (!IsAlphanumeric(i_str[i])) return(false);
		--i;
	}
	
	return(true);
}

bool
StrIsNumeric(char* i_str)
{
	ssize_t i;
	
	if ((i = strlen(i_str) - 1) < 0) return(false);
	
	while (i >= 0)
	{
		if (!IsNumeric(i_str[i])) return(false);
		--i;
	}
	
	return(true);
}



bool
IsWhitespace(char i_token)
{
	/* Not sure if \n would be considered whitespace but this is fine */
	return(i_token == '\n' || i_token == '\r' ||
	       i_token == '\t' ||
	       i_token == ' ');
}

bool
IsAlphanumeric(char i_token)
{
	return((i_token >= '0' && i_token <= '9') ||
	       (i_token >= 'A' && i_token <= 'Z') ||
	       (i_token >= 'a' && i_token <= 'z'));
}

bool
IsNumeric(char i_token)
{
	return(i_token >= '0' && i_token <= '9');
}



char*
stristr(const char* i_a, const char* i_b)
{
	const char* p1 = i_a;
	const char* p2 = i_b;
	const char* r = *p2 == 0 ? i_a : 0;
	
	while (*p1 != 0 && * p2 != 0)
	{
		if (toupper((unsigned char) *p1) == toupper((unsigned char) *p2))
		{
			if (r == 0)
			{
				r = p1;
			}
			p2++;
		}
		else
		{
			p2 = i_b;
			if (r != 0)
			{
				p1 = r + 1;
			}
			
			if (toupper((unsigned char)*p1) == toupper((unsigned char)*p2))
			{
				r = p1;
				p2++;
			}
			else
			{
				r = 0;
			}
		}
		
		p1++;
	}
	
	return *p2 == 0 ? (char*)r : 0;
}

/* The memmove impl crashes like 1/100 of the time lol */
char*
strr(char* io_str, char* i_term, char* i_replacementTerm)
{
#if 0
	/* this is a mess...... */
	char  dirtyFix[512] = { 0 };
	char* endOfTerm;
	int   l, tlen;
	int   startOfEndOfTerm, endOfTermDelta;
	
	l = strlen(io_str);
	tlen = strlen(i_term);
	
	if ((endOfTerm = strstr(io_str, i_term)) != NULL)
	{
		endOfTermDelta = endOfTerm - io_str;
		/* I.E. at the start of i_term */
		startOfEndOfTerm = tlen - endOfTermDelta;
		
		/* - 1 so theres no < */
		strncpy(dirtyFix, io_str, startOfEndOfTerm - 1);
		strcpy(&dirtyFix[strlen(dirtyFix)], i_replacementTerm);
		
		strncpy(&dirtyFix[strlen(dirtyFix)], &io_str[endOfTermDelta + tlen],
		        (l - 1) - (endOfTermDelta + tlen - 1));
		strcpy(io_str, dirtyFix);
	}
#else
	char* start;
	int   l, tlen, rlen, dlen;
	
	l = strlen(io_str);
	
	if ((start = strstr(io_str, i_term)) != NULL)
	{
		tlen = strlen(i_term);
		rlen = strlen(i_replacementTerm);
		dlen = rlen - tlen;
		
		memmove(&start[rlen], &start[tlen], (int) (&io_str[l] - &start[tlen]));
		io_str[l + dlen] = '\0';
		strncpy(start, i_replacementTerm, rlen);
	}
	
	
#endif
	return(io_str);
}

