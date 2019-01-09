/*
 * ProcessControlBlock.hpp
 *
 *  Created on: 2018年12月24日
 *      Author: peter
 */

#ifndef PROCESSCONTROLBLOCK_HPP_
#define PROCESSCONTROLBLOCK_HPP_

#include <chrono>
#include <thread>
#include <mutex>
#include <random>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/group.hpp>
#include <boost/format.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include "GridLayout.hpp"
#include "MemoryAllocator.hpp"

class ProcessControlBlock
{
	private:
		const int p_id; ///< 进程 id
		const std::chrono::seconds p_required_seconds; ///< 该进程需要执行的时长
		std::chrono::seconds p_has_executed_seconds; ///< 该进程已执行的时长
		MemoryAllocator ma;
		int using_memory = -1;

	public:
		ProcessControlBlock(int p_id, std::chrono::seconds p_required_seconds, int required_memory_slice_size) :
				p_id(p_id),
				p_required_seconds(p_required_seconds),
				p_has_executed_seconds(),
				ma(required_memory_slice_size)
		{
		}

		void memory_using(std::chrono::milliseconds milli);

		friend class ProcessDisplay;
		friend class ProcessGroup;
};



class ProcessDisplay : public nana::label
{
	private:
		std::unique_ptr<ProcessControlBlock> p;

	public:
		ProcessDisplay(nana::window parent, std::unique_ptr<ProcessControlBlock> && p) :
				nana::label(parent), p(std::move(p))
		{
			this->nana::label::bgcolor(nana::color(180, 180, 220));
			this->text_align(nana::align::left, nana::align_v::top);
//			this->nana::label::typeface(nana::paint::font("*", 10));
		}

		void refresh()
		{
			boost::format fmt(
					"p_id: %d\n"
					"p_require: %d (s)\n"
					"p_remain: %d (s)\n"
					"p_memory_allocated: %s\n"
					"p_memory_using: %s"
			);
			std::ostringstream p_memory_slice;
			for (int slice : p->ma.memory_required) {
				p_memory_slice << slice << "   ";
			}

			this->nana::label::caption(
					(fmt % p->p_id
							% p->p_required_seconds.count()
							% (p->p_required_seconds - p->p_has_executed_seconds).count()
							% p_memory_slice.str()
							% p->ma.print_memory(p->using_memory)
					).str()
			);
		}

		ProcessControlBlock& native()
		{
			return *p;
		}

		bool run_for(std::chrono::seconds sec)
		{
			if (native().p_has_executed_seconds >= native().p_required_seconds) {
				return true;
			}
			this->nana::label::bgcolor(nana::color(180, 220, 180));
			native().memory_using(sec);
			++(native().p_has_executed_seconds);
			if (native().p_has_executed_seconds >= native().p_required_seconds) {
				this->nana::label::bgcolor(nana::color(220, 220, 220));
				return true;
			} else {
				this->nana::label::bgcolor(nana::color(180, 180, 220));
				return false;
			}
		}

};


class ProcessGroup
{
	public:
		inline static nana::form * fm;
		inline static boost::optional<GridLayout> * grid_layout;
		inline static nana::button * bt_add_new_process;

		inline static boost::ptr_list<ProcessDisplay> process_display_group;
		inline static std::mutex display_group_visit_lock;

		static void add_new_process(std::unique_ptr<ProcessControlBlock> && p)
		{
			{
				std::lock_guard lk(display_group_visit_lock);
				process_display_group.push_back(new ProcessDisplay(*fm, std::move(p)));
			}
			refresh_group();
		}

		static void refresh_group()
		{
			(*grid_layout).emplace(*fm, *bt_add_new_process);
			{
				std::lock_guard lk(display_group_visit_lock);
				size_t i = 0;
				auto it = process_display_group.begin();
				while (i < 9 && it != process_display_group.end()) {
					auto & ele = *it;
					ele.refresh();
					(*grid_layout)->add(i, ele);
					++i;
					++it;
				}
			}
			(*grid_layout)->collocate();
		}

		static void handle()
		{
			using namespace std::literals::chrono_literals;
			std::thread t([]() {
				while (true) {
					while (!display_group_visit_lock.try_lock()) {}
					auto it = process_display_group.begin();
					display_group_visit_lock.unlock();
					while (it != process_display_group.end()) {
						auto & ref = *it;
						if (ref.run_for(2s)) {
							std::lock_guard lk(display_group_visit_lock);
							process_display_group.erase(it++);
						} else {
							std::lock_guard lk(display_group_visit_lock);
							++it;
						}
						refresh_group();
					}
				}
			});
			t.detach();
		}

};


inline void ProcessControlBlock::memory_using(std::chrono::milliseconds milli)
{
	int n = 4;

	std::vector<int> choosen_slice;
	std::mt19937 eg(std::random_device { }());
	std::sample(ma.memory_required.cbegin(), ma.memory_required.cend(), std::back_inserter(choosen_slice), n, eg);

	for (int slice_id : choosen_slice) {
		std::cout << "using " << slice_id << std::endl;
		this->using_memory = slice_id;
		bool has_existed = false;
		for (auto it = ma.memory_in_using.begin(); it != ma.memory_in_using.end(); ++it) {
			int existed_slice = *it;
			if (existed_slice == slice_id) {
				has_existed = true;
				ma.memory_in_using.erase(it);
				ma.memory_in_using.push_back(existed_slice);
				break;
			}
		}
		if (has_existed == false) {
			if (ma.memory_in_using.size() == ma.max_memory_size) {
				std::cout << "淘汰" << ma.memory_in_using.front() << std::endl;
				ma.memory_in_using.pop_front();
			}
			ma.memory_in_using.push_back(slice_id);
		}
		ProcessGroup::refresh_group();
		std::this_thread::sleep_for(milli);
	}
	std::cout << std::endl;
}

#endif /* PROCESSCONTROLBLOCK_HPP_ */
