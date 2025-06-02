#ifndef MY_HELPERS_H_
#define MY_HELPERS_H_

#define defer(value)                                                           \
    do {                                                                       \
        result = value;                                                        \
        goto defer;                                                            \
    } while (0)

#define unused(expr) ((void)(expr))
#define loop while (1)
#define unimplemented()                                                        \
    do {                                                                       \
        printf("Unimplemented: %s:%d\n", __FILE__, __LINE__);                  \
        exit(69);                                                              \
    } while (0)

#endif // MY_HELPERS_H_
