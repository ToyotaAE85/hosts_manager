#include "./component.hpp"

#include "./system.hpp"
#include "./localization.hpp"
#include "./file_internals.hpp"

#include <ranges>
#include <algorithm>

namespace Component {
	HWND ListView::hwnd() const {
		return hwnd_;
	}

	ListView& ListView::create(HWND parent) {

		INITCOMMONCONTROLSEX icex{ };
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_LISTVIEW_CLASSES;

		InitCommonControlsEx(&icex);

		hwnd_ = Utility::create_window({
				.className = WC_LISTVIEW,
				.style = WS_CHILD | WS_VISIBLE | LVS_REPORT
						| LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_EDITLABELS,
				.hParent = parent,
				.id = reinterpret_cast<HMENU>(10001),
				.hInst = GetModuleHandleW(NULL),
			});

		ListView_SetExtendedListViewStyle(hwnd_,
			LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER
			| LVS_EX_COLUMNSNAPPOINTS | LVS_EX_TWOCLICKACTIVATE | LVS_EX_GRIDLINES
		);

		return *this;
	}


	auto ListView::retrieve_filtered_view() {

		using namespace System;

		auto pWnd{ reinterpret_cast<MainWindow*>(GetPropW(GetParent(hwnd_), L"MAINMENUWINDOW")) };

		if (!pWnd)
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_HandleFailed);

		// first 3 bits, [comments][blank][invalid] 
		const auto& [filter_modes] { pWnd->request_settings<Setting::FilterModes>() };

		auto vw{
			line_data_
			| std::views::enumerate
			| std::views::filter([filter_modes](const auto& p) {
				auto [_, line] { p };

				if ((filter_modes & (1 << 0)) &&
					line.type == FileInternals::Line::Type::INVALID)
					return false;

				if ((filter_modes & (1 << 1)) &&
					line.type == FileInternals::Line::Type::BLANK)
					return false;

				if ((filter_modes & (1 << 2)) &&
					line.type == FileInternals::Line::Type::COMMENT)
					return false;

				return true;
			})
		};

		return vw;
	}


	ListView& ListView::populate_rows(List::PopulateRows from) {

		if (from == List::PopulateRows::FromFile) {
			rewrite_data();
		}

		auto vw{ retrieve_filtered_view() };

		for (const auto& [idx, line] : vw) {

			auto cnt_text{ std::to_wstring(idx + 1) };

			LVITEMW lvItem{ };
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = static_cast<int>(idx);

			lvItem.iSubItem = 0;
			lvItem.pszText = const_cast<LPWSTR>(cnt_text.c_str());

			auto in_idx{ ListView_InsertItem(hwnd_, &lvItem) };

			lvItem.iItem = in_idx;
			lvItem.iSubItem = 1;
			lvItem.pszText = const_cast<LPWSTR>(line.ip.c_str());
			ListView_SetItem(hwnd_, &lvItem);

			lvItem.iItem = in_idx;
			lvItem.iSubItem = 2;
			lvItem.pszText = const_cast<LPWSTR>(Localization::get_text(line.type).data());
			ListView_SetItem(hwnd_, &lvItem);

			lvItem.iItem = in_idx;
			lvItem.iSubItem = 3;
			lvItem.pszText = const_cast<LPWSTR>(line.domains.c_str());
			ListView_SetItem(hwnd_, &lvItem);
		}

		return *this;
	}


	LRESULT ListView::MessageHandler(LPARAM) {
		//LPNMHDR lpmsg{ reinterpret_cast<LPNMHDR>(lParam) };

		//switch (lpmsg->code) {
		//	default: return 0;
		//}

		return 0;
	}


	void ListView::rewrite_data() {

		using namespace System;

		auto pWnd{ reinterpret_cast<MainWindow*>(GetPropW(GetParent(hwnd_), L"MAINMENUWINDOW")) };

		if (!pWnd)
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_HandleFailed);

		// Type::CriticalError calls for std::terminate so this is ok
		const auto& [file] { pWnd->request_settings<Setting::CurrentFile>() };

		auto data{ FileInternals::load_data(file)};

		if (!data) {
			
			// Deal with the error later
			return;
		}

		line_data_ = std::move(data.value());
	}


	void ListView::refresh_list(List::PopulateRows from) {
		ListView_DeleteAllItems(hwnd_);

		populate_rows(from);

		UpdateWindow(hwnd_);
	}


	void ListView::update_labels() {
		LVCOLUMN lvc = { };
		lvc.mask = LVCF_TEXT;

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Line).data());
		ListView_SetColumn(hwnd_, 0, &lvc);

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Ip).data());
		ListView_SetColumn(hwnd_, 1, &lvc);

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Type).data());
		ListView_SetColumn(hwnd_, 2, &lvc);

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Domain).data());
		ListView_SetColumn(hwnd_, 3, &lvc);

		auto vw{ retrieve_filtered_view() };

		for (const auto& [idx, line] : vw)
			ListView_SetItemText(hwnd_, idx, 2, const_cast<LPWSTR>(Localization::get_text(line.type).data()));

		UpdateWindow(hwnd_);
	}
}