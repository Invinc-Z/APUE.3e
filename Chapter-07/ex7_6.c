#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    size_t n = 5;

    long *la = calloc(n, sizeof(long));
    void **pa = calloc(n, sizeof(void *));

    if (!la || !pa) {
        perror("calloc");
        return 1;
    }

    printf("long array:\n");
    for (size_t i = 0; i < n; i++) {
        printf("  la[%zu] = %ld\n", i, la[i]);
    }

    printf("\npointer array:\n");
    for (size_t i = 0; i < n; i++) {
        printf("  pa[%zu] = %p\n", i, pa[i]);
    }

    free(la);
    free(pa);

    return 0;
}

