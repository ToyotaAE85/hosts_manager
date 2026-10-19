#include "./system.hpp"

#include "./identifiers.hpp"
#include "./hosts.hpp"
#include "./file_internals.hpp"

#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")


System::MainWindow::MainWindow(HINSTANCE hInst) noexcept : hInstance_{ hInst } {}


void System::MainWindow::initialize(int nCmdShow) {

	Language::set(Language::Type::Spanish);

	settings_.language = Language::Type::Spanish;
	settings_.text_size = 16;

	WNDCLASSEXW wnd{ };
	wnd.lpfnWndProc = BaseProc;
	wnd.cbSize = sizeof(WNDCLASSEXW);
	wnd.hInstance = hInstance_;
	wnd.lpszClassName = L"MAINMENUWINDOW";
	
	RegisterClassExW(&wnd);

	Utility::create_window({
		.className = L"MAINMENUWINDOW",
		.windowName = Localization::get_text(Localization::Text::MainMenuWindow),
		.style = WS_OVERLAPPEDWINDOW,
		.hInst = hInstance_,
		.lpParam = this
	});

	UpdateWindow(hwnd_);
	ShowWindow(hwnd_, nCmdShow);
}


void System::MainWindow::prepare_ui() {

	using namespace List;
	using namespace Localization;

	screenData_.generate(hwnd_);

	menuBar_.create(hwnd_);

	listView_.create(hwnd_).populate_columns(
		ColumnData{ .name = get_text(Text::Lv_Line), .fmt = LVCFMT_CENTER, .size = 100 },
		ColumnData{ .name = get_text(Text::Lv_Ip), .fmt = LVCFMT_CENTER, .size = 100 },
		ColumnData{ .name = get_text(Text::Lv_Type), .fmt = LVCFMT_CENTER, .size = 100 },
		ColumnData{ .name = get_text(Text::Lv_Domain), .fmt = LVCFMT_LEFT, .size = 100 }
	).populate_rows(PopulateRows::FromFile);

	//initialize_fonts();

}

void System::MainWindow::handle_file_backup() {
	auto result{ FileInternals::backup_file() };

	if (!result) {
		handle_internal_error(result.error());
		return;
	}

	Message::display(Message::Type::Info, Localization::Text::Info_HostsFileBacked);
}

void System::MainWindow::handle_file_cleanup() {
	auto result{ FileInternals::delete_file_contents() };

	if (!result) {
		handle_internal_error(result.error());
		return;
	}

	Message::display(Message::Type::Info, Localization::Text::Info_HostsFileCleaned);
}



LRESULT System::MainWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CREATE: {

			SetPropW(hwnd_, L"MAINMENUWINDOW", this);
			prepare_ui();

			return 0;
		}
		case WM_NOTIFY: {
			LPNMHDR lpnmsg{ reinterpret_cast<LPNMHDR>(lParam) };

			if (lpnmsg->hwndFrom == listView_.hwnd()) {
				listView_.MessageHandler(lParam);
			}

			break;
		}
		case WM_SIZE: {

			using namespace Screen; // needed for | overloads.
			screenData_.regenerate_client_size(LOWORD(lParam), HIWORD(lParam));

			MoveWindow(
				listView_.hwnd(),
				screenData_ | From::Window | Dimension::Width | Percentage(5),
				screenData_ | From::Window | Dimension::Height | Percentage(5),
				screenData_ | From::Window | Dimension::Width | Percentage(90),
				screenData_ | From::Window | Dimension::Height | Percentage(80),
				TRUE
			);

			return 0;
		}
		case WM_GETMINMAXINFO: {
			using namespace Screen;
			MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);

			mmi->ptMinTrackSize.x = screenData_ | From::Screen | Dimension::Width | Percentage(30);
			mmi->ptMinTrackSize.y = 300; // Minimum height

			return 0; // Return 0 if you handle this message
		}
		case WM_PAINT: {
			PAINTSTRUCT ps{ };
			HDC hdc{ BeginPaint(hwnd_, &ps) };

			FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

			EndPaint(hwnd_, &ps);
		}
		case WM_COMMAND: {
			WORD wId { LOWORD(wParam) };

			WmId::MenuBar mId{ static_cast<WmId::MenuBar>(wId) };

			if (mId > WmId::MenuBar::Min && mId < WmId::MenuBar::Max) {
				menuBar_.MessageHandler(wId);
				return 0;
			}

			break;
		}
		case WM_CLOSE: {

			if (settings_.changes_performed)
				if (Message::display(Message::Type::Warning,
					Localization::Text::Warning_ChangesPending) == IDNO) 
					return 0;

			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case (UINT)WmId::Custom::Request_Language_Change: {

			Language::set(settings_.language);

			SetWindowTextW(hwnd_, Localization::get_text(Localization::Text::MainMenuWindow).data());
			menuBar_.create(hwnd_);
			listView_.update_labels();

			Message::display(Message::Type::Info, Localization::Text::Info_LanguageUpdate);

			return 0;
		}
		case (UINT)WmId::Custom::Request_Filter_Change: {

			listView_.refresh_list(List::PopulateRows::FromMemory);
			Message::display(Message::Type::Info, Localization::Text::Info_ListRowsUpdate);

			return 0;
		}
		case (UINT)WmId::Custom::Request_List_Reload: {

			if (settings_.changes_performed) 
				if (Message::display(Message::Type::Warning, Localization::Text::Warning_ReloadWillOverWrite) == IDNO) 
					return 0;
			
			listView_.refresh_list(List::PopulateRows::FromFile);
			Message::display(Message::Type::Info, Localization::Text::Info_ListReloadUpdate);
			settings_.changes_performed = false;

			return 0;
		}

		case (UINT)WmId::Custom::Request_TextSize_Change: {
			return 0;
		}

		case (UINT)WmId::Custom::Request_Clear_File: {
			handle_file_cleanup();
			return 0;
		}

		case (UINT)WmId::Custom::Request_Backup_File: {
			handle_file_backup();
			return 0;
		}
	}
	return DefWindowProcW(hwnd_, msg, wParam, lParam);
}