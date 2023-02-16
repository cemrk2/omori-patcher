// Types from: https://github.com/bellard/quickjs/blob/master/quickjs.c and https://github.com/bellard/quickjs/blob/master/quickjs.h
// Not using any include statements, etc. so that it can be recognized by x64dbg

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

union JSValueUnion {
    int32_t int32;
    double float64;
    void* ptr;
};

struct JSValue {
    JSValueUnion u;
    int64_t tag;
};

struct list_head {
    list_head *next;
    list_head *prev;
};

struct JSGCObjectHeader {
    int ref_count; /* must come first, 32-bit */
    uint32_t gc_obj_type;
    uint8_t mark; /* used by the GC */
    uint8_t dummy1; /* not used by the GC */
    uint16_t dummy2; /* not used by the GC */
    list_head link;
};

struct JSRuntime {

};

struct JSContext {
    JSGCObjectHeader header;
    JSRuntime *rt;
    list_head link;

    uint16_t binary_object_count;
    int binary_object_size;
};