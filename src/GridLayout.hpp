/*
 * GridLayout.hpp
 *
 *  Created on: 2018年12月25日
 *      Author: peter
 */

#ifndef GRIDLAYOUT_HPP_
#define GRIDLAYOUT_HPP_

#include <nana/gui/place.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>

/**
 * @brief 主窗口的网格布局辅助类
 */
class GridLayout : public nana::place
{
	public:

		GridLayout(nana::form & fm, nana::button & bt_add_new_process) :
				nana::place(fm)
		{
			this->nana::place::div(
					"vert"
					"<height=10>"
					"<<grid0><width=10><grid1><width=10><grid2>>"
					"<height=10>"
					"<<grid3><width=10><grid4><width=10><grid5>>"
					"<height=10>"
					"< <><bt_add_new_process width=120><> height=60>"
					"<height=10>"
			);
			(*this)["bt_add_new_process"] << bt_add_new_process;
		}

		template <typename Widget>
		void add(int i, Widget & widget)
		{
			using namespace std::literals::string_literals;
			if (i >= 6) {
				return;
			}
			(*this)[("grid"s + std::to_string(i)).c_str()] << widget;
		}
};



#endif /* GRIDLAYOUT_HPP_ */
