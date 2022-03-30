#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mapreduce.h"
#include "hashmap.h"
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <sys/stat.h>
#define mapCount 100
Mapper map;
Reducer reducer;
Partitioner partitioner;
int reducer_count;
pthread_mutex_t fileCountMutex;
int mappedCounter = 0;
int fileCount;
struct files* fileNames;
struct files {
	char *name;
};

typedef struct vNode {
    char* value;
    struct vNode* next;
} vNode;
typedef struct kNode {
    char* key;
    vNode* head;
    struct kNode* next;
} kNode;
typedef struct kEntry {
    kNode* head;
    pthread_mutex_t lock;
} kEntry;
typedef struct pEntry{
    kEntry map[mapCount];
    int key_num;
    pthread_mutex_t lock;
    kNode* sorted;
    int curr_visit;
} pEntry;
pEntry pList[64];

int compareStr(const void* str1, const void* str2) {
    char* key1 = ((kNode*)str1)->key;
    char* key2 = ((kNode*)str2)->key;
    if (strcmp(key1,key2) == 0) {
        return 0;
    }
    else {
        return -1;
    }
}

int compareFiles(const void* p1, const void* p2) {
	struct files* file1 = (struct files *) p1;
	struct files* file2 = (struct files *) p2;
	struct stat st1, st2;
	stat(file1->name, &st1);
	stat(file2->name, &st2);
	long int size1 = st1.st_size;
	long int size2 = st2.st_size;
	return (size1 - size2);
}

char* get_func(char *key, int partitionNum) {
    kNode* keyArray = pList[partitionNum].sorted;
    char* value;
    while(4) {
        int curr = pList[partitionNum].curr_visit;
        if (strcmp(keyArray[curr].key, key) == 0){
            if (keyArray[curr].head == NULL)
                return NULL;
            vNode* temp = keyArray[curr].head->next;
            value = keyArray[curr].head->value;
            keyArray[curr].head = temp;
            return value;
        } else {
            pList[partitionNum].curr_visit++;
            continue;
        }
        return NULL;
    }
}

void* reducerThreads(void* arg1){
    int partitionNum = *(int*)arg1;
    if(pList[partitionNum].key_num == 0) return NULL;
    pList[partitionNum].sorted = malloc(sizeof(kNode)*pList[partitionNum].key_num);
    int count = 0;
    int i = 0;
    while (i < mapCount) {
        kNode *curr = pList[partitionNum].map[i].head;
        if (curr == NULL)  {
            i++;
            continue;
        }
        while (curr != NULL) {
            pList[partitionNum].sorted[count] = *curr;
            count++;
            curr = curr -> next;
        }
        i++;
    }
    qsort(pList[partitionNum].sorted, pList[partitionNum].key_num, sizeof(kNode), compareStr);
    for (int x = 0; x < pList[partitionNum].key_num; x++) {
        char *key = pList[partitionNum].sorted[x].key;
        (*reducer)(key,get_func,partitionNum);
    }
    //Free data from the heap
    for (int i = 0; i < mapCount; i++) {
        kNode *curr = pList[partitionNum].map[i].head;
        if (curr == NULL) continue;
        while (curr != NULL){
            free(curr->key);
            curr->key = NULL;
            vNode* vCurr = curr->head;
            while (vCurr != NULL){
                free(vCurr->value);
                vCurr->value = NULL;
                vNode* temp = vCurr -> next;
                free(vCurr);
                vCurr = temp;
            }
            vCurr = NULL;
            kNode* tempK = curr -> next;
            free(curr);
            curr = tempK;
        }
        curr = NULL;
    }
    free(pList[partitionNum].sorted);
    pList[partitionNum].sorted = NULL;
    free(arg1); 
    arg1 = NULL;
    return NULL;
}

