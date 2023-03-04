// Auto generated with ghidra (mostly)
typedef unsigned char   undefined;

// typedef unsigned long long    GUID;
// typedef pointer32 ImageBaseOffset32;

// typedef unsigned char    bool;
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
// typedef short    wchar_t;
typedef unsigned short    word;
typedef struct JSContext JSContext, *PJSContext;

typedef struct JSValue JSValue, *PJSValue;

typedef ulonglong size_t;

typedef struct JSGCObjectHeader JSGCObjectHeader, *PJSGCObjectHeader;

typedef struct JSRuntime JSRuntime, *PJSRuntime;

typedef struct list_head list_head, *Plist_head;

typedef ushort uint16_t;

typedef struct JSShape JSShape, *PJSShape;

typedef ulonglong uint64_t;

typedef int BOOL;

typedef union JSValueUnion JSValueUnion, *PJSValueUnion;

typedef longlong int64_t;

typedef struct JSMallocFunctions JSMallocFunctions, *PJSMallocFunctions;

typedef struct JSMallocState JSMallocState, *PJSMallocState;

typedef uint uint32_t;

typedef struct JSString JSString, *PJSString;

typedef struct JSString JSAtomStruct;

typedef struct JSClass JSClass, *PJSClass;

typedef enum enum_8 {
    JS_GC_PHASE_NONE=0,
    JS_GC_PHASE_DECREF=1,
    JS_GC_PHASE_REMOVE_CYCLES=2
} enum_8;

typedef enum enum_8 JSGCPhaseEnum;

typedef ulonglong uintptr_t;

typedef struct JSStackFrame JSStackFrame, *PJSStackFrame;

typedef int (JSInterruptHandler)(struct JSRuntime *, void *);

typedef void (JSHostPromiseRejectionTracker)(struct JSContext *, struct JSValue, struct JSValue, int, void *);

typedef char * (JSModuleNormalizeFunc)(struct JSContext *, char *, char *, void *);

typedef struct JSModuleDef JSModuleDef, *PJSModuleDef;

typedef JSModuleDef * (JSModuleLoaderFunc)(struct JSContext *, char *, void *);

typedef struct JSSharedArrayBufferFunctions JSSharedArrayBufferFunctions, *PJSSharedArrayBufferFunctions;

typedef uchar uint8_t;

typedef struct JSObject JSObject, *PJSObject;

typedef struct JSShapeProperty JSShapeProperty, *PJSShapeProperty;

typedef int int32_t;

typedef struct JSRefCountHeader JSRefCountHeader, *PJSRefCountHeader;

typedef union _union_29 _union_29, *P_union_29;

typedef uint32_t JSAtom;

typedef void (JSClassFinalizer)(struct JSRuntime *, struct JSValue);

typedef void (JS_MarkFunc)(struct JSRuntime *, struct JSGCObjectHeader *);

typedef void (JSClassGCMark)(struct JSRuntime *, struct JSValue, JS_MarkFunc *);

typedef JSValue (JSClassCall)(struct JSContext *, struct JSValue, struct JSValue, int, struct JSValue *, int);

typedef struct JSClassExoticMethods JSClassExoticMethods, *PJSClassExoticMethods;

typedef struct JSPropertyDescriptor JSPropertyDescriptor, *PJSPropertyDescriptor;

typedef struct JSPropertyEnum JSPropertyEnum, *PJSPropertyEnum;

struct JSShapeProperty {
    uint32_t hash_next:26;
    uint32_t flags:6;
    JSAtom atom;
};

struct list_head {
    struct list_head * prev;
    struct list_head * next;
};

struct JSGCObjectHeader {
};

struct JSShape {
    struct JSGCObjectHeader header;
    uint8_t is_hashed;
    uint8_t has_small_array_index;
    uint32_t hash;
    uint32_t prop_hash_mask;
    int prop_size;
    int prop_count;
    int deleted_prop_count;
    struct JSShape * shape_hash_next;
    struct JSObject * proto;
    struct JSShapeProperty prop[0];
};

struct JSObject {
};

struct JSModuleDef {
};

union JSValueUnion {
    int32_t int32;
    double float64;
    void * ptr;
};

struct JSRefCountHeader {
    int ref_count;
};

struct JSValue {
    union JSValueUnion u;
    int64_t tag;
};

struct JSPropertyDescriptor {
    int flags;
    struct JSValue value;
    struct JSValue getter;
    struct JSValue setter;
};

struct JSPropertyEnum {
    int is_enumerable;
    JSAtom atom;
};

union _union_29 {
    uint8_t str8[0];
    uint16_t str16[0];
};

struct JSString {
    struct JSRefCountHeader header;
    uint32_t len:31;
    uint8_t is_wide_char:1;
    uint32_t hash:30;
    uint8_t atom_type:2;
    uint32_t hash_next;
    union _union_29 u;
};

