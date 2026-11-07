#include "./system.hpp"

#include "./identifiers.hpp"
#include "./file_internals.hpp"

#include <CommCtrl.h>
#include <ShObjIdl.h>
#include <algorithm>

namespace System {
	
	MainWindow::MainWindow(HINSTANCE hInst) noexcept 
		: hInstance_{ hInst }, screenData_{}, inner_settings_{}, 
		listView_{}, menuBar_{}, buttons_{} { }

	HWND MainWindow::hwnd() const { return hwnd_; }

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

		const auto& [textsize, filters, changes] { inner_settings_ };

		screenData_.generate(hwnd_);

		menuBar_.update_settings(textsize, filters).create(hwnd_);

		listView_.create(hwnd_).populate_columns(
			ColumnData{ .name = get_text(Text::Lv_Line), .fmt = LVCFMT_CENTER,
				.size = screenData_ | From::Window | Dimension::Width | Percentage(5) },

			ColumnData{ .name = get_text(Text::Lv_Ip), .fmt = LVCFMT_CENTER,
			.size = screenData_ | From::Window | Dimension::Width | Percentage(5) },

			ColumnData{ .name = get_text(Text::Lv_Type), .fmt = LVCFMT_CENTER,
			.size = screenData_ | From::Window | Dimension::Width | Percentage(5) },

			ColumnData{ .name = get_text(Text::Lv_Domain), .fmt = LVCFMT_LEFT,
			.size = screenData_ | From::Window | Dimension::Width | Percentage(70) })
			.populate_rows(PopulateRows::FromFile)
			.set_font_height(textsize);

