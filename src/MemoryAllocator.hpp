/**
 * @file MemoryAllocator.hpp
 * @date 2018-12-25
 * @author 倪文卿
 * @copyright 作者保留所有权利
 */

#ifndef MEMORYALLOCATOR_HPP_
#define MEMORYALLOCATOR_HPP_

#include <deque>
#include <unordered_set>

class ProcessControlBlock;

class MemoryAllocator
{
	public:
		inline static int next_slice_id = 0;

		constexpr static int max_memory_size = 5;

		std::deque<int> memory_in_using;

		std::unordered_set<int> memory_required;

		MemoryAllocator(int n)
		{
			this->memory_required.reserve(n);
			for (int i = 0; i < n; ++i) {
				this->memory_required.insert(next_slice_id++);
			}
		}

		std::string print_memory(int highlight_slice)
		{
			std::ostringstream out;
			int count = 0;
			for (int existed_slice : memory_in_using) {
				if (existed_slice == highlight_slice) {
					out << "[";
				}
				out << existed_slice;
				if (existed_slice == highlight_slice) {
					out << "]";
				}
				out << "   ";
				++count;
			}
			if (count < max_memory_size) {
				for (int i = 0; i < max_memory_size - count; ++i) {
					out << "[ ]   ";
				}
			}
			return out.str();
		}

};

#endif /* MEMORYALLOCATOR_HPP_ */
