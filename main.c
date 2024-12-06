#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef void* (*fun)(void*);

typedef struct {
    pthread_t id;
    unsigned bucket_size;
    int* bucket;
} thread_t;

void* sorter(thread_t* thread);

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        printf("faltan argumentos\n");
        return 1;
    }

    const char* filename = argv[1];
    unsigned bucket_size = atoi(argv[2]);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("el archivo '%s' no abre :c\n", filename);
        return 1;
    }

    unsigned vector_size;
    if (fscanf(file, "%u", &vector_size) != 1) {
        printf("no hay %d entradas en el archivo, o el formato es incorrecto\n", vector_size);
        return 1;
    }

    int* vector = malloc(vector_size * sizeof(int));
    if (vector == NULL) {
        printf("no hay memoria\n");
        return 1;
    }

    for (int i = 0; i < vector_size; i++) {
        if (fscanf(file, "%d", vector + i) != 1) {
            printf("no hay %d entradas en el archivo, o el formato es incorrecto\n", vector_size);
            return 1;
        }
    }

    int vector_min, vector_max;
    vector_min = vector_max = vector[0];
    for (int i = 0; i < vector_size; i++) {
        if (vector[i] > vector_max)
            vector_max = vector[i];
        if (vector[i] < vector_min)
            vector_min = vector[i];
    }

    int remain = (vector_max - vector_min) % bucket_size;
    int nthreads = (vector_max - vector_min + remain) / bucket_size;
    thread_t* threads = malloc(nthreads * sizeof(thread_t));

    unsigned bucket_index = 0;

    for (int i = 0; i < nthreads; i++) {
        int* bucket = vector + bucket_index;
        int bucket_min = vector_min + bucket_size * i;
        int bucket_max = bucket_min + bucket_size;
        int vec_bucket_size = 0;

        for (int j = bucket_index; j < vector_size; j++) {
            if (bucket_min <= vector[j] && vector[j] < bucket_max) {
                int tmp = vector[j];
                vector[j] = bucket[vec_bucket_size];
                bucket[vec_bucket_size] = tmp;
                vec_bucket_size++;
            }
        }

        threads[i].bucket = bucket;
        threads[i].bucket_size = vec_bucket_size;
        bucket_index += vec_bucket_size;
        
        pthread_create(&threads[i].id, NULL, (fun) sorter, &threads[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i].id, NULL);
    }

    for (int i = 0; i < vector_size; i++) {
        if ((i + 1) % 10 == 0) printf("\n");
        printf("%6d, ", vector[i]);
    }
    printf("\n");

    free(vector);
    free(threads);
    fclose(file);
    return 0;
}

void* sorter(thread_t* thread) {
    if (thread->bucket_size < 2) return NULL;

    int pi = thread->bucket_size - 1;
    int pivot = thread->bucket[pi];
    int i = -1;

    for (int j = 0; j < thread->bucket_size - 1; j++) {
        if (thread->bucket[j] < pivot) {
            i++;
            int tmp = thread->bucket[i];
            thread->bucket[i] = thread->bucket[j];
            thread->bucket[j] = tmp;
        }
    }

    int tmp = thread->bucket[i + 1];
    thread->bucket[i + 1] = thread->bucket[pi];
    thread->bucket[pi] = tmp;

    int api = i + 1;

    thread_t left = { .bucket_size = api, .bucket = thread->bucket };
    thread_t right = { .bucket_size = thread->bucket_size - api - 1, .bucket = thread->bucket + api + 1 };
    sorter(&left);
    sorter(&right);
}
