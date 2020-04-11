#ifndef BLOCK_H_INCLUSION_GUARD
#define BLOCK_H_INCLUSION_GUARD

#include "stdlib.h"
#include "memory.h"

struct block
{
 void* data; 
 size_t length;
 size_t capacity;
};

typedef struct block block;

#define block_data(TYPE, buffer) ((TYPE*)(buffer).data)

#define block_index(TYPE, buffer, index) (block_data(TYPE, buffer) + (index))

#define block_get(TYPE, buffer, index) (*block_index(TYPE, buffer, index))

#define block_set(TYPE, buffer, index, value) (block_get(TYPE, buffer, index) = (value))

#define block_length(buffer) (buffer).length

#define block_capacity(buffer) (buffer).capacity

block block_make_(size_t length, size_t size)
{
 block buffer = { 0 };
 buffer.data = malloc(length * size);
 buffer.length = buffer.capacity = length;
 return buffer;
}

#define block_make(TYPE, length) block_make_(length, sizeof(TYPE))

#define block_shrink(buffer) ((buffer).length = 0)

#define block_reset(buffer) ((buffer).data = NULL, (buffer).length = (buffer).capacity = 0)

#define block_clear_range(TYPE, buffer, start, end)\
 do\
 {\
  size_t begin = (start);\
  size_t length = (end) - begin;\
  memset(block_index(TYPE, (buffer), begin), 0, length * sizeof(TYPE));\
 }\
 while(0)

#define block_clear(TYPE, buffer)\
 do\
 {\
  block_clear_range(TYPE, buffer, 0, (buffer).length);\
 }\
 while(0)

#define block_resize(TYPE, buffer, size)\
 do\
 {\
  size_t request = (size);\
  size_t capacity = (buffer).capacity;\
  if(request > capacity)\
  {\
   size_t expanded = capacity;\
   if(expanded == 0)\
    ++expanded;\
   while(expanded < request)\
    expanded <<= 1;\
   (buffer).data = realloc((buffer).data, expanded * sizeof(TYPE));\
   (buffer).capacity = expanded;\
  }\
  size_t length = (buffer).length;\
  if(request > length)\
   memset(block_index(TYPE, buffer, length), 0, (request - length) * sizeof(TYPE)); \
  (buffer).length = request;\
 }\
 while(0)

#define block_insert(TYPE, buffer, index, value)\
 do\
 {\
  size_t position = (index);\
  size_t end = (buffer).length;\
  if(position >= end)\
   block_resize(TYPE, buffer, position + 1);\
  block_set(TYPE, buffer, position, (value));\
 }\
 while(0)

#define block_empty(buffer) ((buffer).length == 0)

#define block_push(TYPE, buffer, value) block_insert(TYPE, buffer, (buffer).length, value)

#define block_pop(TYPE, buffer) block_resize(TYPE, buffer, (buffer).length - 1)

#define block_front(TYPE, buffer) block_get(TYPE, buffer, 0)

#define block_bottom(TYPE, buffer) block_front(TYPE, buffer)

#define block_back(TYPE, buffer) block_get(TYPE, buffer, (buffer).length - 1)

#define block_top(TYPE, buffer) block_back(TYPE, buffer)

#define block_append(TYPE, buffer, other)\
 do\
 {\
  size_t length = (buffer).length;\
  size_t additional = (other).length; \
  block_resize(TYPE, (buffer), length + additional);\
  memcpy(block_index(TYPE, (buffer), length), (other).data, additional * sizeof(TYPE));\
 }\
 while(0)

#define block_copy(TYPE, buffer, source) (buffer).length = 0; block_append(TYPE, (buffer), (source))

block block_clone_(block other, size_t size)
{
 block buffer = { 0 };
 size_t length = other.length;
 size_t actual = length * size;
 buffer.data = malloc(actual);
 memcpy(buffer.data, other.data, actual);
 buffer.length = buffer.capacity = length;
 return buffer;
}

