#pragma once
#include "memory.h"

namespace foundation
{
	template <int BLOCK_SIZE, int BLOCK_COUNT>
	class BlockAllocator : public Allocator
	{
	public:
		BlockAllocator( Allocator & backing = memory_globals::default_allocator() );
		virtual ~BlockAllocator();

		virtual void *allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN);

		virtual void deallocate(void *);

		virtual uint32_t allocated_size(void *) {return SIZE_NOT_TRACKED;}
		virtual uint32_t total_allocated() {return SIZE_NOT_TRACKED;}

		void compact( int max_blocks = 1024 );

	private:
		uint32_t _allocated_table[BLOCK_COUNT];
	};

	template< typename T >
	struct BlockPointer
	{
		BlockPointer(void*) {}

		T* operator

	private:

	};


	typedef BlockAllocator<4096, 1024>	Block4KAllocator4M;
	typedef BlockAllocator<4096, int BLOCK_COUNT> Block4KAllocator;

	template <int BLOCK_SIZE, int BLOCK_COUNT>
	BlockAllocator<BLOCK_SIZE, BLOCK_COUNT>::BlockAllocator( Allocator &backing = memory_globals::default_allocator() )
		: _backing(backing)
		, _
	{

	}
}