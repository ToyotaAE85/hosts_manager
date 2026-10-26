#include "./actions_menu.hpp"

#include "./win32_utility.hpp"
#include "./localization.hpp"
#include "./identifiers.hpp"

namespace ActionsMenu {

	void Buttons::initialize(HWND parent, Screen::Data screendata) {
		using namespace Screen;
		using namespace Win32Utility;

		Win32Utility::CreateWindowData base{
			.exStyle = WS_EX_WINDOWEDGE,
			.className = L"BUTTON",
			.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_VCENTER | BS_CENTER | BS_MULTILINE | BS_PUSHBUTTON,
			.y = screendata | From::Window | Dimension::Height | Percentage(90),
			.width = screendata | From::Window | Dimension::Width | Percentage(12),
			.height = screendata | From::Window | Dimension::Height | Percentage(6),
			.hParent = parent,
			.hInst = (HINSTANCE)GetWindowLongPtrW(parent, GWLP_HINSTANCE),
		};

		base.windowName = Localization::get_text(Localization::Text::Control_NewEntry);
		base.x = screendata | From::Window | Dimension::Width | Percentage(5);
		base.id = static_cast<std::uint64_t>(WmId::Control::NewEntry);
		new_ = create_window(base);


		base.windowName = Localization::get_text(Localization::Text::Control_DeleteEntry);
		base.x = screendata | From::Window | Dimension::Width | Percentage(20);
		base.id = static_cast<std::uint64_t>(WmId::Control::DeleteEntry);
		delete_ = create_window(base);


		base.windowName = Localization::get_text(Localization::Text::Control_ApplyFilter);
		base.x = screendata | From::Window | Dimension::Width | Percentage(44);
		base.id = static_cast<std::uint64_t>(WmId::Control::ApplyFilter);
		apply_ = create_window(base);


		base.windowName = Localization::get_text(Localization::Text::Control_SaveEntry);
		base.x = screendata | From::Window | Dimension::Width | Percentage(68);
		base.id = static_cast<std::uint64_t>(WmId::Control::SaveEntry);
		base.style |= WS_DISABLED; // this change persists, careful down the line
		save_ = create_window(base);


		base.windowName = Localization::get_text(Localization::Text::Control_DiscardChanges);
		base.x = screendata | From::Window | Dimension::Width | Percentage(83);
		base.id = static_cast<std::uint64_t>(WmId::Control::DiscardChanges);
		discard_ = create_window(base);
	}

	void Buttons::resize(Screen::Data screendata) {

		using namespace Screen;

		const int b_y{
			screendata | From::Window | Dimension::Height | Percentage(90)
		};

		const int b_w{
			screendata | From::Window | Dimension::Width | Percentage(12)
		};

		const int b_h{
			screendata | From::Window | Dimension::Height | Percentage(6)
		};

		MoveWindow(
			new_,
			screendata | From::Window | Dimension::Width | Percentage(5),
			b_y,
			b_w,
			b_h,
			TRUE
		);

		MoveWindow(
			delete_,
			screendata | From::Window | Dimension::Width | Percentage(20),
			b_y,
			b_w,
			b_h,
			TRUE
		);

		MoveWindow(
			apply_,
			screendata | From::Window | Dimension::Width | Percentage(44),
			b_y,
			b_w,
			b_h,
			TRUE
		);

		MoveWindow(
			save_,
			screendata | From::Window | Dimension::Width | Percentage(68),
			b_y,
			b_w,
			b_h,
			TRUE
		);

		MoveWindow(
			discard_,
			screendata | From::Window | Dimension::Width | Percentage(83),
			b_y,
			b_w,
			b_h,
			TRUE
		);
	}

}