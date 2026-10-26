#pragma once

#include "./win32api.hpp"

#include "./component.hpp"
#include "./localization.hpp"

#include <cstdint>

#include <tuple>
#include <type_traits>
#include <utility>
#include <filesystem>

namespace System {
	
	// get rid of this, use wParam/lParam instead to pass stuff around yo
	struct SettingsLayout {
		Language::Type language{ };
		std::uint8_t text_size{ };
		std::uint8_t filter_modes{ };
		bool changes_performed{ false };
		std::filesystem::path current_file{ L"" };
	};

	namespace Setting {
		struct Language {};
		struct TextSize {};
		struct FilterModes {};
		struct ChangedBehaviour {};
		struct CurrentFile {};
	}

	class MainWindow final : Component::WindowBase {

		bool window_changes_{ };
		SettingsLayout settings_{ };

		HINSTANCE hInstance_{ };
	
		Component::ListView listView_{ };
		Component::MenuBar menuBar_{ };

		Screen::Data screenData_{ };

		HWND control_delete_{ };
		HWND control_new_{ };
		HWND control_save_{ };
		HWND control_apply_{ };
		HWND control_discard_{ };

	public:

		MainWindow(HINSTANCE hInst) noexcept;

		void initialize(int);
		
		//get rid of this
		template<typename ...Tags>
		auto request_settings() const {
			return std::make_tuple(
				[](const auto& settings, auto tag) {
					using T = decltype(tag);
					if constexpr (std::is_same_v<T, Setting::TextSize>)			return settings.text_size;
					if constexpr (std::is_same_v<T, Setting::Language>)			return settings.language;
					if constexpr (std::is_same_v<T, Setting::FilterModes>)		return settings.filter_modes;
					if constexpr (std::is_same_v<T, Setting::ChangedBehaviour>) return settings.changes_performed;
					if constexpr (std::is_same_v<T, Setting::CurrentFile>)		return settings.current_file;
				}(settings_, Tags{})...);
		}
		
		template<typename ...Tags>
		void update_settings(const auto&... new_values) {
			static_assert(sizeof...(Tags) == sizeof...(new_values));

			([&](auto tag, const auto& value) {
				using T = decltype(tag);
				if constexpr (std::is_same_v<T, Setting::Language>)				settings_.language = value;
				if constexpr (std::is_same_v<T, Setting::TextSize>)				settings_.text_size = value;
				if constexpr (std::is_same_v<T, Setting::FilterModes>)			settings_.filter_modes = value;
				if constexpr (std::is_same_v<T, Setting::ChangedBehaviour>)		settings_.changes_performed = value;
				if constexpr (std::is_same_v<T, Setting::CurrentFile>)			settings_.current_file = value;
				}(Tags{}, new_values), ...);
		}

	private:
		// maybe keep this
		template<std::same_as<Utility::FontData> ...T >
		void apply_font_to(T& ... args) {
			([&](auto& par) {
				auto& [font, hwnd, data] { par };

				HFONT new_font{ Utility::create_font(data) };

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

			[[maybe_unused]] int res{ Message::display(type, Localization::TextMap<decltype(err)>{}.map(err)) };

			return;
		}

		void prepare_ui();

		void handle_file_backup();

		void handle_file_cleanup();

		void handle_file_restore();

		void pending_changes(bool val);

		void initialize_controls();

	protected:
		LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;
	};

}
