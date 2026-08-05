

#define XFREE(p)              \
    do {                      \
        free(p);              \
        (p) = NULL;           \
    } while (0)



#define XFCLOSE(fp)            \
    do {                      \
        if ((fp) != NULL) {   \
            fclose(fp);       \
            (fp) = NULL;      \
        }                     \
    } while (0)