void MR_Emit(char *key, char *value) {
    unsigned long partition_number = (*partitioner)(key, reducer_count);
    unsigned long map_number = MR_DefaultHashPartition(key, mapCount);
    pthread_mutex_lock(&pList[partition_number].map[map_number].lock);
    kNode* temp = pList[partition_number].map[map_number].head;
    while(temp != NULL){
        if (strcmp(temp->key, key) == 0){
            break;
        }
        else {

        
        temp = temp->next;
        }
    }
    //create a value node
    vNode* new_v = malloc(sizeof(vNode));
    if (new_v == NULL) {
        perror("malloc");
        pthread_mutex_unlock(&pList[partition_number].map[map_number].lock);
        return; // fail
    }
    else { 
    new_v->value = malloc(sizeof(char)*20);
    if (new_v->value == NULL)
        printf("ERROR");
    strcpy(new_v->value, value);
    new_v->next = NULL;
    //if there is no existing node for same key
    if (temp == NULL){
        kNode *new_key = malloc(sizeof(kNode));
        if (new_key == NULL) {
            perror("malloc");
            pthread_mutex_unlock(&pList[partition_number].map[map_number].lock);
            return; // fail
        }
        new_key->head = new_v;
        new_key->next = pList[partition_number].map[map_number].head;
        pList[partition_number].map[map_number].head = new_key;
        
        new_key->key = malloc(sizeof(char)*20);
        if (new_key->key == NULL)
            printf("ERROR");
        strcpy(new_key->key, key);
        pthread_mutex_lock(&pList[partition_number].lock);
        pList[partition_number].key_num++;
        pthread_mutex_unlock(&pList[partition_number].lock);

    } else {
        //if there is existing node for same key
        new_v->next = temp->head;
        temp->head = new_v;
    }
    }

    pthread_mutex_unlock(&pList[partition_number].map[map_number].lock);

}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

void * mappers(void * arg) {
    while(1) {
        if(mappedCounter >= fileCount) {
            return NULL;
        }
        char * fileName;
        pthread_mutex_lock(&fileCountMutex);
        fileName = fileNames[mappedCounter].name;
        mappedCounter++;
        pthread_mutex_unlock(&fileCountMutex);
        (*map)(fileName);
    }
}



void MR_Run(int argc, char *argv[], 
	    Mapper map1, int num_mappers, 
	    Reducer reduce, int num_reducers, 
	    Partitioner partition) 
    {   
    fileNames = malloc((argc-1) * sizeof(struct files));
	for(int i = 0; i <argc-1; i++) {
		fileNames[i].name = malloc(sizeof(char) *  (strlen(argv[i+1])+1));
		strcpy(fileNames[i].name, argv[i+1]);
	}
	// Sorting files by size, shortest first
	qsort(&fileNames[0], argc-1, sizeof(struct files), compareFiles);
   // initialize(argc,fileNames, num_reducers,map,reduce,partition);
    int rc = pthread_mutex_init(&fileCountMutex, NULL);
    assert(rc == 0);
    mappedCounter = 0;
    partitioner = partition;
    reducer_count = num_reducers;
    fileCount = argc-1;
    map = map1;
    reducer = reduce;
 
    for (int y = 0; y < reducer_count; y++){
        pthread_mutex_init(&pList[y].lock, NULL);
        pList[y].key_num = 0;
        pList[y].curr_visit = 0;
        pList[y].sorted = NULL;
        for (int j = 0; j < mapCount; j++){
            pList[y].map[j].head = NULL;
            pthread_mutex_init(&pList[y].map[j].lock, NULL);
        } 
    }

    // create map threads
    pthread_t mapthreads[num_mappers];
    if (num_mappers == 0) {
        return; //fail
    }
    if (num_reducers == 0) {
        return; //fail
    }
    int i = 0;
    while (i < num_mappers) {
        pthread_create(&mapthreads[i], NULL, mappers, NULL);
        i++;
    }
    int j = 0;
    while (j < num_mappers) {
        pthread_join(mapthreads[j], NULL);
        j++;
    }
    // create reduce threads
    pthread_t reducethreads[num_reducers];
    int k = 0;
    while (k < num_reducers) {
         void* arg = malloc(sizeof(int));
        *(int*)arg = k;
        pthread_create(&reducethreads[k], NULL, reducerThreads, arg);
        k++;
    }
    int x = 0;
    while (x < num_reducers) {
        pthread_join(reducethreads[x], NULL);
        x++;
    }
}
