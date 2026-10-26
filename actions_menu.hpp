#pragma once

#include "./win32_api.hpp"

#include "./screen.hpp"

#include "./localization.hpp"

namespace ActionsMenu {

	struct Buttons {
		HWND new_{ };
		HWND delete_{ };
		HWND save_{ };
		HWND apply_{ };
		HWND discard_{ };

		void initialize(HWND parent, Screen::Data screendata);
		void resize(Screen::Data screendata);
		void update_labels();
		void enable_buttons(bool status);

	private:

		void update_labels_(HWND to, Localization::Text text);
	};

}