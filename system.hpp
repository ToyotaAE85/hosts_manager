#pragma once

#include "./win32_api.hpp"

#include "./localization.hpp"

#include "./window_base.hpp"
#include "./menu_bar.hpp"
#include "./list_view.hpp"
#include "./actions_menu.hpp"
#include "./win32_utility.hpp"

#include <type_traits>
#include <utility>

namespace Constants {
	constexpr inline unsigned int MinTextSize{ 10 };
	constexpr inline unsigned int MaxTextSize{ 25 };
}

namespace System {

	struct Settings {
		std::uint32_t text_size{ };
		std::uint32_t filter_modes{ };
		bool changes_performed{ };
	};

	class MainWindow final : WndBase::WindowBase {

		
		HINSTANCE hInstance_{ };
		
		Screen::Data screenData_{ };
		Settings inner_settings_{ };

		List::ListView listView_{ };
		Menu::MenuBar menuBar_{ };
		ActionsMenu::Buttons buttons_{ };

	public:

		MainWindow(HINSTANCE hInst) noexcept;

		void initialize(int);
		
	private:
		// maybe keep this, just make it work already lmfao
		template<std::same_as<Win32Utility::FontData> ...T >
		void apply_font_to(T& ... args) {
			([&](auto& par) {
				auto& [font, hwnd, data] { par };

				HFONT new_font{ Win32Utility::create_font(data) };

				SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(new_font), TRUE);

				if (font)
					DeleteFontW(font);

				font = new_font;

			}(args), ...);
		}

		//hmmm, not sure about this one.
		template<typename T>
		void handle_internal_error(T data) {
			auto [type, err] { data };

			[[maybe_unused]] int res{ 
				Message::display(type, Localization::TextMap<decltype(err)>{}.map(err)) 
			};

			return;
		}

		void prepare_ui();

		void handle_file_backup();

		void handle_file_cleanup();

		void handle_file_restore();

		void change_setting(WmId::Custom setting, WmId::MenuBar value);

		void update_self_title();

		void flush_dns();

	protected:
		LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;
	};

}
