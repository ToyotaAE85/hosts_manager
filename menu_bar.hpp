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

		LRESULT MessageHandler(WORD cmd);

		MenuBar& update_menu_settings(std::uint32_t textsize, std::uint32_t filtermodes) noexcept;

	private:

		HMENU build_menu();

		bool update_menu_labels(HMENU hMenu);

		bool check_menu_item(HMENU hMenu, WmId::MenuBar target, bool check);

		bool rename_menu_item(HMENU hMenu, WmId::MenuBar id, std::wstring_view new_text);
	};
}