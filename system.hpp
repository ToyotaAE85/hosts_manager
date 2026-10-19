#pragma once

#include "./win32api.hpp"

#include "./component.hpp"
#include "./localization.hpp"

#include <cstdint>

#include <tuple>
#include <type_traits>
#include <utility>

namespace System {

	struct SettingsLayout {
		Language::Type language{ };
		std::uint8_t text_size{ };
		std::uint8_t filter_modes{ };
		bool changes_performed{ false };
	};

	namespace Setting {
		struct Language {};
		struct TextSize {};
		struct FilterModes {};
	}

	class MainWindow final : Component::WindowBase {

		bool window_changes_{ };
		SettingsLayout settings_{ };

		HINSTANCE hInstance_{ };
	
		Component::ListView listView_{ };
		Component::MenuBar menuBar_{ };

		Screen::Data screenData_{ };

	public:

		MainWindow(HINSTANCE hInst) noexcept;

		void initialize(int);

		template<typename ...Tags>
		auto request_settings() const {
			return std::make_tuple(
				[](const auto& settings, auto tag) {
					using T = decltype(tag);
					if constexpr (std::is_same_v<T, Setting::TextSize>)   return settings.text_size;
					if constexpr (std::is_same_v<T, Setting::Language>)   return settings.language;
					if constexpr (std::is_same_v<T, Setting::FilterModes>) return settings.filter_modes;
				}(settings_, Tags{})...);
		}

		template<typename ...Tags>
		void update_settings(const auto&... new_values) {
			static_assert(sizeof...(Tags) == sizeof...(new_values));

			([&](auto tag, const auto& value) {
				using T = decltype(tag);
				if constexpr (std::is_same_v<T, Setting::TextSize>)   settings_.text_size = value;
				else if constexpr (std::is_same_v<T, Setting::Language>)   settings_.language = value;
				else if constexpr (std::is_same_v<T, Setting::FilterModes>) settings_.filter_modes = value;
				}(Tags{}, new_values), ...);
		}

	private:

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

		template<typename T>
		void handle_internal_error(T data) {
			auto [type, err] { data };

			int res{ Message::display(type, Localization::TextMap<decltype(err)>{}.map(err)) };

			if (type == Message::Type::Warning && res == IDNO)
				PostMessageW(hwnd_, WM_CLOSE, 0, 0);

			return;
		}

		void prepare_ui();

		void handle_file_backup();

		void handle_file_cleanup();

	protected:
		LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;
	};

}
