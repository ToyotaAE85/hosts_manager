#include "./system.hpp"

#include "./identifiers.hpp"
#include "./file_internals.hpp"

#include <CommCtrl.h>
#include <ShObjIdl.h>
#include <algorithm>

namespace System {

	MainWindow::MainWindow(HINSTANCE hInst) noexcept : hInstance_{ hInst } {}

	void MainWindow::initialize(int nCmdShow) {

		Language::set(Language::Type::Spanish);

		inner_settings_.text_size = 16;

		WNDCLASSEXW wnd{ };
		wnd.lpfnWndProc = BaseProc;
		wnd.cbSize = sizeof(WNDCLASSEXW);
		wnd.hInstance = hInstance_;
		wnd.lpszClassName = L"MAINMENUWINDOW";

		if (!RegisterClassExW(&wnd))
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_UIFailed);

		Win32Utility::create_window({
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
		using namespace Screen;

		screenData_.generate(hwnd_);

		menuBar_.create(hwnd_);

		listView_.create(hwnd_).populate_columns(
			ColumnData{ .name = get_text(Text::Lv_Line), .fmt = LVCFMT_CENTER, 
				.size = screenData_ | From::Window | Dimension::Width | Percentage(5) },
			
			ColumnData{ .name = get_text(Text::Lv_Ip), .fmt = LVCFMT_CENTER, 
			.size = screenData_ | From::Window | Dimension::Width | Percentage(5) },
			
			ColumnData{ .name = get_text(Text::Lv_Type), .fmt = LVCFMT_CENTER, 
			.size = screenData_ | From::Window | Dimension::Width | Percentage(5) },
			
			ColumnData{ .name = get_text(Text::Lv_Domain), .fmt = LVCFMT_LEFT, 
			.size = screenData_ | From::Window | Dimension::Width | Percentage(70) }
		).populate_rows(PopulateRows::FromFile);

		buttons_.initialize(hwnd_, screenData_);
		buttons_.enable_buttons(inner_settings_.changes_performed);

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

		if (inner_settings_.changes_performed) {
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
					pFileMenu->SetFolder(psiDefaultFolder);
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
							auto res{ FileInternals::set_target_file(pszFilePath) };
							CoTaskMemFree(pszFilePath);

							if (!res) {
								handle_internal_error(res.error());
							}
							else {
								buttons_.enable_buttons(inner_settings_.changes_performed = true);
								update_self_title();
								listView_.refresh_list(List::PopulateRows::FromFile);
								Message::display(Message::Type::Info, Text::Info_BackupLoaded);
							}
						}

						psiItem->Release();
					}
				}
				pFileMenu->Release();
			}
			CoUninitialize();
		}

		return;
	}

	int MainWindow::flush_dns() {
		return 0;
	}

	void MainWindow::update_self_title() {
		std::wstring title_text{ Localization::get_text(Localization::Text::MainMenuWindow) };

		if (inner_settings_.changes_performed)
			title_text = std::format(L"{} *", title_text);

		SetWindowTextW(hwnd_, title_text.c_str());
	}

	void MainWindow::change_setting(WmId::Custom setting, WmId::MenuBar value) {
		
		using namespace WmId;


		if (setting == Custom::Min || setting > Custom::Request_TextSize_Change)
			return;

		if (value < WmId::MenuBar::Text_IncreaseSize || value > WmId::MenuBar::Language_English)
			return;

		
		auto& [text_size, filter_modes, _] { inner_settings_ };

		if (setting == Custom::Request_Language_Change) {

			switch (value) {
				case MenuBar::Language_Spanish: Language::set(Language::Type::Spanish); break;
				default: Language::set(Language::Type::English); break;
			}
			
			update_self_title();
			buttons_.update_labels();
			listView_.update_labels();
		}
		else if (setting == Custom::Request_Filter_Change) {

			if (value == MenuBar::Filter_All) {
				filter_modes ^= 0x7;
			}
			else {
				const std::uint32_t bit_shift{
					static_cast<std::uint32_t>(value) - static_cast<std::uint32_t>(MenuBar::Filter_Invalid)
				};

				filter_modes ^= (0x1 << bit_shift);
			}

			listView_.update_filters(inner_settings_.filter_modes).refresh_list(List::PopulateRows::FromMemory);
		}
		else if (setting == Custom::Request_TextSize_Change) {

			if (text_size == Constants::MinTextSize || text_size == Constants::MaxTextSize)
				return;

			if (value == MenuBar::Text_DecreaseSize) {
				text_size = std::clamp(text_size - 1, Constants::MinTextSize, Constants::MaxTextSize);
			}
			else {
				text_size = std::clamp(text_size + 1, Constants::MinTextSize, Constants::MaxTextSize);
			}

			//further logic here
		}

		menuBar_.update_menu_settings(text_size, filter_modes).create(hwnd_);
		
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
					return listView_.MessageHandler(lParam);
				}

				return 0;
			}
			case WM_SIZE: {

				using namespace Screen;
				screenData_.regenerate_client_size(LOWORD(lParam), HIWORD(lParam));

				MoveWindow(
					listView_.hwnd(),
					screenData_ | From::Window | Dimension::Width | Percentage(5),
					screenData_ | From::Window | Dimension::Height | Percentage(5),
					screenData_ | From::Window | Dimension::Width | Percentage(90),
					screenData_ | From::Window | Dimension::Height | Percentage(80),
					TRUE
				);

				buttons_.resize(screenData_);

				return 0;
			}
			case WM_GETMINMAXINFO: {
				using namespace Screen;
				MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);

				mmi->ptMinTrackSize.x = screenData_ | From::Screen | Dimension::Width | Percentage(45);
				mmi->ptMinTrackSize.y = screenData_ | From::Screen | Dimension::Height | Percentage(45);

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

				WmId::Control cId{ static_cast<WmId::Control>(wId) };

				if (cId == WmId::Control::ApplyFilter) {
					flush_dns();
					return 0;
				}

				if (cId == WmId::Control::SaveEntry) {
					auto res{ FileInternals::save_data(listView_.data()) };

					if (!res) {
						handle_internal_error(res.error());
						return 0;
					}

					
					buttons_.enable_buttons(inner_settings_.changes_performed = false);
					update_self_title();

					Message::display(Message::Type::Info, Localization::Text::Info_FileSaveSuccess);

					return 0;
				}

				return 0;
			}
			case WM_CLOSE: {

				if (inner_settings_.changes_performed)
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

				change_setting(WmId::Custom::Request_Language_Change, static_cast<WmId::MenuBar>(wParam));
				Message::display(Message::Type::Info, Localization::Text::Info_LanguageUpdate);

				return 0;
			} 
			case (UINT)WmId::Custom::Request_Filter_Change: {
				
				change_setting(WmId::Custom::Request_Filter_Change, static_cast<WmId::MenuBar>(wParam));
				Message::display(Message::Type::Info, Localization::Text::Info_ListRowsUpdate);

				return 0;
			}
			case (UINT)WmId::Custom::Request_TextSize_Change: {
				change_setting(WmId::Custom::Request_TextSize_Change, static_cast<WmId::MenuBar>(wParam));
				return 0;
			}
			case (UINT)WmId::Custom::Request_List_Reload: {

				const auto hosts_int{ FileInternals::validate_hosts() };

				if (!hosts_int) {
					handle_internal_error(hosts_int.error());
					return 0;
				}

				if (inner_settings_.changes_performed)
					if (Message::display(Message::Type::Warning, Localization::Text::Warning_ReloadWillOverWrite) == IDNO)
						return 0;

				listView_.refresh_list(List::PopulateRows::FromFile);
				Message::display(Message::Type::Info, Localization::Text::Info_ListReloadUpdate);

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