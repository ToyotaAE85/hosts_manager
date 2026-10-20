#include "./system.hpp"

#include "./identifiers.hpp"
#include "./file_internals.hpp"

#pragma comment(lib, "comctl32.lib")
#include <CommCtrl.h>

#include <ShObjIdl.h>

namespace System {

	MainWindow::MainWindow(HINSTANCE hInst) noexcept : hInstance_{ hInst } {}


	void MainWindow::initialize(int nCmdShow) {

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


	void MainWindow::prepare_ui() {

		using namespace List;
		using namespace Localization;
		using Screen::operator|;

		screenData_.generate(hwnd_);

		menuBar_.create(hwnd_);

		listView_.create(hwnd_).populate_columns(
			ColumnData{ .name = get_text(Text::Lv_Line), .fmt = LVCFMT_CENTER, 
				.size = screenData_ | Screen::From::Window | Screen::Dimension::Width | Screen::Percentage(5) },
			
			ColumnData{ .name = get_text(Text::Lv_Ip), .fmt = LVCFMT_CENTER, 
			.size = screenData_ | Screen::From::Window | Screen::Dimension::Width | Screen::Percentage(5) },
			
			ColumnData{ .name = get_text(Text::Lv_Type), .fmt = LVCFMT_CENTER, 
			.size = screenData_ | Screen::From::Window | Screen::Dimension::Width | Screen::Percentage(5) },
			
			ColumnData{ .name = get_text(Text::Lv_Domain), .fmt = LVCFMT_LEFT, 
			.size = screenData_ | Screen::From::Window | Screen::Dimension::Width | Screen::Percentage(70) }
		).populate_rows(PopulateRows::FromFile);

		//initialize_fonts();

	}

	void MainWindow::handle_file_backup() {
		auto result{ FileInternals::backup_file() };

		if (!result) {
			handle_internal_error(result.error());
			return;
		}

		Message::display(Message::Type::Info, Localization::Text::Info_HostsFileBacked);
	}

	void MainWindow::handle_file_cleanup() {
		auto result{ FileInternals::delete_file_contents() };

		if (!result) {
			handle_internal_error(result.error());
			return;
		}

		Message::display(Message::Type::Info, Localization::Text::Info_HostsFileCleaned);
	}

	void MainWindow::handle_file_restore() {

		using namespace Localization;

		if (settings_.changes_performed) {
			if (Message::display(Message::Type::Warning, Text::Warning_ChangesPending) == IDNO)
				return;
		}
		
		HRESULT hr{ CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) };

		if (SUCCEEDED(hr)) {
			IFileOpenDialog* pFileMenu{};
			
			hr = CoCreateInstance(CLSID_FileOpenDialog,
				nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileMenu));

			if (SUCCEEDED(hr)) {

				auto backup_dir{ FileInternals::get_backup_dir() };

				if (!backup_dir) {
					handle_internal_error(backup_dir.error());
					pFileMenu->Release();
					CoUninitialize();
					return;
				}

				pFileMenu->SetTitle(get_text(Text::FileOpenDialogTitle).data());
				IShellItem* psiDefaultFolder{};

				hr = SHCreateItemFromParsingName(backup_dir.value().wstring().data(), 
					nullptr, IID_PPV_ARGS(&psiDefaultFolder));

				if (SUCCEEDED(hr)) {
					hr = pFileMenu->SetFolder(psiDefaultFolder);
					psiDefaultFolder->Release();

				}

				COMDLG_FILTERSPEC flSpec{
					get_text(Text::FileOpenDialogBackupName).data(), L"*.bak"
				};

				pFileMenu->SetFileTypes(1, &flSpec);

				hr = pFileMenu->Show(hwnd_);

				if (SUCCEEDED(hr)) {
					IShellItem* psiItem{};
					hr = pFileMenu->GetResult(&psiItem);

					if (SUCCEEDED(hr)) {
						PWSTR pszFilePath{ };
						hr = psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

						if (SUCCEEDED(hr)) {
							settings_.current_file = pszFilePath;
							CoTaskMemFree(pszFilePath);

							listView_.refresh_list(List::PopulateRows::FromFile);
							Message::display(Message::Type::Info, Text::Info_BackupLoaded);
						}

						psiItem->Release();
					}
				}
				pFileMenu->Release();
			}
			CoUninitialize();
		}
	}


	LRESULT MainWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
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

				return 0;
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

				mmi->ptMinTrackSize.x = screenData_ | From::Screen | Dimension::Width | Percentage(45);
				mmi->ptMinTrackSize.y = screenData_ | From::Screen | Dimension::Width | Percentage(30);;

				return 0;
			}
			case WM_PAINT: {
				PAINTSTRUCT ps{ };
				HDC hdc{ BeginPaint(hwnd_, &ps) };

				FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

				EndPaint(hwnd_, &ps);

				return 0;
			}
			case WM_COMMAND: {
				WORD wId{ LOWORD(wParam) };

				WmId::MenuBar mId{ static_cast<WmId::MenuBar>(wId) };

				if (mId > WmId::MenuBar::Min && mId < WmId::MenuBar::Max) {
					menuBar_.MessageHandler(wId);
					return 0;
				}

				return 0;
			}
			case WM_CLOSE: {

				if (settings_.changes_performed)
					if (Message::display(Message::Type::Warning,
						Localization::Text::Warning_ChangesPending) == IDNO)
							return 0;

				// Further logic goes here

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

				const auto hosts_int{ FileInternals::validate_hosts() };

				if (!hosts_int) {
					handle_internal_error(hosts_int.error());
					return 0;
				}

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
			case (UINT)WmId::Custom::Request_Restore_File: {
				handle_file_restore();
				return 0;
			}
		}
		return DefWindowProcW(hwnd_, msg, wParam, lParam);
	}
}