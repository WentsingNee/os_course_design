/**
 * @file main.cpp
 * @date 2018-12-24
 * @author 倪文卿
 * @copyright 作者保留所有权利
 */

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include "add_new_process.hpp"
#include "GridLayout.hpp"
#include <boost/optional.hpp>


int main()
{
	nana::form fm(nana::API::make_center(1000, 600));
	boost::optional<GridLayout> grid_layout;
	nana::button bt_add_new_process(fm, "创建新进程");

	ProcessGroup::fm = &fm;
	ProcessGroup::grid_layout = &grid_layout;
	ProcessGroup::bt_add_new_process = &bt_add_new_process;

	fm.bgcolor(nana::color(255, 255, 255));

	bt_add_new_process.events().click([&](){
		add_new_process([&](std::unique_ptr<ProcessControlBlock> && p){
			ProcessGroup::add_new_process(std::move(p));
			ProcessGroup::refresh_group();
		});
	});

	grid_layout.emplace(fm, bt_add_new_process);
	grid_layout->collocate();

	fm.show();

	ProcessGroup::handle();
	nana::exec();
}
