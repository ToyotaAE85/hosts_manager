#pragma once

#include "./win32api.hpp"

#include "./hosts.hpp"
#include "./localization.hpp"
#include "./identifiers.hpp"


#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

#include <string>
#include <vector>
#include <concepts>


namespace Message {

	enum class Type {
		CriticalError,
		Error,
		Warning,
		Info,
		Confirm,
	};

	int display(Type error, Localization::Text error_text);

}

namespace Screen {

	struct Percentage {
		std::int32_t prc_;
		explicit Percentage(std::int32_t prc) noexcept : prc_{ prc } {}
	};

	struct WindowSize {
		std::int32_t x;
		std::int32_t y;
	};

	enum class From {
		Screen,
		Window,
	};

	enum class Dimension {
		Width,
		Height,
	};

	struct Data {

		WindowSize abso_pos_;
		WindowSize client_size_;


		void generate(HWND parent);

		void regenerate_client_size(std::int32_t width, std::int32_t height);
	};


	struct Pipestate {
		const Data& data;
		From from;
		Dimension dim{ Dimension::Width };
	};


	Pipestate operator|(Data& obj, From from);

	Pipestate operator|(Pipestate obj, Dimension dim);

	int operator|(Pipestate obj, Percentage prc);

}

namespace List {
	enum class PopulateRows {
		FromFile,
		FromMemory
	};

	struct ColumnData {
		std::wstring_view name;
		int fmt;
		int size;
	};
}


namespace Utility {

	struct CreateWindowData {
		DWORD exStyle{ 0 };

		std::wstring_view className;
		std::wstring_view windowName{ L"" };

		DWORD style;

		int x{ CW_USEDEFAULT };
		int y{ CW_USEDEFAULT };
		int width{ CW_USEDEFAULT };
		int height{ CW_USEDEFAULT };

		HWND hParent{ nullptr };
		HMENU id{ };
		HINSTANCE hInst{ nullptr };
		LPVOID lpParam{ nullptr };
	};

	struct CreateFontData {
		std::int32_t font_height;
		std::int32_t font_width{ 0 };

		std::int32_t escape_angle{ 0 };
		std::int32_t orientation{ 0 };

		std::int32_t font_weight{ FW_DONTCARE };

		DWORD italic{ FALSE };
		DWORD underline{ FALSE };
		DWORD strike_out{ FALSE };

		DWORD char_set{ DEFAULT_CHARSET };

		DWORD out_precision{ OUT_DEFAULT_PRECIS };
		DWORD clip_precision{ CLIP_DEFAULT_PRECIS };

		DWORD quality{ DEFAULT_QUALITY };

		DWORD pitch_family{ FF_DONTCARE };
		LPCWSTR font_name{ L"Segoe UI" };
	};

	struct FontData {
		HFONT font;
		HWND target;
		CreateFontData data;
	};

	HWND create_window(const CreateWindowData& data);

	HFONT create_font(const CreateFontData& data);
}

namespace Component {


	class WindowBase {
	protected:
		HWND hwnd_{ nullptr };

		static LRESULT CALLBACK BaseProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		virtual LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
	};


	class ListView {

		HWND hwnd_{ };
		std::vector<Hosts::Line::Data> line_data_{ };

	public:

		HWND hwnd() const;

		ListView& create(HWND parent);

		template<std::same_as<List::ColumnData> ...T>
		ListView& populate_columns(T... args) {

			LVCOLUMNW lvc{ };
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

			ListView_InsertColumn(hwnd_, 0, &lvc); // dummy column.

			int col_counter{ 1 };

			((
				lvc.fmt = args.fmt,
				lvc.iSubItem = col_counter,
				lvc.pszText = const_cast<LPWSTR>(args.name.data()),
				lvc.cx = args.size,

				ListView_InsertColumn(hwnd_, col_counter, &lvc),
				++col_counter
			), ...);

			ListView_DeleteColumn(hwnd_, 0);

			return *this;
		}

		ListView& populate_rows(List::PopulateRows input);

		LRESULT MessageHandler(LPARAM lParam);

		void refresh_list(List::PopulateRows from);

		void update_labels();

	private:

		void rewrite_data();

		auto retrieve_filtered_view();

	};


	class MenuBar {

		HWND parent_{ };
		HMENU hMenu_{ };

	public:

		void create(HWND parent);

		LRESULT MessageHandler(WORD cmd);

	private:

		HMENU build_menu();

		bool check_menu_item(HMENU hMenu, WmId::MenuBar target, bool check);

		bool rename_menu_item(HMENU hMenu, WmId::MenuBar id, std::wstring_view new_text);

		bool update_menu_labels(HMENU hMenu);
	};

}


template <>
struct Localization::TextMap<Message::Type> {
	static constexpr Localization::Text map(Message::Type type) noexcept {
		switch (type) {
			case Message::Type::CriticalError:	return Localization::Text::MessageType_CriticalError;
			case Message::Type::Error:			return Localization::Text::MessageType_Error;
			case Message::Type::Warning:		return Localization::Text::MessageType_Warning;
			case Message::Type::Info:			return Localization::Text::MessageType_Info;
			case Message::Type::Confirm:		return Localization::Text::MessageType_Confirm;
			[[unlikely]] default:				return Localization::Text::NO_TEXT;
		}
	}
};
