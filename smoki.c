#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include "block.h"

#define showc(ch) printf("%s : `%c`\n", #ch, ch)
#define shows(text) printf("%s : `%s`\n", #text, text)
#define showl(value) printf("%s : %zu\n", #value, (size_t)value)
#define showd(value) printf("%s : %g\n", #value, (double)value)
#define showb(value) printf("%s : %s\n", #value, value ? "true" : "false")

/* No error checks done here. Failure is not an option! */
void urandom(void* buffer, size_t size)
{
 static FILE* stream = NULL;
 if(!stream)
  stream = fopen("/dev/urandom", "rb"); 
 fread(buffer, 1, size, stream);
}

long randlong(void)
{
 long word;
 urandom(&word, sizeof(long));
 return word; 
}

int toss(void)
{
 enum { size = 512 };
 typedef unsigned long ulong;
 static ulong buffer[size];
 static ulong index = size;
 static ulong bits = CHAR_BIT;
 if(++bits >= CHAR_BIT)
 {
  bits = 0;
  if(++index >= size)
  {
   index = 0;
   urandom(buffer, size);
  }
 }
 int flip = buffer[index] & 1;
 buffer[index] >>= 1;
 return flip;
}

block text_to_binary(char* text)
{
 block result = { 0 };
 for(;;)
 {
  unsigned char data = *text++;
  if(data == 0)
   break;
  size_t length = CHAR_BIT;
  while(length--)
  {
   block_push(int, result, data & 1);
   data >>= 1;
  }
 }
 return result;
}

void pad(block* result, size_t width)
{
 size_t length = result->length;
 if(length > width)
  block_resize(int, *result, width);
 else while(length++ <= width)
  block_push(int, *result, toss());
}

void binp(block binary)
{
 size_t length = binary.length;
 for(size_t index = 0; index < length; ++index)
  printf("%d", block_get(int, binary, index));
 puts("");
}

void randomize(block* binary)
{
 size_t length = binary->length;
 for(size_t index = 0; index < length; ++index)
  block_set(int, *binary, index, toss());
}

void randomize_biased(block* binary)
{
 size_t length = binary->length;
 for(size_t index = 0; index < length; ++index)
 {
  if(!(randlong() % 3))
  {
   int state = block_get(int, *binary, index);
   block_set(int, *binary, index, !state);
  }  
 }
}

void increment(block* binary)
{
 int carry = 1;
 size_t length = binary->length;
 for(size_t index = 0; index < length; ++index)
 {
  int send = block_get(int, *binary, index);
  int sum = send + carry;
  carry = (sum > 1);
  sum &= 1;
  block_set(int, *binary, index, sum);
 }
}

void xor(block identity, block signature, block* result)
{
 size_t length = identity.length; 
 block_resize(int, *result, length);
 for(size_t index = 0; index < length; ++index)
 {
  int mapping = (block_get(int, signature, index) != block_get(int, identity, index));
  block_set(int, *result, index, mapping);
 }
}

void transform(block identity, block signature, block* result)
{
 xor(identity, signature, result);
 increment(result); 
}

void test(char* text)
{
 block input = text_to_binary(text);
 size_t length = input.length;
 if(length & 1)
  pad(&input, ++length);
 puts("Initial shared state:");
 binp(input);
 block hints = block_clone(int, input);
 block biased = block_make(int, length);
 block random = block_make(int, length);
 unsigned char message = randlong();
 const size_t bits = CHAR_BIT;
 showl(message);
 long bitbox = message;
 long msb = 1;
 long received = 0;
 size_t count = bits;
 while(count--)
 {  
  block_copy(int, input, biased);
  randomize_biased(&biased);
  randomize(&random);
  int send = bitbox & 1;
  bitbox >>= 1;
  if(send)
   transform(input, biased, &input);
  else
   transform(input, random, &input); 
  showl(send);
  puts("Shared state:");
  binp(input); 
  if(send)
   received |= msb;
  msb <<= 1;
  xor(input, hints, &hints);
  puts("Hints:");
  binp(hints);
 }
 puts("Final shared state:");
 binp(input);
 showl(received);
 block_free(input);
 block_free(hints);  
 block_free(biased); 
 block_free(random); 
}

int main(int argc, char** argv)
{
 srand(time(0));
 puts("- Smoki -"); 
 if(argc == 1)
  test(*argv);
 else while(*(++argv))
  test(*argv);
 puts("Done!"); 
 return 0; 
}
