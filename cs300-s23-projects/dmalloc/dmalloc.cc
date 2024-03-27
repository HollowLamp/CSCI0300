#define DMALLOC_DISABLE 1
#include "dmalloc.hh"
#include <cassert>
#include <cstring>
#include <map>

std::map<void*, int> leakch;
std::map<void*, bool> freed;
static dmalloc_stats gls;
/*
leakch，存储所有ptr与line的键值对
freed，存储所有ptr与freed的键值对
(int*)ptr
ptr-1指向size
ptr-2指向是否被释放过 1是没有
ptr-3指向是否被分配过 1是有
ptr-4指向约定秘密值
*/
/**
 * dmalloc(sz,file,line)
 *      malloc() wrapper. Dynamically allocate the requested amount `sz` of memory and 
 *      return a pointer to it 
 * 
 * @arg size_t sz : the amount of memory requested 
 * @arg const char *file : a string containing the filename from which dmalloc was called 
 * @arg long line : the line number from which dmalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dmalloc(size_t sz, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    char* ptr = (char*)base_malloc(sz + 17);
    if(!ptr || sz > ((unsigned int)(-1)>>1)){
        gls.nfail++;
        gls.fail_size += sz;
        return nullptr;
    }
    void* ret = ptr + 16;
    gls.nactive++;
    gls.ntotal++;
    gls.total_size += sz;
    gls.active_size += sz;
    int*  data = (int*)ptr;
    *(data + 3) = sz;       //allocated size
    *(data + 2) = 1;        //freed
    *(data + 1) = 1;        //allocated
    leakch[ret] = line;
    *(data) = 0xfc;         //secret value
    *((char*)ret + sz) = *(data);
    
    if(gls.heap_max < (uintptr_t) ((char*)ret + sz)){
        gls.heap_max = (uintptr_t) ((char*)ret + sz);     
    }
    if(!gls.heap_min || gls.heap_min > (uintptr_t) ret ){
        gls.heap_min = (uintptr_t) ret;
    }

    return ret;
}

/**
 * dfree(ptr, file, line)
 *      free() wrapper. Release the block of heap memory pointed to by `ptr`. This should 
 *      be a pointer that was previously allocated on the heap. If `ptr` is a nullptr do nothing. 
 * 
 * @arg void *ptr : a pointer to the heap 
 * @arg const char *file : a string containing the filename from which dfree was called 
 * @arg long line : the line number from which dfree was called 
 */
void dfree(void* ptr, const char* file, long line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    // Your code here.
    if(!ptr){
        return;
    }
    if((uintptr_t) ptr > gls.heap_max || (uintptr_t) ptr < gls.heap_min){
        fprintf(stderr, "MEMORY BUG???: invalid free of pointer %p, not in heap\n", ptr);    
        abort();
    }
    int* data = (int*)ptr;
    if(!(*(data - 3))){
        fprintf(stderr, "MEMORY BUG: test???.cc:%ld: invalid free of pointer %p, not allocated\n", line, ptr);
        for(auto p = leakch.cbegin(); p != leakch.cend(); ++p) {
            int* key = (int*)p->first;
            char* step = (char*)p->first;
            if((uintptr_t)ptr > (uintptr_t)step && (uintptr_t)ptr < (uintptr_t)(step + *(key - 1))){
                fprintf(stderr, "test???.cc:%d: %p is %ld bytes inside a %d byte region allocated here", leakch[key], ptr, ((uintptr_t)ptr - (uintptr_t)step), *(key - 1));
            }
        }
        abort();
    }
    int allocated = leakch.count(ptr);
    if(!allocated){
        fprintf(stderr, "MEMORY BUG: test???.cc:%ld: invalid free of pointer %p, not allocated\n", line, ptr);
        abort();
    }
    if(!(*(data - 2)) || freed[ptr]){
        fprintf(stderr, "MEMORY BUG???: invalid free of pointer %p, double free\n", ptr);
        abort();
    }
    
    if(*((char*)ptr + *(data - 1))!= (char)*(data - 4)){
        fprintf(stderr, "MEMORY BUG???: detected wild write during free of pointer %p\n", ptr);
        abort();
    }
    gls.nactive--;
    *(data - 2) = 0;
    freed[ptr] = 1;
    gls.active_size -= *(data - 1);
    base_free(ptr);
}

/**
 * dcalloc(nmemb, sz, file, line)
 *      calloc() wrapper. Dynamically allocate enough memory to store an array of `nmemb` 
 *      number of elements with wach element being `sz` bytes. The memory should be initialized 
 *      to zero  
 * 
 * @arg size_t nmemb : the number of items that space is requested for
 * @arg size_t sz : the size in bytes of the items that space is requested for
 * @arg const char *file : a string containing the filename from which dcalloc was called 
 * @arg long line : the line number from which dcalloc was called 
 * 
 * @return a pointer to the heap where the memory was reserved
 */
void* dcalloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).
    if(nmemb > ((unsigned int)(-1)>>1) || sz > ((unsigned int)(-1)>>1)){
        gls.nfail++;
        return nullptr;
    }
    void* ptr = dmalloc(nmemb * sz, file, line);
    if (ptr) {
        memset(ptr, 0, nmemb * sz);
    }
    return ptr;
}

/**
 * get_statistics(stats)
 *      fill a dmalloc_stats pointer with the current memory statistics  
 * 
 * @arg dmalloc_stats *stats : a pointer to the the dmalloc_stats struct we want to fill
 */
void get_statistics(dmalloc_stats* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(dmalloc_stats));

    // Your code here.
    memcpy(stats, &gls, sizeof(dmalloc_stats));
}

/**
 * print_statistics()
 *      print the current memory statistics to stdout       
 */
void print_statistics() {
    dmalloc_stats stats;
    get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/**  
 * print_leak_report()
 *      Print a report of all currently-active allocated blocks of dynamic
 *      memory.
 */
void print_leak_report() {
    // Your code here.
    for(auto p = leakch.cbegin(); p != leakch.cend(); ++p) {
        int* key = (int*)p->first;
        if(*(key - 2)){
            printf("LEAK CHECK: test???.cc:%d: allocated object %p with size %d\n",leakch[key], key, *(key - 1));
        }
    }
}
