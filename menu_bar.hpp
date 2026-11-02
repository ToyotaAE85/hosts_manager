#pragma once

#include "./win32_api.hpp"
#include "./identifiers.hpp"

#include <cstdint>
#include <string_view>

namespace Menu {
	class MenuBar {

		HWND parent_{ };
		HMENU hMenu_{ };

		std::uint32_t textsize_{ };
		std::uint32_t filtermodes_{ };

	public: // hmm, passing the required data at creation.... keep for now?

		void create(HWND parent);

		LRESULT message_handler(WORD cmd);

		// updating settings requires a full recreation of the menu, call create() afterwards
		MenuBar& update_settings(std::uint32_t text_size, std::uint32_t filter_modes);

	private:

		HMENU build_menu();

		bool update_labels(HMENU hMenu);

		bool check_item(HMENU hMenu, WmId::MenuBar target, bool check);

		bool rename_item(HMENU hMenu, WmId::MenuBar id, std::wstring_view new_text);

		bool disable_item(HMENU hMenu, WmId::MenuBar id);
	};
}