#ifndef _3CCB_UTILS_STRING_
#define _3CCB_UTILS_STRING_

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>



ssize_t ScanForToken       (const char*, char);
ssize_t ReverseScanForToken(const char*, char);

ssize_t ScanRangeForToken       (const char*, char, size_t);
ssize_t ReverseScanRangeForToken(const char*, char, size_t);

ssize_t ScanRangeForTokenCount                    (const char*, char, size_t);
ssize_t ScanForTokenCountIgnoreQuotationScope     (const char*, char);
ssize_t ScanRangeForTokenCountIgnoreQuotationScope(const char*, char, size_t);

bool StrIsAlphanumeric(char*);
bool StrIsNumeric     (char*);

bool IsWhitespace  (char);
bool IsAlphanumeric(char);
bool IsNumeric     (char);

char* stristr(const char*, const char*);
char* strr   (char*, char*, char*);

#endif
