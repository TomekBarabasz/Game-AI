#pragma once
#include <bitset>
#include <array>
#include <intrin.h>
#include <map>

template <int NumBlocks>
struct OccupancyMask
{
	using BitBlockType = long long;
	static constexpr unsigned NumBitsPerBitBlock = sizeof(BitBlockType) * 8;
	static constexpr unsigned NumBitBlocks = (NumBlocks + NumBitsPerBitBlock -1 ) / NumBitsPerBitBlock;
	std::array<BitBlockType, NumBitBlocks> m_mask;
	unsigned first_free_chunk;

	OccupancyMask()
	{
		clear();
	}

	void clear() 
	{
		memset(&m_mask[0], 0xFF, sizeof(BitBlockType) * NumBitBlocks);
		const int num_bits_in_last_block = NumBlocks % NumBitsPerBitBlock;
		if (num_bits_in_last_block != 0) {
			m_mask[NumBitBlocks - 1] = (BitBlockType(1) << num_bits_in_last_block) - 1;
		}
		first_free_chunk = 0;
	}
	unsigned long long findChunkBlock(size_t size)
	{
		const auto last_idx = NumBitsPerBitBlock - size;
		BitBlockType bmask = (1 << size) - 1;
		unsigned long idx;
		bool nonepty_found = false;
		for(auto bbi = first_free_chunk;bbi<NumBitBlocks;++bbi)
		//for(auto bbi = 0;bbi<NumBitBlocks;++bbi)
		{
			auto bitblock = m_mask[bbi];
			while (bitblock) 
			{
				nonepty_found = true;
				_BitScanForward64(&idx, bitblock);
				if (idx > last_idx) break;
				BitBlockType bmask_shifted = bmask << idx;
				if ( (bitblock &bmask_shifted) == bmask_shifted )
				{
					m_mask[bbi] &= ~bmask_shifted;
					return bbi * NumBitsPerBitBlock + idx;
				}
				bitblock &= bitblock - 1;
			}
			if (!nonepty_found) {
				++first_free_chunk;
			}
		}
		return -1;
	}
	void freeChunkBlock(unsigned long chunk_idx, size_t size)
	{
		const unsigned long bbi = chunk_idx / NumBitsPerBitBlock;
		const unsigned long long mask = ((1ull << size ) - 1 ) <<  chunk_idx;
		
		_ASSERT( 0 == (m_mask[bbi] & mask) );
		m_mask[bbi] |= mask;
		if (first_free_chunk > bbi) {
			first_free_chunk = bbi;
		}
	}
};
template <int CHUNK_SIZE, int NUM_CHUNKS_PER_BLOCK>
struct ObjectPoolMultisize
{
	static const int NumChunksPerBlock = NUM_CHUNKS_PER_BLOCK;
	static const int ChunkSize = CHUNK_SIZE;
	struct ChunkBlock 
	{
		OccupancyMask<NUM_CHUNKS_PER_BLOCK> mask;
		size_t num_free_chunks;
		uint8_t *start_of_block, *end_of_block;
		ChunkBlock()
		{
			mask.clear();
			start_of_block = new uint8_t[NUM_CHUNKS_PER_BLOCK * CHUNK_SIZE];
			end_of_block = start_of_block + NUM_CHUNKS_PER_BLOCK * CHUNK_SIZE;
			num_free_chunks = NUM_CHUNKS_PER_BLOCK;
		}
		bool operator<(const ChunkBlock& other) const
		{
			return this->end_of_block < other.end_of_block;
		}
		~ChunkBlock()
		{
			delete[] start_of_block;
		}
	};
	
	ObjectPoolMultisize()
	{
	}
	~ObjectPoolMultisize()
	{
		for (auto it : blocks) {
			delete it.second;
		}
	}
	
	uint8_t* alloc(size_t num_chunks)
	{
		tot_current_usage += num_chunks;
		max_usage = __max(max_usage, tot_current_usage);

		for (auto it: blocks) {
			ChunkBlock & cb = *it.second;
			if (cb.num_free_chunks >= num_chunks) {
				const auto idx = cb.mask.findChunkBlock(num_chunks);
				if (idx != (unsigned long long)-1)	{
					cb.num_free_chunks -= num_chunks;
					return cb.start_of_block + CHUNK_SIZE * idx;
				}
			}
		}
		ChunkBlock & cb = *allocChunkBlock();
		const auto idx = cb.mask.findChunkBlock(num_chunks);
		_ASSERT(idx != (unsigned long long) - 1);
		cb.num_free_chunks -= num_chunks;
		return cb.start_of_block + CHUNK_SIZE * idx;
	}

	template <typename T, typename...Args>
	T* alloc(size_t num_chunks, Args...args)
	{
		uint8_t *ptr = alloc(num_chunks);
		return new(ptr) T(args...);
	}

	void free(uint8_t* ptr, size_t num_chunks)
	{
		tot_current_usage -= num_chunks;
		auto it = blocks.lower_bound(reinterpret_cast<unsigned long long>(ptr));
		ChunkBlock & cb = *it->second;
		cb.num_free_chunks += num_chunks;
		unsigned idx = unsigned( (ptr - cb.start_of_block) / CHUNK_SIZE );
		cb.mask.freeChunkBlock(idx, num_chunks);
#ifdef _DEBUG
		memset(ptr, 0xcb, num_chunks * CHUNK_SIZE);
#endif
	}

	template <typename T>
	void free(T* ptr, size_t num_chunks)
	{
		ptr->~T();
		free(reinterpret_cast<uint8_t*>(ptr), num_chunks);
	}

	void free_all()
	{
		for (auto it : blocks)
		{
			auto & cb = *(it.second);
			cb.mask.clear();
			cb.num_free_chunks = 0;
		}
		tot_current_usage = 0;
	}
	void reset_stats()			{ max_usage = 0;			}
	size_t get_max_usage()		{ return max_usage;			}
	size_t get_current_usage()	{ return tot_current_usage; }

protected:
	ChunkBlock* allocChunkBlock()
	{
		auto block = new ChunkBlock;
		blocks[reinterpret_cast<unsigned long long>(block->end_of_block)] = block;
		return block;
	}
	std::map<unsigned long long, ChunkBlock*> blocks;
	size_t max_usage = 0, tot_current_usage = 0;
};
