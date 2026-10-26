#include "./window_base.hpp"

namespace WndBase {

	LRESULT CALLBACK WindowBase::BaseProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		WindowBase* wndPtr{ nullptr };

		if (msg == WM_NCCREATE) {
			CREATESTRUCT* createPtr{ reinterpret_cast<CREATESTRUCT*>(lParam) };
			wndPtr = reinterpret_cast<WindowBase*>(createPtr->lpCreateParams);

			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wndPtr));
			wndPtr->hwnd_ = hwnd;
		}
		else {
			wndPtr = reinterpret_cast<WindowBase*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		}

		if (wndPtr)
			return wndPtr->MessageHandler(msg, wParam, lParam);

		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	LRESULT WindowBase::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
		return DefWindowProcW(hwnd_, msg, wParam, lParam);
	}

}