struct JSMallocFunctions {
    void * (* js_malloc)(struct JSMallocState *, size_t);
    void (* js_free)(struct JSMallocState *, void *);
    void * (* js_realloc)(struct JSMallocState *, void *, size_t);
    size_t (* js_malloc_usable_size)(void *);
};

struct JSSharedArrayBufferFunctions {
    void * (* sab_alloc)(void *, size_t);
    void (* sab_free)(void *, void *);
    void (* sab_dup)(void *, void *);
    void * sab_opaque;
};

struct JSMallocState {
    size_t malloc_count;
    size_t malloc_size;
    size_t malloc_limit;
    void * opaque;
};

struct JSRuntime {
    struct JSMallocFunctions mf;
    struct JSMallocState malloc_state;
    char * rt_info;
    int atom_hash_size;
    int atom_count;
    int atom_size;
    int atom_count_resize;
    uint32_t * atom_hash;
    JSAtomStruct * * atom_array;
    int atom_free_index;
    int class_count;
    struct JSClass * class_array;
    struct list_head context_list;
    struct list_head gc_obj_list;
    struct list_head gc_zero_ref_count_list;
    struct list_head tmp_obj_list;
    JSGCPhaseEnum gc_phase:8;
    size_t malloc_gc_threshold;
    uintptr_t stack_size;
    uintptr_t stack_top;
    uintptr_t stack_limit;
    struct JSValue current_exception;
    BOOL in_out_of_memory:8;
    struct JSStackFrame * current_stack_frame;
    JSInterruptHandler * interrupt_handler;
    void * interrupt_opaque;
    JSHostPromiseRejectionTracker * host_promise_rejection_tracker;
    void * host_promise_rejection_tracker_opaque;
    struct list_head job_list;
    JSModuleNormalizeFunc * module_normalize_func;
    JSModuleLoaderFunc * module_loader_func;
    void * module_loader_opaque;
    BOOL can_block:8;
    struct JSSharedArrayBufferFunctions sab_funcs;
    int shape_hash_bits;
    int shape_hash_size;
    int shape_hash_count;
    struct JSShape * * shape_hash;
    void * user_opaque;
};

struct JSStackFrame {
    struct JSStackFrame * prev_frame;
    struct JSValue cur_func;
    struct JSValue * arg_buf;
    struct JSValue * var_buf;
    struct list_head var_ref_list;
    uint8_t * cur_pc;
    int arg_count;
    int js_mode;
    struct JSValue * cur_sp;
};

struct JSClass {
    uint32_t class_id;
    JSAtom class_name;
    JSClassFinalizer * finalizer;
    JSClassGCMark * gc_mark;
    JSClassCall * call;
    struct JSClassExoticMethods * exotic;
};

struct JSContext {
    struct JSGCObjectHeader header;
    struct JSRuntime * rt;
    struct list_head link;
    uint16_t binary_object_count;
    int binary_object_size;
    struct JSShape * array_shape;
    struct JSValue * class_proto;
    struct JSValue function_proto;
    struct JSValue function_ctor;
    struct JSValue array_ctor;
    struct JSValue regexp_ctor;
    struct JSValue promise_ctor;
    struct JSValue native_error_proto[8];
    struct JSValue iterator_proto;
    struct JSValue async_iterator_proto;
    struct JSValue array_proto_values;
    struct JSValue throw_type_error;
    struct JSValue eval_obj;
    struct JSValue global_obj;
    struct JSValue global_var_obj;
    uint64_t random_state;
    int interrupt_counter;
    BOOL is_error_property_enabled;
    struct list_head loaded_modules;
    JSValue (* compile_regexp)(struct JSContext *, struct JSValue, struct JSValue);
    JSValue (* eval_internal)(struct JSContext *, struct JSValue, char *, size_t, char *, int, int);
    void * user_opaque;
};

struct JSClassExoticMethods {
    int (* get_own_property)(struct JSContext *, struct JSPropertyDescriptor *, struct JSValue, JSAtom);
    int (* get_own_property_names)(struct JSContext *, struct JSPropertyEnum * *, uint32_t *, struct JSValue);
    int (* delete_property)(struct JSContext *, struct JSValue, JSAtom);
    int (* define_own_property)(struct JSContext *, struct JSValue, JSAtom, struct JSValue, struct JSValue, struct JSValue, int);
    int (* has_property)(struct JSContext *, struct JSValue, JSAtom);
    JSValue (* get_property)(struct JSContext *, struct JSValue, JSAtom, struct JSValue);
    int (* set_property)(struct JSContext *, struct JSValue, JSAtom, struct JSValue, struct JSValue, int);
};

