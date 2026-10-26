#include "./message_handler.hpp"

#include "./win32_api.hpp"

#include <format>

namespace Message {

	static UINT set_flags(Type type) noexcept {

		UINT flags{ MB_TASKMODAL | MB_TOPMOST };

		switch (type) {
			case Type::CriticalError:	flags |= MB_ICONERROR | MB_OK; break;
			case Type::Error:			flags |= MB_ICONSTOP | MB_YESNO; break;
			case Type::Warning:			flags |= MB_ICONWARNING | MB_YESNO; break;
			case Type::Info:			flags |= MB_ICONINFORMATION | MB_OK; break;
			case Type::Confirm:			flags |= MB_ICONINFORMATION | MB_OKCANCEL; break;
			[[unlikely]] default:		flags |= MB_OK; break;
		}

		return flags;
	}

	static std::wstring fmt_error(Type type, std::wstring_view text) {

		std::wstring fmt_out{ text };

		if (type != Type::Info && type != Type::Confirm) {

			if (type == Type::CriticalError) {
				fmt_out = std::format(L"{}\n{}", fmt_out,
					Localization::get_text(Localization::Text::MessageMisc_CriticalLabel));
			}
			else {
				if (type == Type::Error) {
					fmt_out = std::format(L"{}\n{}", fmt_out,
						Localization::get_text(Localization::Text::MessageMisc_ErrorLabel));
				}

				fmt_out = std::format(L"{}\n{}", fmt_out,
					Localization::get_text(Localization::Text::MessageMisc_ContinueLabel));
			}
		}

		return fmt_out;
	}

	int display(Type type, Localization::Text text) {

		UINT flags{ set_flags(type) };

		std::wstring err_str{ Localization::get_text(text) };
		err_str = fmt_error(type, err_str);

		int result{ MessageBoxExW(nullptr, err_str.c_str(), Localization::get_text(type).data(), flags, 0) };

		if (type == Type::CriticalError || (type == Type::Error && result == IDNO)) {
			PostQuitMessage(0);
			std::terminate();
		}

		return result;
	}
} // Namespace Message