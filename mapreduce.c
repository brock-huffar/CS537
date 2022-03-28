#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mapreduce.h"
#include "hashmap.h"
#include <errno.h>
#include <pthread.h>

struct kv {
    char* key;
    char* value;
};

struct kv_list {
    struct kv** elements;
    size_t num_elements;
    size_t size;
    pthread_mutex_t listLock;
};

struct kv_list kvl;
size_t kvl_counter;

void init_kv_list(size_t size) {
    kvl.elements = (struct kv**) malloc(size * sizeof(struct kv*));
    kvl.num_elements = 0;
    kvl.size = size;
}

void add_to_list(struct kv* elt) {
    if (kvl.num_elements == kvl.size) {
	kvl.size *= 2;
	kvl.elements = realloc(kvl.elements, kvl.size * sizeof(struct kv*));
    }
    kvl.elements[kvl.num_elements++] = elt;
}

void MR_Emit(char *key, char *value) {
    pthread_mutex_lock(&(kvl.listLock));
    struct kv *elt = (struct kv*) malloc(sizeof(struct kv));
    if (elt == NULL) {
	printf("Malloc error! %s\n", strerror(errno));
	exit(1);
    }
    elt->key = strdup(key);
    elt->value = strdup(value);
    add_to_list(elt);
    pthread_mutex_unlock(&(kvl.listLock));
    return;
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}



void MR_Run(int argc, char *argv[], 
	    Mapper map, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition) 
    {   
    init_kv_list(10);
    pthread_t threads[num_mappers];
    void * retvals[num_mappers];
    int count;
    for (count = 0; count < num_mappers; ++count) {
        if (pthread_create(&threads[count], NULL, (void*)map, argv[count+1]) != 0) {
            fprintf(stderr, "Error: Cannot create mapping thread # %d\n", count);
            break;
        }
    }
    for (int i = 0; i < count; ++i) {
        if (pthread_join(threads[i], &retvals[i]) != 0) {
          fprintf(stderr, "Error: Cannot join mapping thread # %d\n", i);
        }
    }


    
}
