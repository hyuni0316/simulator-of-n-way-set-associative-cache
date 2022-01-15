/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2021
 *
 */


#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

/* get the global variables in other files */

extern int global_timestamp;

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

// data in cache with size (# of sets X # of associations),
// 4X1 for direct, 2X2 for 2-way, 1X4 for fully
cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
// data in memory with size of 128 words
int memory_array[DEFAULT_MEMORY_SIZE_WORD];

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = {0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef};
    unsigned char sample_downward[16] = {0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010};
    int index, i=0, j=1, gap = 1;
    
    for (index=0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;
        
        if (i == 0 && j == i+gap)
            j = i + (++gap);
            
        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}   

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        for (j=0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k=0; k<DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\n");
        }
        printf("\n");
    }
}
/* find_entry_index_in_set() is to find the entry index in set for copying to cache.
   If the set has only 1 entry, entry_index is 0.
   When the same tag exists, check the tag of all entries, and if the same tag exists entry_index is i.
   When the same tag doesn't exist, check whether any entry has any empty cache space or not by checking 'valid',
    and if the entry's valid is 0, then entry_index will be i.
   Search over all entries to find the least recently used entry by checking 'timestamp'.
   Set the LRU_timestamp as the timestamp of first entry and set the min_index as 0.
   Check all entries, and find timestamp which has minimum value.
   In each case, change the timestamp.
   Return the cache index for copying from memory. */
int find_entry_index_in_set(void *addr, int cache_index) {
    int entry_index, i, LRU_timestamp, min_index;
    int byte_address = *((int *)addr);
    int block_address = byte_address/DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int Tag = block_address / (4/DEFAULT_CACHE_ASSOC);
    
   if(DEFAULT_CACHE_ASSOC == 1){
        entry_index = 0;
        cache_entry_t *pEntry = &cache_array[cache_index][0];
        pEntry->timestamp = global_timestamp;
    }
    else{
        for(i=0; i<DEFAULT_CACHE_ASSOC; i++){
            cache_entry_t *pEntry = &cache_array[cache_index][i];
            if(pEntry->tag == Tag){
                entry_index = i;
                pEntry->timestamp = global_timestamp;
                return entry_index;
            }
        }
        for(i=0; i<DEFAULT_CACHE_ASSOC; i++){
            cache_entry_t *pEntry = &cache_array[cache_index][i];
            if(pEntry->valid == 0){
                entry_index = i;
                pEntry->timestamp = global_timestamp;
                return entry_index;
            }
        }
       
        cache_entry_t *pEntry = &cache_array[cache_index][0];
        LRU_timestamp = pEntry->timestamp;
        min_index = 0;
        for(i=1; i<DEFAULT_CACHE_ASSOC; i++){
            cache_entry_t *pEntry = &cache_array[cache_index][i];
            if(LRU_timestamp > pEntry->timestamp ){
                LRU_timestamp = pEntry->timestamp;
                min_index = i;
            }
        }
        entry_index = min_index;
        cache_array[cache_index][min_index].timestamp = global_timestamp;
    }
    return entry_index;
}

/* check_cache_data_hit() is to return the data in cache.
   Add this cache access cycle to global access cycle
   Check all entries in a set and if entry has non-zero valid, the first_val will be 1.
   If there is no data in cache, data is missed and return -1.
   Otherwise, check all entries in a set and if the same tag exists, data is hit and entry_index is i.
   If the same tag doesn't exist, data is missed and return -1.
   In hit case, change the timestamp,
   and return the accessed data with a suitable type(b, h, or w). Return -1 for unknown type */
