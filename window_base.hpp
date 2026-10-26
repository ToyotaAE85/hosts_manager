#pragma once

#include "./win32_api.hpp"

namespace WndBase {
	class WindowBase {
	protected:
		HWND hwnd_{ nullptr };

		static LRESULT CALLBACK BaseProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		virtual LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
	};
}