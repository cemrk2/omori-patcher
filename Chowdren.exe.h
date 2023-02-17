typedef unsigned char   undefined;

typedef unsigned long long    GUID;
typedef pointer32 ImageBaseOffset32;

typedef unsigned char    bool;
typedef unsigned char    byte;
typedef unsigned int    dword;
typedef long double    longdouble;
typedef long long    longlong;
typedef unsigned long long    qword;
typedef unsigned char    uchar;
typedef unsigned int    uint;
typedef unsigned long    ulong;
typedef unsigned long long    ulonglong;
typedef unsigned char    undefined1;
typedef unsigned short    undefined2;
typedef unsigned int    undefined4;
typedef unsigned long long    undefined8;
typedef unsigned short    ushort;
typedef unsigned short    wchar16;
typedef short    wchar_t;
typedef unsigned short    word;
typedef struct JSMemoryUsage JSMemoryUsage, *PJSMemoryUsage;

typedef ulonglong uint64_t;

struct JSMemoryUsage {
    uint64_t malloc_size;
    uint64_t malloc_limit;
    uint64_t memory_used_size;
    uint64_t malloc_count;
    uint64_t memory_used_count;
    uint64_t atom_count;
    uint64_t atom_size;
    uint64_t str_count;
    uint64_t str_size;
    uint64_t obj_count;
    uint64_t obj_size;
    uint64_t prop_count;
    uint64_t prop_size;
    uint64_t shape_count;
    uint64_t shape_size;
    uint64_t func_count;
    uint64_t func_size;
    uint64_t func_code_size;
    uint64_t js_func_pc2line_count;
    uint64_t js_func_pc2line_size;
    uint64_t c_func_count;
    uint64_t array_count;
    uint64_t fast_array_count;
    uint64_t fast_array_elements;
    uint64_t binary_object_count;
    uint64_t binary_object_size;
};

