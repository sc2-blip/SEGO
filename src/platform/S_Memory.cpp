#include "Local.h"

#define MEM_LOG "^3[Memory]^7 "

static size_t	s_memBytesAllocated;
static int		s_memAllocCount;

struct S_MemHeader 
{
	size_t size;
};

void S_MemInit( void )
{ // init memory
	s_memBytesAllocated = 0;
	s_memAllocCount		= 0;

	Com_Printf(MEM_LOG "memory initialized\n" MEM_LOG "s_memBytesAllocated = %zu\n" MEM_LOG "s_memAllocCount = %d\n", 
		s_memBytesAllocated,
		s_memAllocCount
	);
}

void* S_Malloc( size_t size )
{ // allocate memory
	S_MemHeader *header = ( S_MemHeader* )malloc( sizeof( S_MemHeader ) + size );

	if ( !header )
	{
		// we have a big problem
		Com_Error( MEM_LOG "S_Malloc: failed on allocation of %zu bytes", size );
	}

	header->size = size; // store size in the header

	s_memBytesAllocated += size; // add and increment
	s_memAllocCount++;

	return ( void* )( header + 1 ); 

}

void* S_ReAlloc( void *ptr, size_t newSize )
{
	if ( !ptr )
		return S_Malloc( newSize );

	// if newSize is 0 then free
	if ( !newSize )
	{
		S_Free( ptr );
		return NULL;
	}

	// go back to header
	S_MemHeader *oldHeader = ( ( S_MemHeader* ) ptr ) - 1;
	
	// Store old size because it wont exist here in a short few moments
	size_t oldSize = oldHeader->size;

	S_MemHeader *newHeader = ( S_MemHeader* )realloc( oldHeader, sizeof( S_MemHeader ) + newSize );
	
	if ( !newHeader )
	{ // if reallocation fails...
    	Com_Error( MEM_LOG "S_ReAlloc: failed on reallocation of %zu bytes", newSize );
	}

	newHeader->size = newSize;
	s_memBytesAllocated -= oldSize;
	s_memBytesAllocated += newHeader->size;

	// return user pointer (header + 1):
	return ( void* )( newHeader + 1 );
}

void S_Free( void* ptr ) 
{ // free memory
	if ( !ptr )
		return; // no go

	S_MemHeader *header = (( S_MemHeader* )ptr) - 1; // walk back up ptr & find our header to free

	s_memBytesAllocated -= header->size; // remove size & decrement
	s_memAllocCount--; 

	free(header);
}

void S_MemInfo( void )
{ // see memory
	Com_Printf(
		MEM_LOG "s_memBytesAllocated = %zu\n"
		MEM_LOG "s_memAllocCount = %i\n",
		s_memBytesAllocated,
		s_memAllocCount
	);
}

void S_MemShutdown( void )
{
	if ( s_memAllocCount != 0 || s_memBytesAllocated != 0 )
	{
		Com_Printf( 
			MEM_LOG "leak: %i allocations, %zu bytes still live\n",
			s_memAllocCount,
			s_memBytesAllocated
		);
	} 
	else
	{
		Com_Printf ( MEM_LOG "clean, no leaks, ready for shut down\n");
	}
}
