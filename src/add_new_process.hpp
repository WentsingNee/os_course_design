/**
 * @file add_new_process.hpp
 * @date 2018-12-24
 * @author 倪文卿
 * @copyright 作者保留所有权利
 */

#ifndef ADD_NEW_PROCESS_HPP_
#define ADD_NEW_PROCESS_HPP_

#include <iostream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>

#include "ProcessControlBlock.hpp"

template <typename Callback>
void add_new_process(Callback && callback)
{
	nana::form fm(nana::API::make_center(500, 400));
	fm.caption("添加新进程");

	nana::place layout(fm);
	layout.div("vert"
			"<height=5%>"
			"<<width=10><lb_p_id><width=10><tx_p_id><width=10>>"
			"<height=5%>"
			"<<width=10><lb_p_execute_time><width=10><tx_p_execute_seconds><width=10>>"
			"<height=10%>"
			"<<width=10><lb_p_memory_slice><width=10><tx_p_memory_slice><width=10>>"
			"<height=10%>"
			"<<><bt_confirm><> height=50>"
			"<height=5%>");

	nana::label lb_p_id(fm, "进程 id");
	nana::label lb_p_execute_time(fm, "执行时间 (s)");
	nana::label lb_p_memory_slice(fm, "内存大小");

	layout["lb_p_id"] << lb_p_id;
	layout["lb_p_execute_time"] << lb_p_execute_time;
	layout["lb_p_memory_slice"] << lb_p_memory_slice;

	lb_p_id.typeface(nana::paint::font("*", 15));
	lb_p_execute_time.typeface(nana::paint::font("*", 15));
	lb_p_memory_slice.typeface(nana::paint::font("*", 15));

	lb_p_id.text_align(nana::align::left, nana::align_v::center);
	lb_p_execute_time.text_align(nana::align::left, nana::align_v::center);
	lb_p_memory_slice.text_align(nana::align::left, nana::align_v::center);

	nana::textbox tx_p_id(fm);
	nana::textbox tx_p_execute_seconds(fm);
	nana::textbox tx_p_memory_slice(fm);

	layout["tx_p_id"] << tx_p_id;
	layout["tx_p_execute_seconds"] << tx_p_execute_seconds;
	layout["tx_p_memory_slice"] << tx_p_memory_slice;

	tx_p_id.typeface(nana::paint::font("*", 15));
	tx_p_execute_seconds.typeface(nana::paint::font("*", 15));
	tx_p_memory_slice.typeface(nana::paint::font("*", 15));

	tx_p_id.multi_lines(false);
	tx_p_execute_seconds.multi_lines(false);
	tx_p_memory_slice.multi_lines(false);

	nana::button bt_confirm(fm, "添加");

	/// 单击确定按钮或焦点在确定按钮上按回车所触发的回调函数
	auto confirm_events = ([&]() {
		int p_id;
		std::chrono::seconds p_execute_seconds;
		int p_memory_slice;

		try {
			p_id = std::stoi(tx_p_id.caption());
		} catch (const std::invalid_argument & e) {
			tx_p_id.caption("");
			tx_p_id.focus();
			return;
		}

		try {
			p_execute_seconds = std::chrono::seconds(std::stoi(tx_p_execute_seconds.caption()));
		} catch (const std::invalid_argument & e) {
			tx_p_execute_seconds.caption("");
			tx_p_execute_seconds.focus();
			return;
		}

		try {
			p_memory_slice = std::stoi(tx_p_memory_slice.caption());
		} catch (const std::invalid_argument & e) {
			tx_p_memory_slice.caption("");
			tx_p_memory_slice.focus();
			return;
		}

		callback(std::make_unique<ProcessControlBlock>(p_id, p_execute_seconds, p_memory_slice));
		fm.close();
	});

	bt_confirm.events().click(confirm_events);
	bt_confirm.events().key_press([&](const nana::arg_keyboard& arg) {
		if (arg.key == 65421) {
			// 当按下回车键时, 触发确认操作
			confirm_events();
		}
	});

	layout["bt_confirm"] << bt_confirm;
	layout.collocate();
	tx_p_id.focus();
	fm.show();
	nana::exec();
}

#endif /* ADD_NEW_PROCESS_HPP_ */