#define block_clone(TYPE, buffer) block_clone_((buffer), sizeof(TYPE))

#define block_swap(buffer, other)\
 do\
 {\
  block stored = (buffer);\
  (buffer) = (other);\
  (other) = stored;\
 }\
 while(0)

#define block_free(buffer) (free((buffer).data), block_reset(buffer))

#define block_for_each(TYPE, buffer, action)\
 do\
 {\
  typedef void (*callback)(const void*, size_t);\
  callback step = (callback)action;\
  for(size_t index = 0, length = (buffer).length; index < length; ++index)\
   step(block_index(TYPE, buffer, index), index);\
 }\
 while(0)

typedef int (*block_compare)(const void*, const void*);

#define block_sort(TYPE, buffer, compare)\
 qsort((buffer).data, (buffer).length, sizeof(TYPE), (block_compare)compare)

#define block_compare_up(lhs, rhs) (lhs < rhs ? -1 : lhs > rhs ? 1 : 0)

#define block_compare_down(lhs, rhs) block_compare_up(rhs, ls)

#define return_block_compare_up(TYPE, left, right) TYPE lhs = *left, rhs = *right; return block_compare_up(lhs, rhs)

int chars_ascending(char* left, char* right) { return_block_compare_up(char, left, right); }
int bytes_ascending(unsigned char* left, unsigned char* right) { return_block_compare_up(unsigned char, left, right); }
int shorts_ascending(short* left, short* right) { return_block_compare_up(short, left, right); }
int ushorts_ascending(unsigned short* left, unsigned short* right) { return_block_compare_up(unsigned short, left, right); }
int ints_ascending(int* left, int* right) { return_block_compare_up(int, left, right); }
int uints_ascending(unsigned int* left, unsigned int* right) { return_block_compare_up(unsigned int, left, right); }
int longs_ascending(long* left, long* right) { return_block_compare_up(long, left, right); }
int ulongs_ascending(unsigned long* left, unsigned long* right) { return_block_compare_up(unsigned long, left, right); }
int floats_ascending(float* left, int* right) { return_block_compare_up(float, left, right); }
int doubles_ascending(double* left, double* right) { return_block_compare_up(double, left, right); }
int literals_ascending(char** left, char** right) 
{ 
 char* lhs = *left; 
 char* rhs = *right;
 if(lhs == NULL)
  return -1;
 if(rhs == NULL)
   return 1;
 return strcmp(lhs, rhs);
}

int chars_descending(char* left, char* right) { return_block_compare_up(char, right, left); }
int bytes_descending(unsigned char* left, unsigned char* right) { return_block_compare_up(unsigned char, right, left); }
int shorts_descending(short* left, short* right) { return_block_compare_up(short, right, left); }
int ushorts_descending(unsigned short* left, unsigned short* right) { return_block_compare_up(unsigned short, right, left); }
int ints_descending(int* left, int* right) { return_block_compare_up(int, right, left); }
int uints_descending(unsigned int* left, unsigned int* right) { return_block_compare_up(unsigned int, right, left); }
int longs_descending(long* left, long* right) { return_block_compare_up(long, right, left); }
int ulongs_descending(unsigned long* left, unsigned long* right) { return_block_compare_up(unsigned long, right, left); }
int floats_descending(float* left, int* right) { return_block_compare_up(float, right, left); }
int doubles_descending(double* left, double* right) { return_block_compare_up(double, right, left); }
int literals_descending(char** left, char** right) 
{
 char* lhs = *left; 
 char* rhs = *right;
 if(lhs == NULL)
  return 1;
 if(rhs == NULL)
   return -1;
 return strcmp(rhs, lhs);
}

#define block_search(TYPE, buffer, key, compare)\
 ((TYPE*)bsearch(&key, (buffer).data, (buffer).length, sizeof(TYPE), (block_compare)compare))

#endif

