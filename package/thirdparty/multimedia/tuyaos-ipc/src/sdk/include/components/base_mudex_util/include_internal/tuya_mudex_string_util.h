#ifndef __TUYA_MUDEX_STRING_H__
#define __TUYA_MUDEX_STRING_H__

#include <stddef.h>
#include <stdint.h>


size_t tuya_util_strlcpy(char *dst, const char *src, size_t size);

size_t tuya_util_strlcat(char *dst, const char *src, size_t size);

size_t tuya_util_strlcatf(char *dst, size_t size, const char *fmt, ...);


static inline  int tuya_util_isdigit(int c)
{
    return c >= '0' && c <= '9';
}

static inline  int tuya_util_isgraph(int c)
{
    return c > 32 && c < 127;
}

static inline  int tuya_util_isspace(int c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
           c == '\v';
}

static inline  int tuya_util_toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        c ^= 0x20;
    return c;
}

static inline  int tuya_util_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 0x20;
    return c;
}


int tuya_util_strcasecmp(const char *a, const char *b);


int tuya_util_strncasecmp(const char *a, const char *b, size_t n);


int tuya_util_match_name(const char *name, const char *names);

//匹配文件后缀
int tuya_util_match_ext(const char *filename, const char *extensions);

#define AV_ESCAPE_FLAG_WHITESPACE (1 << 0)

#define AV_ESCAPE_FLAG_STRICT (1 << 1)



#define AV_UTF8_FLAG_ACCEPT_INVALID_BIG_CODES          1 ///< accept codepoints over 0x10FFFF
#define AV_UTF8_FLAG_ACCEPT_NON_CHARACTERS             2 ///< accept non-characters - 0xFFFE and 0xFFFF
#define AV_UTF8_FLAG_ACCEPT_SURROGATES                 4 ///< accept UTF-16 surrogates codes
#define AV_UTF8_FLAG_EXCLUDE_XML_INVALID_CONTROL_CODES 8 ///< exclude control codes not accepted by XML

#define AV_UTF8_FLAG_ACCEPT_ALL \
    AV_UTF8_FLAG_ACCEPT_INVALID_BIG_CODES|AV_UTF8_FLAG_ACCEPT_NON_CHARACTERS|AV_UTF8_FLAG_ACCEPT_SURROGATES

int tuya_util_match_list(const char *name, const char *list, char separator);



#endif /* AVUTIL_AVSTRING_H */
