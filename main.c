/*
 * main.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2021
 *
 */

#include <stdio.h>
#include "cache_impl.h"

int global_timestamp = 0;

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

/* retrieve_data() is to get accessed_data.
   Invoke check_cache_data_hit().
   In case of the cache miss event, access the main memory by invoking access_memory() and increase num_cache_misses by one.
   In case of the cache hit event, increase num_cache_hits by one.
   Increase global_timestamp by one.
   Increase the num_bytes according to the data_type. byte (1 byte), halfword(2 bytes), word(4 bytes) */

int retrieve_data(void *addr, char data_type) {
    int value_returned = -1;
    
    int check_hit = check_cache_data_hit((int *)addr, data_type);
    
    if(check_hit == -1){
        value_returned = access_memory((int *)addr, data_type);
        num_cache_misses += 1;
    }
    else{
        value_returned = check_hit;
        num_cache_hits += 1;
    }
    
    global_timestamp +=1;
    
    if(data_type == 'b')
        num_bytes += 1;
    else if (data_type == 'h')
        num_bytes += 2;
    else
        num_bytes += 4;

    return value_returned;
}

/* In main function, read the input file and generate an output file.
   To make an output file, read each line in the input file using a while loop
   and get accessed_data in given (address, type) by invoking retrieve_data().
   calculate and print hit_ratio and bandwidth for each cache mechanism as regards  to cache association size. */

int main(void) {
    FILE *ifp = NULL, *ofp = NULL;
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */
    float hit_ratio;
    float bandwidth;
    
    init_memory_content();
    init_cache_content();
    
    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }
    
    
    fprintf(ofp,"[Accessed Data]\n");
    
    while(!feof(ifp)){
        fscanf(ifp, "%ld %c\n", &access_addr, &access_type);
        accessed_data = retrieve_data( &access_addr, access_type);
        fprintf(ofp, "%d %c 0x%x\n",(int)access_addr, access_type, accessed_data);
    }
    
    fprintf(ofp,"— — — — — — — — — — — — — — — — — — —\n");
    if(DEFAULT_CACHE_ASSOC == 1) fprintf(ofp,"[Direct mapped cache performance]\n");
    else if(DEFAULT_CACHE_ASSOC == 2) fprintf(ofp,"[2-way set associative cache performance]\n");
    else fprintf(ofp,"[fully associative cache performance]\n");
    
    hit_ratio = ((float)num_cache_hits / (float)(num_cache_hits + num_cache_misses));
    bandwidth = ((float)num_bytes / (float)num_access_cycles);
    fprintf(ofp,"Hit ratio = %.2f (%d/%d)\n", hit_ratio, num_cache_hits, num_cache_hits + num_cache_misses);
    fprintf(ofp,"Bandwidth = %.2f (%d/%d)", bandwidth, num_bytes, num_access_cycles);
    
    fclose(ifp);
    fclose(ofp);
    
    print_cache_entries();
    return 0;
}
