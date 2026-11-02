#pragma once

#include "./win32_api.hpp"
#include "./screen.hpp"
#include "./localization.hpp"
#include "./win32_utility.hpp"

#include <vector>

namespace ActionsMenu {

	struct Buttons {
		HWND new_{ };
		HWND delete_{ };
		HWND save_{ };
		HWND apply_{ };
		HWND discard_{ };

		Buttons& initialize(HWND parent, Screen::Data screen);
		void resize(Screen::Data screendata);
		void update_labels();
		Buttons& enable_buttons(bool status);
		Buttons& set_font_height(int new_height);

	private:

		std::vector<Win32Utility::FontControl> font_data_{ };

		void assign_font_data();
		void update_labels_(HWND to, Localization::Text text);
		void resize_fallback(int y, int w, int h, Screen::Data screen);
	};

}