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
	}
	unsigned long long findChunkBlock(size_t size)
	{
		const auto last_idx = NumBitsPerBitBlock - size;
		BitBlockType bmask = (1 << size) - 1;
		unsigned long idx;
		for(int bbi=0;bbi<NumBitBlocks;++bbi)
		{
			auto bitblock = m_mask[bbi];
			while (bitblock) 
			{
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
		}
		return -1;
	}
	void freeChunkBlock(unsigned long chunk_idx, size_t size)
	{
		const unsigned long long bbi = chunk_idx / NumBitsPerBitBlock;
		const unsigned long long mask = ((1ull << size ) - 1 ) <<  chunk_idx;
		
		_ASSERT( 0 == (m_mask[bbi] & mask) );
		m_mask[bbi] |= mask;
	}
};
template <int ChunkSize, int NumChunksPerBlock>
struct ObjectPoolMultisize
{
	static const int _NumChunksPerBlock = NumChunksPerBlock;
	struct ChunkBlock 
	{
		OccupancyMask<NumChunksPerBlock> mask;
		size_t num_free_chunks, first_free_chunk;
		uint8_t *start_of_block, *end_of_block;
		ChunkBlock()
		{
			mask.clear();
			start_of_block = new uint8_t[NumChunksPerBlock * ChunkSize];
			end_of_block = start_of_block + NumChunksPerBlock * ChunkSize;
			num_free_chunks = NumChunksPerBlock;
			first_free_chunk = 0;
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
					return cb.start_of_block + ChunkSize * idx;
				}
			}
		}
		ChunkBlock & cb = *allocChunkBlock();
		const auto idx = cb.mask.findChunkBlock(num_chunks);
		_ASSERT(idx != (unsigned long long) - 1);
		cb.num_free_chunks -= num_chunks;
		return cb.start_of_block + ChunkSize * idx;
	}

	template <typename T, typename...Args>
	T* alloc(size_t size, Args...args)
	{
		uint8_t *ptr = alloc(size);
		return new(ptr) T(args...);
	}

	void free(uint8_t* ptr, size_t num_chunks)
	{
		tot_current_usage -= num_chunks;
		auto it = blocks.lower_bound(reinterpret_cast<unsigned long long>(ptr));
		ChunkBlock & cb = *it->second;
		cb.num_free_chunks += num_chunks;
		unsigned idx = unsigned( (ptr - cb.start_of_block) / ChunkSize );
		cb.mask.freeChunkBlock(idx, num_chunks);
	}

	template <typename T>
	void free(T* ptr, size_t num_chunks)
	{
		ptr->~T();
		free(reinterpret_cast<uint8_t*>(ptr), num_chunks);
	}

	void reset_stats()
	{
		max_usage = 0;
	}
	size_t get_max_usage() { return max_usage; }
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
