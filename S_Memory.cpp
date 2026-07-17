#include "Local.h"

#define MEM_LOG "^3[Memory]^7 "

static size_t	s_memBytesAllocated;
static int		s_memAllocCount;

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
