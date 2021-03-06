#include <stdlib.h>
#include <string.h>
#include "array.h"

struct array {
    unsigned m;
    unsigned n;
    void **as;
};

// resize the array to the given value, can be larger or smaller than the current size
// if the new size is smaller, the items at index size or higher are lost
// does nothing if the new size is not different from the old size
static bool resize(array *arr, unsigned s) {
    if (s < arr->n) {
        void **as;
        if ((as = malloc(s * sizeof(void *))) == null) {
            return false;
        }

        memcpy(as, arr->as, s * sizeof(void *));
        free(arr->as);
        arr->as = as;
        arr->m = s;
        arr->n = arr->n < s ? arr->n : s;

        return true;
    } else if (s > arr->n) {
        void **as;
        if ((as = realloc(arr->as, s * sizeof(void *))) != null) {
            arr->as = as;
            arr->m = s;

            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

optional arr_init(unsigned m) {
    optional opt;
    array *arr;
    if ((arr = malloc(sizeof(array))) == null) {
        opt.e = false;
        opt.err = malloc_fail;
        return opt;
    }

    if ((arr->as = malloc(m * sizeof(void *))) == null) {
        free(arr);
        opt.e = false;
        opt.err = malloc_fail;
        return opt;
    }

    arr->m = m;
    arr->n = 0;

    opt.e = true;
    opt.val = arr;
    return opt;
}

optional arr_copy(array *arr, unsigned m) {
    optional opt;

    array *new_arr;
    if ((new_arr = malloc(sizeof(array))) == null) {
        opt.e = false;
        opt.err = malloc_fail;
        return opt;
    }

    if ((new_arr->as = malloc(m * sizeof(void *))) == null) {
        free(new_arr);
        opt.e = false;
        opt.err = malloc_fail;
        return opt;
    }

    new_arr->m = m;

    if (arr == null) {
        new_arr->n = 0;
        memset(new_arr->as, 0, m * sizeof(void *));
    } else {
        new_arr->n = arr->n < m ? arr->n : m;
        memcpy(new_arr->as, arr->as, new_arr->n * sizeof(void *));
    }

    opt.e = true;
    opt.val = arr;
    return opt;
}

void arr_free(array *arr) {
    free(arr->as);
    free(arr);
}

optional arr_peek(array *arr) {
    optional opt;
    if (arr->n > 0) {
        opt.e = true;
        opt.val = arr->as[arr->n - 1];
    } else {
        opt.e = false;
        opt.err = out_of_bounds;
    }

    return opt;
}

optional arr_pop(array *arr) {
    optional opt;
    if (arr->n > 0) {
        arr->n--;
        opt.e = true;
        opt.val = arr->as[arr->n];
        arr->as[arr->n] = null;

        if (arr->n < (arr->m / 4)) {
            resize(arr, arr->m / 2);
        }
    } else {
        opt.e = false;
        opt.err = out_of_bounds;
    }

    return opt;
}

bool arr_push(array *arr, void *a) {
    if (arr->n == arr->m && !resize(arr, arr->m * 2)) {
        return false;
    }

    arr->as[arr->n++] = a;

    return true;
}

bool arr_concat(array *dest, array *src) {
    if (dest->n + src->n >= dest->m) {
        unsigned new_size = dest->m + src->m;
        void **new_as;
        if ((new_as = malloc(new_size * sizeof(void *))) == null) {
            return false;
        }
        memcpy(new_as, dest->as, dest->n * sizeof(void *));
        free(dest->as);
        dest->as = new_as;
    }

    memcpy(&dest->as[dest->n], src->as, src->n * sizeof(void *));
    dest->n += src->n;
    return true;
}

void arr_foreach(array *arr, void *(*func)(void *)) {
    int i;
    for (i = 0; i < arr->n; i++) {
        arr->as[i] = (func)(arr->as[i]);
    }
}

int arr_reduce(array *arr, optional (*func)(void *)) {
    int i, c = 0;
    for (i = 0; i < arr->n; i++) {
        optional opt = func(arr->as[i]);
        if (opt.e) {
            arr->as[c++] = opt.val;
        }
    }

    if (c < arr->n) {
        arr->n = (unsigned) c;
        memset(&arr->as[arr->n], 0, (arr->m - arr->n) * sizeof(void *));
    }

    return c;
}

unsigned arr_size(array *arr) {
    return arr->n;
}

optional arr_get(array *arr, unsigned i) {
    optional opt;
    if (i < arr->n) {
        opt.e = true;
        opt.val = &arr->as[i];
    } else {
        opt.e = false;
        opt.err = out_of_bounds;
    }
    return opt;
}

bool arr_insert(array *arr, void *a, unsigned i) {
    return false;
}
