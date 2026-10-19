#include "./component.hpp"

#include <format>

namespace Message {

	static UINT set_flags(Type error) noexcept {

		UINT flags{ MB_TASKMODAL | MB_TOPMOST };

		switch (error) {
			case Type::CriticalError:	flags |= MB_ICONERROR | MB_OK; break;
			case Type::Error:			flags |= MB_ICONSTOP | MB_YESNO; break;
			case Type::Warning:			flags |= MB_ICONWARNING | MB_YESNO; break;
			case Type::Info:			flags |= MB_ICONINFORMATION | MB_OK; break;
			case Type::Confirm:			flags |= MB_ICONINFORMATION | MB_OKCANCEL; break;
			[[unlikely]] default:		flags |= MB_OK; break;
		}

		return flags;
	}

	static std::wstring fmt_error(Type error, std::wstring_view text) {

		std::wstring fmt_out{ text };

		if (error != Type::Info && error != Type::Confirm) {

			if (error == Type::CriticalError) {
				fmt_out = std::format(L"{}\n{}", fmt_out,
					Localization::get_text(Localization::Text::MessageMisc_CriticalLabel));
			}
			else {
				if (error == Type::Error) {
					fmt_out = std::format(L"{}\n{}", fmt_out,
						Localization::get_text(Localization::Text::MessageMisc_ErrorLabel));
				}

				fmt_out = std::format(L"{}\n{}", fmt_out,
					Localization::get_text(Localization::Text::MessageMisc_ContinueLabel));
			}
		}

		return fmt_out;
	}

	int display(Type error, Localization::Text text) {

		UINT flags{ set_flags(error) };

		std::wstring err_str{ Localization::get_text(text) };
		err_str = fmt_error(error, err_str);

		int result{ MessageBoxExW(nullptr, err_str.c_str(), Localization::get_text(error).data(), flags, 0) };

		if (error == Type::CriticalError || (error == Type::Error && result == IDNO)) {
			PostQuitMessage(0);
			std::terminate();
		}

		return result;
	}
} // Namespace Message

namespace Component {

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

} // Namespace Component