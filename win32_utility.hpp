#pragma once

#include "./win32_api.hpp"

#include <string_view>
#include <cstdint>

namespace Win32Utility {

	struct CreateWindowData {
		DWORD exStyle{ 0 };

		std::wstring_view className;
		std::wstring_view windowName{ L"" };

		DWORD style;

		std::int32_t x{ CW_USEDEFAULT };
		std::int32_t y{ CW_USEDEFAULT };
		std::int32_t width{ CW_USEDEFAULT };
		std::int32_t height{ CW_USEDEFAULT };

		HWND hParent{ nullptr };
		std::uint64_t id{ };
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