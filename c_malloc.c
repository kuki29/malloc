#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define u8 uint8_t
#define u16 uint16_t
#define STACK_SIZE 32
#define HEAP_SIZE STACK_SIZE * 4
#define HEADER 4
#define MAX_ENTITIES 40

static u16 IN_USE;

typedef struct virtual_memory
{
	u8 stack[STACK_SIZE];
	char** unmapped;
	u8 heap[HEAP_SIZE];

	struct
	{
		char** data;
		char** bss;
		char** text;
	}data_t;
}virtual_memory_t;

typedef struct entity
{
	u8* ptr;
	u16 size;
}entity_t;

entity_t LIST[MAX_ENTITIES];

void LOG()
{
	printf("OUR LIST\n");
	for (unsigned i = 0; i < IN_USE; i++)
	{
		printf("Data + header.[%p]. Memory of our heap free:[%u]\n", LIST[i].ptr, LIST[i].size);
	}
	printf("Entities in use:[%d]\n", (sizeof(LIST) / sizeof(LIST[0]) - IN_USE));
}

entity_t* new_entity(size_t size)
{
	if (LIST[0].ptr == NULL && LIST[0].size == 0)
	{
		static virtual_memory_t vm;
		LIST[0].ptr = vm.heap;
		LIST[0].size = HEAP_SIZE;
		IN_USE++;
		LOG();
	}

	entity_t* best = LIST;

	for (unsigned i = 0; i < IN_USE; i++)
	{
		if (LIST[0].size >= size && LIST[i].size < best->size)
		{
			best = &LIST[i];
		}
	}

	return best;
}

void* c_malloc(size_t size)
{
	//todo: make heap responsive
	assert(size <= HEAP_SIZE);

	size += HEADER;

	entity_t* e = new_entity(size);

	u8* start = e->ptr;
	u8* user_ptr = start + HEADER;

	*start = size;

	e->ptr += size;
	e->size -= size;

	assert(e->size >= 0);
	LOG();

	return user_ptr;
}

void c_free(void* ptr)
{
	u8* start = (u8*)ptr - HEADER;

	entity_t *prev = 0, *next = 0;
	for (size_t i = 0; i < IN_USE; i++)
	{
		if (LIST[i].ptr + LIST[i].size == start)
			prev = &LIST[i];
		if (start + *start == LIST[i].ptr)
			next = &LIST[i];
	}
	if (next)
	{
		next->ptr = start;
		next->size += *start;
		*start = next->size;
		IN_USE--;
	}
	if (prev)
	{
		prev->size += *start;
		if (next)
		{
			next->ptr = LIST[IN_USE].ptr;
			next->size = LIST[IN_USE].size;
		}
		IN_USE--;
	}
	if (!(prev || next))
	{
		LIST[IN_USE].ptr = start;
		LIST[IN_USE].size = *start;
	}
	IN_USE++;
	LOG();
}

void test()
{
	typedef struct foo
	{
		int a;
		int b;
	}foo_t;

	foo_t* foo;
	char* bar;
	int* buzz;

	foo = c_malloc(sizeof(foo_t));
	bar = c_malloc(5);
	buzz = c_malloc(sizeof(int));

	foo->a = 5;
	foo->a = 10;

	strcpy(bar, "bar");
	memcpy(buzz, &foo->a, sizeof(int));

	printf("Address: [%p], data: [%d] [%d]\n", foo, foo->a, foo->b);
	printf("Address: [%p], data: [%s]\n", bar, bar);
	printf("Address: [%p], data: [%d]\n", buzz, *buzz);

	c_free(foo);
	c_free(bar);
	c_free(buzz);
}

int main(int argc, char** argv)
{
	test();
	getchar();
	return 0;
}
