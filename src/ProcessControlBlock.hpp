/**
 * @file ProcessControlBlock.hpp
 * @date 2018-12-24
 * @author 倪文卿
 * @copyright 作者保留所有权利
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

/**
 * @brief 进程控制块
 */
class ProcessControlBlock
{
	private:
		const int p_id; ///< 进程 id
		const std::chrono::seconds p_required_seconds; ///< 该进程需要执行的时长
		std::chrono::seconds p_has_executed_seconds; ///< 该进程已执行的时长
		MemoryAllocator memory_allocator; ///< 该进程所享有的物理块信息
		int using_memory = -1; ///< 记录此进程当前正在访问到的页面号

	public:
		ProcessControlBlock(int p_id, std::chrono::seconds p_required_seconds, int required_memory_slice_size) :
				p_id(p_id),
				p_required_seconds(p_required_seconds),
				p_has_executed_seconds(),
				memory_allocator(required_memory_slice_size)
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
			for (int slice : p->memory_allocator.memory_required) {
				p_memory_slice << slice << "   ";
			}

			this->nana::label::caption(
					(fmt % p->p_id
							% p->p_required_seconds.count()
							% (p->p_required_seconds - p->p_has_executed_seconds).count()
							% p_memory_slice.str()
							% p->memory_allocator.print_memory(p->using_memory)
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

			// 创建一个线程，用于模拟时间片轮转进程调度算法
			std::thread t([]() {
				while (true) {
					// display_group_visit_lock 是一个互斥锁，
					// 用于限制只有一个线程对 process_display_group 对象进行写操作
					while (!display_group_visit_lock.try_lock()) {}
					auto it = process_display_group.begin();
					display_group_visit_lock.unlock();
					while (it != process_display_group.end()) {
						if (it->run_for(2s)) {
							// 如果一个进程运行结束，则 run_for 方法返回 true
							// 此时可以将该执行完毕的进程从队列中移除
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

/**
 * @param milli 每访问一个页面后休眠多少毫秒，用于控制 UI 推进速度
 */
inline void ProcessControlBlock::memory_using(std::chrono::milliseconds milli)
{
	int n = 4;

	std::vector<int> chosen_slice;
	std::mt19937 eg(std::random_device { }());
	// 从进程创建时声明使用到的页面中，随机产生 4 片接下来将访问的页面的页面号，保存到 choose_slice 中
	std::sample(memory_allocator.memory_required.cbegin(), memory_allocator.memory_required.cend(), std::back_inserter(chosen_slice), n, eg);

	for (int slice_id : chosen_slice) {
		std::cout << "需要访问： " << slice_id << " 号页面" << std::endl;
		this->using_memory = slice_id;

		auto it = std::find(memory_allocator.memory_in_using.begin(), memory_allocator.memory_in_using.end(), slice_id);
		if (it != memory_allocator.memory_in_using.end()) {
			// 欲访问的页面在内存中
			std::cout << slice_id << " 号页面在内存中，将其提升至栈顶" << std::endl;
			memory_allocator.memory_in_using.erase(it);
			memory_allocator.memory_in_using.push_back(slice_id);
		} else {
			// 欲访问的页面不在内存中
			if (memory_allocator.memory_in_using.size() == memory_allocator.max_memory_size) {
				// 如果该进程的物理块都已装满，则需淘汰掉最近最久未使用的页面
				std::cout << "淘汰 " << memory_allocator.memory_in_using.front() << " 号页面" << std::endl;
				memory_allocator.memory_in_using.pop_front();
			}
			std::cout << "置入 "  << slice_id << " 号页面" << std::endl;
			memory_allocator.memory_in_using.push_back(slice_id);
		}
		// 刷新 UI 页面，将访问的页面号、当前物理块中已存页面等信息反应到用户界面中
		ProcessGroup::refresh_group();
		std::this_thread::sleep_for(milli);
	}
	std::cout << std::endl;
}


#endif /* PROCESSCONTROLBLOCK_HPP_ */
