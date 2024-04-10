# simulator of n-way set associative cache

### implement a simulator of n-way set associative cache mechanism
### 1. Direct mapped cache (n=1)
### 2. 2-way set associative cache (n=2)
### 3. Fully associative cache

#### Report of the porject: [link](https://drive.google.com/file/d/1KFn-uupywD1hrArZKPxOZhSJI0oXks4k/view?usp=sharing)


< code analysis >


**Overall flow:**
-  Open input file in read mode and output file in write mode.
- Initialize contents of a main memory and a cache memory by invoking init_memory_content() and init_cache_content(). 
 Read each line from input file and get accessed data by invoking retrieve_data(). 
 In hit case, get data by accessing directly from the cache. 
 In miss case, get data by accessing the main memory and copying to the cache. 
   ( invoke access_memory() )
 To judge hit case, search the entries of the set and find the entry which has the   same tag. 
 To judge miss case, search the entries of the set and then find the all entries is  empty, or find that same tag doesn’t exist in the set.
 If miss case occurs, we have to find the entry index which is least recently used by comparing timestamp. ( invoke find_entry_index_in_set()  )
 Print the accessed data into output file.
 Increment global_timestamp by 1. 
 Print the hit ratio and the bandwidth.
   
**main.c**
- retrieve_data() : This function is to get accessed data. First, we can know whether it is the case of the cache miss event or not by invoking check_cache_data_hit( ). If check_hit is –1, it is miss event and then access the main memory by invoking access_memory(). Otherwise, it is the case of the cache hit event. Next, increase global_timestamp by one and increase the num_bytes according to the data_type. Finally, function will return the accessed data.
main() : In main function, read the input file and generate an output file. To make an output file, read each line in the input file using a while loop, and get data in given (address, type) by invoking retrieve_data(). Also calculate and print hit_ratio and bandwidth for each cache mechanism as regards  to cache association size.

**cache.c**
First get the global variables in other files, and declare the cache_array and the memory_array. init_memory_content() is to initialize contents of the main memory, and init_cache_content() is to initialize contents of the cache memory. print_cache_entries() is to print all the cache entries.
- find_entry_index_in_set() : This function is to find the entry index in set for copying to cache. If the set has only 1 entry, entry_index is 0 and update timestamp. Otherwise, check the tag of all entries and find out whether the same tag exists or not. If the same tag exists it is a hit case and then entry_index is the index of entry which has the same tag, and update timestamp. 
  When the same tag doesn't exist, check if there exists any empty cache space by checking valid. If any entry's valid is 0, then entry_index will be the index of entry which has the 0 valid, and update timestamp. 
  Finally, If it doesn't fit all the cases above, search over all entries to find the least recently used entry by checking 'timestamp'. Check all entries, and find timestamp which has minimum value. Entry_index will be the index of entry which has the minimum value of timestamp, and update the timestamp.
- check_cache_data_hit() : This function is to return the data in cache. First, add this cache access cycle to global access cycle. Next, check all entries in a set and if there is no data in cache, data is missed and return –1. 
  Otherwise, check all entries in a set and find out whether the same tag exists or not. If the same tag exists, data is hit and entry_index is the index of entry which has the same tag. 
  If the same tag doesn't exist, data is missed and return –1. 
  In hit case, update the timestamp and return the accessed data with a suitable type (b, h, or w). ‘b’ type will return 1 byte data, ‘h’ type will return 2 byte data, and ‘w’ type will return 4 byte data. In each case, we can use bitwise operators to shift of combine data.
- access_memory() : This function is to return the data in main memory. Get the entry index by invoking find_entry_index_in_set() for copying to the cache. Change the valid and tag of the entry. Add this main memory access cycle to global access cycle. 
  To store 8 byte of data, we have to divide into mem1 and mem2 which will have 4byte data each. The value of the memory_array[word_index] and memory_array[word_index+1] are stored in mem1 and mem2 respectively. Fetch the data from the main memory and copy them to the cache. 8 bytes of data are divided into 1 byte and each stored by using bitwise operators( use bitwise AND operation and shift right operation because of little endianness ). 
  Return the accessed data with a suitable type (b, h, or w) by using bitwise operators( use bitwise AND operation and shift left operation to combine data ). Return –1 for unknown type. 