		buttons_.initialize(hwnd_, screenData_)
			.set_font_height(textsize).enable_buttons(changes);

	}

	void MainWindow::handle_file_backup() {
		auto result{ FileInternals::backup_file() };

		if (!result) {
			handle_internal_error(result.error());
			return;
		}

		Message::display(Message::Type::Info, Localization::Text::Info_HostsFileBacked);
	}

	// this function is a bit odd, why are we cleaning the hosts file instead of the current table... ?
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

	void MainWindow::handle_internal_error(const FileInternals::ErrorData& error) {

		auto [type, err] { error };

		[[maybe_unused]] int res{
			Message::display(type, Localization::TextMap<decltype(err)>{}.map(err))
		};
	}

	void MainWindow::flush_dns() {
		
		BOOL res{ };

		STARTUPINFO si{ };
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi{ };
		wchar_t cmd_args[]{ L"ipconfig.exe /flushdns" };

		res = CreateProcessW(NULL, cmd_args, NULL, NULL, 
			FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

		if (!res) {
			int mres{ Message::display(Message::Type::Warning, Localization::Text::Warning_CreateIPCProcessFailed) };

			if (mres == IDNO)
				PostQuitMessage(0);

			return;
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		Message::display(Message::Type::Info, Localization::Text::Info_FilterRulesRefreshed);

	}

	void MainWindow::save_data() {
		auto res{ FileInternals::save_data(listView_.data()) };

		if (!res) {
			handle_internal_error(res.error());
			return;
		}

		buttons_.enable_buttons(inner_settings_.changes_performed = false);
		update_self_title();
		listView_.refresh_list(List::PopulateRows::FromFile);

		Message::display(Message::Type::Info, Localization::Text::Info_FileSaveSuccess);
	}

	void MainWindow::update_self_title() {
		std::wstring title_text{ Localization::get_text(Localization::Text::MainMenuWindow) };

		if (inner_settings_.changes_performed)
			title_text = std::format(L"{} *", title_text);

		SetWindowTextW(hwnd_, title_text.c_str());
	}

	void MainWindow::change_setting(WmId::Custom setting, WmId::MenuBar value) {
		
		using namespace WmId;

		auto& [text_size, filter_modes, _] { inner_settings_ };

		if (setting == Custom::Request_Language_Change) {

			if (value == MenuBar::Language_Spanish)
				Language::set(Language::Type::Spanish);
			else
				Language::set(Language::Type::English);
			
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

			int nts{ value == MenuBar::Text_DecreaseSize ? -1 : 1 };

			text_size = std::clamp(text_size + nts, Constants::MinTextSize, Constants::MaxTextSize);

			nts = text_size + nts;

			if (nts < Constants::MinTextSize || 
				nts > Constants::MaxTextSize)
				MessageBeep(MB_OK);

			listView_.set_font_height(text_size);
			buttons_.set_font_height(text_size);

		}

		menuBar_.update_settings(text_size, filter_modes).create(hwnd_);
		
	}

	void MainWindow::resize_window_elements(std::int32_t nWidth, std::int32_t nHeight) {
		screenData_.regenerate_client_size(nWidth, nHeight);

		listView_.resize(screenData_);
		buttons_.resize(screenData_);
	}

	void MainWindow::handle_file_reload() {
		const auto hosts_int{ FileInternals::validate_hosts() };

		if (!hosts_int) {
			handle_internal_error(hosts_int.error());
			return;
		}

		if (inner_settings_.changes_performed)
			if (Message::display(Message::Type::Warning, Localization::Text::Warning_ReloadWillOverWrite) == IDNO)
				return;

		buttons_.enable_buttons(inner_settings_.changes_performed = false);
		update_self_title();
		listView_.refresh_list(List::PopulateRows::FromFile);
		Message::display(Message::Type::Info, Localization::Text::Info_ListReloadUpdate);
	}

	LRESULT MainWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
			case WM_CREATE: {
				prepare_ui();
				return 0;
			}
			case WM_NOTIFY: {
				LPNMHDR lpnmsg{ reinterpret_cast<LPNMHDR>(lParam) };

				if (lpnmsg->hwndFrom == listView_.hwnd()) 
					return listView_.message_handler(lParam);

				return 0;
			}
			case WM_SIZE: {
				resize_window_elements(LOWORD(lParam), HIWORD(lParam));
				return 0;
			}
			case WM_GETMINMAXINFO: {
				using namespace Screen;
				MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);

				mmi->ptMinTrackSize.x = screenData_ | From::Screen | Dimension::Width | Percentage(45);
				mmi->ptMinTrackSize.y = screenData_ | From::Screen | Dimension::Height | Percentage(50);

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
					menuBar_.message_handler(wId);
					return 0;
				}

				WmId::Control cId{ static_cast<WmId::Control>(wId) };

				if (cId == WmId::Control::ApplyFilter) {
					flush_dns();
				} 
				else if (cId == WmId::Control::SaveEntry) {
					save_data();
				} 
				else if (cId == WmId::Control::DiscardChanges) {
					handle_file_reload();
				} 
				else if (cId == WmId::Control::NewEntry) {
					SetFocus(listView_.hwnd());
					listView_.insert_new_entry();
				}
				else if (cId == WmId::Control::DeleteEntry) {
					listView_.delete_selected_entry();
				}

				if (cId != WmId::Control::NewEntry) {
					EnableWindow(buttons_.delete_, FALSE);
					SetFocus(hwnd_);
				}

				return 0;
			}
			case WM_CLOSE: {

				if (inner_settings_.changes_performed)
					if (Message::display(Message::Type::Warning,
						Localization::Text::Warning_ChangesPending) == IDNO)
							return 0;
				break;
			}
			case WM_DESTROY: {
				PostQuitMessage(0);
				return 0;
			}
			case WM_KEYDOWN: {
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
					if (wParam == 'S' && inner_settings_.changes_performed == true)
						save_data();
					else if (wParam == 'R')
						handle_file_reload();
				} 
				else if (wParam == VK_DOWN || wParam == VK_UP) {
					const HWND lv{ listView_.hwnd() };

					ListView_SetItemState(lv, 0, 
						LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

					ListView_EnsureVisible(lv, 0, FALSE);

					SetFocus(lv);
					return 0;
				}
				break;
			}
			case WM_MOUSEWHEEL: {

				auto vKey{ GET_KEYSTATE_WPARAM(wParam) };
				auto mDelta{ GET_WHEEL_DELTA_WPARAM(wParam) };

				if (vKey == MK_CONTROL) {
					if (mDelta > 0) {
						change_setting(WmId::Custom::Request_TextSize_Change, 
							WmId::MenuBar::Text_IncreaseSize);
					}
					else if (mDelta < 0) {
						change_setting(WmId::Custom::Request_TextSize_Change,
							WmId::MenuBar::Text_DecreaseSize);
					}
				}

				break;
			}

			case static_cast<UINT>(WmId::Custom::Request_Language_Change): {

				change_setting(WmId::Custom::Request_Language_Change, static_cast<WmId::MenuBar>(wParam));
				Message::display(Message::Type::Info, Localization::Text::Info_LanguageUpdate);

				return 0;
			} 
			case static_cast<UINT>(WmId::Custom::Request_Filter_Change): {
				
				change_setting(WmId::Custom::Request_Filter_Change, static_cast<WmId::MenuBar>(wParam));
				Message::display(Message::Type::Info, Localization::Text::Info_ListRowsUpdate);

				return 0;
			}
			case static_cast<UINT>(WmId::Custom::Request_TextSize_Change): {
				change_setting(WmId::Custom::Request_TextSize_Change, static_cast<WmId::MenuBar>(wParam));
				return 0;
			}
			case static_cast<UINT>(WmId::Custom::Request_List_Reload): {
				handle_file_reload();
				return 0;
			}

			case static_cast<UINT>(WmId::Custom::Request_Clear_File): {
				handle_file_cleanup();
				return 0;
			}

			case static_cast<UINT>(WmId::Custom::Request_Backup_File): {
				handle_file_backup();
				return 0;
			}
			case static_cast<UINT>(WmId::Custom::Request_Restore_File): {
				handle_file_restore();
				return 0;
			}
			case static_cast<UINT>(WmId::Custom::ListMutated): {
				buttons_.enable_buttons(inner_settings_.changes_performed = true);
				update_self_title();
				return 0;
			}
			case static_cast<UINT>(WmId::Custom::ListSelected): {

				auto idx{ lParam };
				EnableWindow(buttons_.delete_, idx != -1);

				return 0;
			}
			case static_cast<UINT>(WmId::Custom::ListError): {
				handle_internal_error(FileInternals::ErrorData{
					static_cast<Message::Type>(wParam),
					static_cast<FileInternals::Error>(lParam)
				});
				return 0;
			}
		}
		return DefWindowProcW(hwnd_, msg, wParam, lParam);
	}
}