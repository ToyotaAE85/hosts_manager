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

		HWND hwnd() const;

		void initialize(int);
		
	private:

		//hmmm, not sure about this one.
		template<typename T>
		void handle_internal_error(T data) {
			auto [type, err] { data };

			[[maybe_unused]] int res { 
				Message::display(type, Localization::TextMap<decltype(err)>{}.map(err)) 
			};

			return;
		}

		void prepare_ui();

		void handle_file_backup();

		void handle_file_cleanup();

		void handle_file_restore();

		void change_setting(WmId::Custom setting, WmId::MenuBar value);

		void resize_window_elements(std::int32_t nWidth, std::int32_t nHeight);

		void update_self_title();

		void flush_dns();

		void save_data();

		void handle_file_reload();

	protected:
		LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;
	};

}
