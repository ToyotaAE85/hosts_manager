#pragma once

#include "./win32_api.hpp"

#include "./screen.hpp"


namespace ActionsMenu {

	struct Buttons {
		HWND new_{ };
		HWND delete_{ };
		HWND save_{ };
		HWND apply_{ };
		HWND discard_{ };

		void initialize(HWND parent, Screen::Data screendata);
		void resize(Screen::Data screendata);
	};

}