int check_cache_data_hit(void *addr, char type) {
    int i, first_val=0, compare_tag=0, entry_index=0;
    int byte_address = *((int *)addr);
    int block_address = byte_address/DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int byte_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int cache_index = block_address % (4/DEFAULT_CACHE_ASSOC);
    int Tag = block_address / (4/DEFAULT_CACHE_ASSOC);
    
    num_access_cycles += CACHE_ACCESS_CYCLE;

    for(i=0; i<DEFAULT_CACHE_ASSOC; i++){
        cache_entry_t *pEntry = &cache_array[cache_index][i];
        if(pEntry->valid != 0) {
            first_val = 1;
        }
    }
    if(first_val != 1){
        return -1;
    }
    else{
        for(i=0; i<DEFAULT_CACHE_ASSOC; i++){
            cache_entry_t *pEntry = &cache_array[cache_index][i];
            if(pEntry->tag == Tag){
                entry_index = i;
                compare_tag = 1;
                break;
            }
        }
        if(compare_tag != 1){
            return -1;
        }
    }
    
    /* In hit case */
    cache_entry_t *pEntry = &cache_array[cache_index][entry_index];
    pEntry->timestamp = global_timestamp;
    
    if( type == 'b'){
        return pEntry->data[byte_offset];
    }
    else if( type == 'h'){
        int data = pEntry->data[byte_offset] & 0xffff00ff;
        int data1 = (pEntry->data[byte_offset+1]<<8) & 0xffffff00;
        return data|data1;
    }
    else if( type == 'w'){
        int data = pEntry->data[byte_offset] & 0x000000ff;
        int data1 = (pEntry->data[byte_offset+1]<<8) & 0x0000ff00;
        int data2 = (pEntry->data[byte_offset+2]<<16) & 0x00ff0000;
        int data3 = (pEntry->data[byte_offset+3]<<24) & 0xff000000;
        return data|data1|data2|data3;
    }
    else{
        return -1;
    }
}

/* access_memory() is to return the data in main memory.
   Get the entry index by invoking find_entry_index_in_set() for copying to the cache.
   Change the valid and tag of the entry.
   Add this main memory access cycle to global access cycle.
   The value of the memory_array[word_index] and memory_array[word_index+1] are stored in mem1 and mem2 respectively.
   Fetch the data from the main memory and copy them to the cache.
   8 bytes of data are divided into 1 byte and each stored by using bitwise operators.
   Return the accessed data with a suitable type (b, h, or w). Rreturn -1 for unknown type */
int access_memory(void *addr, char type) {
    int byte_address = *((int *)addr);
    int block_address = byte_address/DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int byte_offset = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int cache_index = block_address % (4/DEFAULT_CACHE_ASSOC);
    int word_index = block_address*DEFAULT_CACHE_BLOCK_SIZE_BYTE/WORD_SIZE_BYTE;
    int Tag = block_address / (4/DEFAULT_CACHE_ASSOC);
    int entry_index, mem1, mem2;
        
    entry_index = find_entry_index_in_set((int *)addr, cache_index);
    cache_entry_t *pEntry = &cache_array[cache_index][entry_index];
    pEntry->valid = 1;
    pEntry->tag = Tag;
    
    num_access_cycles += MEMORY_ACCESS_CYCLE;
    
    mem1 = memory_array[word_index];
    mem2 = memory_array[word_index+1];

    pEntry->data[0] = mem1 & 0xff;
    pEntry->data[1] = (mem1 & 0xff00) >> 8;
    pEntry->data[2] = (mem1 & 0xff0000) >> 16;
    pEntry->data[3] = (mem1 & 0xff000000) >> 24;
    pEntry->data[4] = mem2 & 0xff;
    pEntry->data[5] = (mem2 & 0xff00) >> 8;
    pEntry->data[6] = (mem2 & 0xff0000) >> 16;
    pEntry->data[7] = (mem2 & 0xff000000) >> 24;
    
    if( type == 'b'){
        return pEntry->data[byte_offset];
    }
    else if( type == 'h'){
        int data = pEntry->data[byte_offset] & 0xffff00ff;
        int data1 = (pEntry->data[byte_offset+1]<<8) & 0xffffff00;
        return data|data1;
    }
    else if( type == 'w'){
        int data = pEntry->data[byte_offset] & 0x000000ff;
        int data1 = (pEntry->data[byte_offset+1]<<8) & 0x0000ff00;
        int data2 = (pEntry->data[byte_offset+2]<<16) & 0x00ff0000;
        int data3 = (pEntry->data[byte_offset+3]<<24) & 0xff000000;
        return data|data1|data2|data3; 
    }
    else
    {
        return -1;
    }
}
