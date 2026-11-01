#include "./actions_menu.hpp"

#include "./win32_utility.hpp"
#include "./identifiers.hpp"
#include "./message_handler.hpp"

namespace ActionsMenu {

	void Buttons::initialize(HWND parent, Screen::Data screendata) {
		using namespace Screen;
		using namespace Win32Utility;

		Win32Utility::CreateWindowData base{
			.exStyle = WS_EX_WINDOWEDGE,
			.className = L"BUTTON",
			.windowName = L"UNNAMED",
			.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_VCENTER | BS_CENTER | BS_MULTILINE | BS_PUSHBUTTON,
			.y = screendata | From::Window | Dimension::Height | Percentage(90),
			.width = screendata | From::Window | Dimension::Width | Percentage(12),
			.height = screendata | From::Window | Dimension::Height | Percentage(6),
			.hParent = parent,
			.hInst = (HINSTANCE)GetWindowLongPtrW(parent, GWLP_HINSTANCE),
		};

		base.x = screendata | From::Window | Dimension::Width | Percentage(5);
		base.id = static_cast<std::uint64_t>(WmId::Control::NewEntry);
		new_ = create_window(base);


		base.x = screendata | From::Window | Dimension::Width | Percentage(44);
		base.id = static_cast<std::uint64_t>(WmId::Control::ApplyFilter);
		apply_ = create_window(base);


		base.x = screendata | From::Window | Dimension::Width | Percentage(20);
		base.style |= WS_DISABLED; // this change persists, careful down the line
		base.id = static_cast<std::uint64_t>(WmId::Control::DeleteEntry);
		delete_ = create_window(base);


		base.x = screendata | From::Window | Dimension::Width | Percentage(68);
		base.id = static_cast<std::uint64_t>(WmId::Control::SaveEntry);
		save_ = create_window(base);


		base.x = screendata | From::Window | Dimension::Width | Percentage(83);
		base.id = static_cast<std::uint64_t>(WmId::Control::DiscardChanges);
		discard_ = create_window(base);

		update_labels();
	}

	void Buttons::resize(Screen::Data screendata) {

		using namespace Screen;

		const int y{
			screendata | From::Window | Dimension::Height | Percentage(90)
		};

		const int w{
			screendata | From::Window | Dimension::Width | Percentage(12)
		};

		const int h{
			screendata | From::Window | Dimension::Height | Percentage(6)
		};

		HDWP hDwp{ BeginDeferWindowPos(5) };

		if (!hDwp) {
			resize_fallback(y, w, h, screendata);
			return;
		}

		hDwp = DeferWindowPos(
			hDwp, new_, nullptr,
			screendata | From::Window | Dimension::Width | Percentage(5),
			y, w, h, SWP_NOZORDER | SWP_NOACTIVATE
		);

		hDwp = DeferWindowPos(
			hDwp, delete_, nullptr,
			screendata | From::Window | Dimension::Width | Percentage(20),
			y, w, h, SWP_NOZORDER | SWP_NOACTIVATE
		);

		hDwp = DeferWindowPos(
			hDwp, apply_, nullptr,
			screendata | From::Window | Dimension::Width | Percentage(44),
			y, w, h, SWP_NOZORDER | SWP_NOACTIVATE
		);

		hDwp = DeferWindowPos(
			hDwp, save_, nullptr,
			screendata | From::Window | Dimension::Width | Percentage(68),
			y, w, h, SWP_NOZORDER | SWP_NOACTIVATE
		);

		hDwp = DeferWindowPos(
			hDwp, discard_, nullptr,
			screendata | From::Window | Dimension::Width | Percentage(83),
			y, w, h, SWP_NOZORDER | SWP_NOACTIVATE
		);

		if (hDwp != nullptr) 
			EndDeferWindowPos(hDwp);
		else 
			resize_fallback(y, w, h, screendata);
	}

	void Buttons::update_labels() {

		update_labels_(new_, Localization::Text::Control_NewEntry);
		update_labels_(delete_, Localization::Text::Control_DeleteEntry);
		update_labels_(apply_, Localization::Text::Control_ApplyFilter);
		update_labels_(save_, Localization::Text::Control_SaveEntry);
		update_labels_(discard_, Localization::Text::Control_DiscardChanges);

	}

	void Buttons::enable_buttons(bool status) {

		// save & revert are enabled if changes are performed on the list.
		// apply is disabled if changes are performed
		// delete entry is only enabled if a listview item is selected (manual behaviour)

		EnableWindow(save_, status);
		EnableWindow(discard_, status);
		EnableWindow(apply_, !status);
	}

	void Buttons::update_labels_(HWND to, Localization::Text text) {
		if (SetWindowTextW(to, Localization::get_text(text).data()) == FALSE)
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_UIFailed);
	}

	void Buttons::resize_fallback(int y, int w, int h, Screen::Data screendata) {
		using namespace Screen;

		MoveWindow(
			new_,
			screendata | From::Window | Dimension::Width | Percentage(5),
			y,
			w,
			h,
			TRUE
		);

		MoveWindow(
			delete_,
			screendata | From::Window | Dimension::Width | Percentage(20),
			y,
			w,
			h,
			TRUE
		);

		MoveWindow(
			apply_,
			screendata | From::Window | Dimension::Width | Percentage(44),
			y,
			w,
			h,
			TRUE
		);

		MoveWindow(
			save_,
			screendata | From::Window | Dimension::Width | Percentage(68),
			y,
			w,
			h,
			TRUE
		);

		MoveWindow(
			discard_,
			screendata | From::Window | Dimension::Width | Percentage(83),
			y,
			w,
			h,
			TRUE
		);
	}
}