#ifndef _WIN_TYPES_H
#define _WIN_TYPES_H

#include <stdint.h>
typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

typedef uint8_t __le8;
typedef uint16_t __le16;
typedef uint32_t __le32;
typedef uint64_t __le64;

typedef int8_t    __s8;
typedef int16_t   __s16;
typedef int32_t   __s32;
typedef int64_t   __s64;

#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */

#endif // _LINUX_TYPES_H