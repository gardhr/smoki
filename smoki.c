/*

*/


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

void rotate(block* binary)
{
 int last;
 size_t length = binary->length;
 for(size_t index = 0; index < length; ++index)
 {
  int bit = block_get(int, *binary, index);
  block_set(int, *binary, index, last);
  last = bit;
 }
 block_set(int, *binary, 0, last);
}

void transform(block* identity, block signature)
{
 rotate(identity);
 xor(signature, *identity, identity);
}

double score(block lhs, block rhs)
{
 size_t matches = 0;
 size_t count = lhs.length;
 for(size_t index = 0; index < count; ++index)
  if(block_get(int, lhs, index) == block_get(int, rhs, index))
   ++matches;
 return (double)matches / count;
}

double randomize_bracket(block* buffer, block comparison, double low, double high)
{
 for(;;)
 {
  randomize(buffer);
  double mark = score(*buffer, comparison);
  if(mark > low && mark < high)
   return mark;
 }
 return 0;
}

void test(char* text)
{
 puts(text);
 block send_key = text_to_binary(text);
 size_t length = send_key.length;
 if(length & 1)
  pad(&send_key, ++length);
 block receive_key = block_clone(int, send_key);
 puts("Initial shared state:");
 binp(send_key);
 block hints = block_make(int, length);
 block more_similar = block_make(int, length);
 block less_similar = block_make(int, length);
 unsigned char message = randlong();
 const size_t bits = CHAR_BIT;
 showl(message);
 long bitbox = message;
 long msb = 1;
 long received = 0;
 size_t count = bits;
 double average = 0.5;
 double roam = 0.05;
 while(count--)
 { 
// SEND

/* Create two equally-biased random distributions */
  randomize_bracket(&more_similar, send_key, average, average + roam);
  randomize_bracket(&less_similar, send_key, average - roam, average);
  showd(score(send_key, less_similar));
  showd(score(send_key, more_similar));
/* If the bit is 1, send the first sequence */
  int bit = bitbox & 1;
  bitbox >>= 1;
  if(bit)
  {
   transform(&send_key, more_similar);
   xor(more_similar, hints, &hints);    
  }
  else
  {
   transform(&send_key, less_similar);
   xor(less_similar, hints, &hints); 
  }

// RECEIVE

/* If the bit is 1, send the first sequence */
 //double grade = score();
  showl(bit);
  puts("Shared state:");
  binp(send_key);
  if(bit)
   received |= msb;
  msb <<= 1;
  puts("Hints:");
  binp(hints);
 }
 showl(received);
 showb(received == message);
 block_free(send_key);
 block_free(hints); 
 block_free(more_similar);
 block_free(less_similar);
}

int main(int argc, char** argv)
{
 puts("- Smoki -");
 if(argc == 1)
  test(*argv);
 else while(*(++argv))
  test(*argv);
 puts("Done!");
 return 0;
}
