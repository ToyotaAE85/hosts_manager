#include "./list_view.hpp"

#include "./localization.hpp"
#include "./win32_utility.hpp"
#include "./identifiers.hpp"

#include <ranges>
#include <algorithm>
#include <string>

#include <cassert>
#include <memory>

//i just noticed theres a 260 char limit... dang it..

namespace List {

	HWND ListView::hwnd() const {
		return hwnd_;
	}

	ListView& ListView::create(HWND parent) {

		INITCOMMONCONTROLSEX icex{ };
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_LISTVIEW_CLASSES;

		if (!InitCommonControlsEx(&icex))
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_UIFailed);

		hwnd_ = Win32Utility::create_window({
				.className = WC_LISTVIEW,
				.style = WS_CHILD | WS_VISIBLE | LVS_REPORT
						 | LVS_NOSORTHEADER,
				.hParent = parent,
				.id = (std::uint64_t)WmId::ListView::List,
				.hInst = GetModuleHandleW(NULL),
			});

		ListView_SetExtendedListViewStyle(hwnd_,
			LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER
			| LVS_EX_COLUMNSNAPPOINTS | LVS_EX_GRIDLINES
		);

		return *this;
	}

	ListView& ListView::update_filters(std::uint32_t filter_modes) {
		filter_modes_ = filter_modes;
		return *this;
	}

	auto ListView::retrieve_filtered_view() const {

		auto vw{
			line_data_
			| std::views::enumerate
			| std::views::filter([this](const auto& p) {
				const auto& [_, line] { p };

				if (line.type == FileInternals::Line::Type::DELETED)
					return false;

				if ((filter_modes_ & (1 << 0)) &&
					line.type == FileInternals::Line::Type::INVALID)
					return false;

				if ((filter_modes_ & (1 << 1)) &&
					line.type == FileInternals::Line::Type::BLANK)
					return false;

				if ((filter_modes_ & (1 << 2)) &&
					line.type == FileInternals::Line::Type::COMMENT)
					return false;

				return true;
			})
		};

		return vw;
	}

	std::int32_t ListView::cell_to_vector_index(std::int32_t idx_cell) const {

		std::int32_t idx{ };

		std::wstring idb;
		idb.resize(11);

		ListView_GetItemText(hwnd_, idx_cell, 0, idb.data(), 10);

		try {
			idx = std::stoi(idb);
		}
		catch (...) {
			Message::display(Message::Type::Error, Localization::Text::Error_IndexParsingFailed);
			return -1;
		}

		// cell_idx contains 1-indexed element, hence -1 needed here.

		return idx - 1;
	}

	void ListView::calculate_edit_control(edit_state& state) {

		auto& [_, itm, sitm, entryidx, __] { state };

		RECT sl_rect{ };
		ListView_GetSubItemRect(hwnd_, itm, sitm, LVIR_BOUNDS, &sl_rect);

		std::wstring buffer{  };

		if (sitm == 1)
			buffer = line_data_[entryidx].ip;
		else
			buffer = line_data_[entryidx].domains;

		if (!edit_) {
			edit_ = Win32Utility::create_window({
				.exStyle = WS_EX_CLIENTEDGE,
				.className = L"EDIT",
				.windowName = L"",
				.style = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
				.x = sl_rect.left,
				.y = sl_rect.top,
				.width = sl_rect.right - sl_rect.left,
				.height = sl_rect.bottom - sl_rect.top,
				.hParent = hwnd_,
				.id = static_cast<std::uint64_t>(WmId::ListView::DynamicEdit),
				.hInst = GetModuleHandleW(NULL),
				});
		}
		else {
			SetWindowPos(
				edit_,
				nullptr,
				sl_rect.left,
				sl_rect.top,
				sl_rect.right - sl_rect.left,
				sl_rect.bottom - sl_rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE
			);
		}

		SetWindowTextW(edit_, buffer.data());

		HFONT hFont{ reinterpret_cast<HFONT>(SendMessageW(hwnd_, WM_GETFONT, 0, 0)) };
		SendMessageW(edit_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

		SendMessageW(edit_, EM_SETSEL, 0, -1);
		SetFocus(edit_);
	}

	void ListView::enable_edit_controls(std::int32_t iitem, std::int32_t isitem) {

		edit_state* obj{ new edit_state };

		active_edit_ = obj;
		auto& [instance, itm, sitm, entryidx, _] { *obj };

		instance = this;
		itm = iitem;
		sitm = isitem;

		entryidx = cell_to_vector_index(itm);

		if (entryidx < 0) {
			active_edit_ = nullptr;
			delete obj;
			return;
		}

		calculate_edit_control(*obj);

		if (!SetWindowSubclass(edit_, ListView::edit_sub_class_procc, 0, reinterpret_cast<DWORD_PTR>(obj))) {
			active_edit_ = nullptr;
			delete obj;
		}
	}

	ListView& ListView::populate_rows(List::PopulateRows from) {

		if (from == List::PopulateRows::FromFile)
			rewrite_data();
		
		auto vw{ retrieve_filtered_view() };
		list_idx_ = 0;

		SendMessageW(hwnd_, WM_SETREDRAW, FALSE, 0);

		for (const auto& [l_idx, line] : vw)
			insert_internal_entry(l_idx + 1, line);

		SendMessageW(hwnd_, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hwnd_, nullptr, TRUE);
		
		return *this;
	}

	LRESULT ListView::message_handler(LPARAM lParam) {
		LPNMHDR lpmsg{ reinterpret_cast<LPNMHDR>(lParam) };

		switch (lpmsg->code) {
			case NM_DBLCLK: {
				LPNMITEMACTIVATE item{ reinterpret_cast<LPNMITEMACTIVATE>(lParam) };

				if (item->iItem != -1 && (item->iSubItem == 1 || item->iSubItem == 3)) {
					enable_edit_controls(item->iItem, item->iSubItem);
				}
				else if (item->iItem == -1) { // a bit odd that this would insert an entry...
					insert_new_entry();
				}

				break;
			}
			case LVN_ITEMCHANGED: {
				LPNMLISTVIEW lpItem{ reinterpret_cast<LPNMLISTVIEW>(lParam) };

				if ((lpItem->uChanged & LVIF_STATE) 
					&& (lpItem->uNewState & LVIS_SELECTED) 
					!= (lpItem->uOldState & LVIS_SELECTED)) {

					std::int32_t idx{ ListView_GetNextItem(hwnd_, -1, LVNI_SELECTED) };

					PostMessageW(GetParent(hwnd_), static_cast<UINT>(WmId::Custom::ListSelected), 0, idx);
				}

				break;
			}
			case LVN_BEGINSCROLL: {
				if (edit_ != nullptr)
					DestroyWindow(edit_);
				
				break;
			}
			case LVN_KEYDOWN: {
				LPNMLVKEYDOWN key{ reinterpret_cast<LPNMLVKEYDOWN>(lParam) };

				if (key->wVKey == VK_DELETE) {
					delete_selected_entry();
				}
				else if (key->wVKey == VK_ESCAPE) {
					SetFocus(GetParent(hwnd_));
				}
				else if (key->wVKey == VK_RETURN) {
					if (ListView_GetSelectedCount(hwnd_) == 1) {

						const std::int32_t item{ ListView_GetNextItem(hwnd_, -1, LVNI_SELECTED) };

						if (item != -1)
							enable_edit_controls(item, 3);

					}
				}
				else if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
					if (key->wVKey == 'X') 
						delete_selected_entry();
					else if (key->wVKey == 'N')
						insert_new_entry();	
				}
				break;
			}
			case NM_KILLFOCUS: {
				ListView_SetItemState(hwnd_, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
				break;
			}
		}
		return 0;
	}

	ListView& ListView::set_font_height(std::int32_t new_height) {

		if (font_data_.empty())
			assign_font_data();
		
		for (auto& font : font_data_) {
			font.data.font_height = new_height;

			Win32Utility::apply_font(font);
		}

		if (edit_ && active_edit_)
			calculate_edit_control(*active_edit_);

		return *this;
	}

	void ListView::assign_font_data() {

		Win32Utility::FontControl ctrl{};
		auto& [font, target, data] { ctrl };

		target = hwnd_;
		font_data_.push_back(ctrl);

		target = ListView_GetHeader(hwnd_);
		data.font_weight = 700;
		font_data_.push_back(ctrl);
	}

	void ListView::rewrite_data() {

		auto data{ FileInternals::load_data(FileInternals::from_target_file()) };

		if (!data) {
			SendMessageW(GetParent(hwnd_), static_cast<UINT>(WmId::Custom::ListError), 
				static_cast<WPARAM>(data.error().type), static_cast<LPARAM>(data.error().err));
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

		auto cmp_idx{ 0 };
		for (const auto& [idx, line] : vw)
			ListView_SetItemText(hwnd_, cmp_idx++, 2, const_cast<LPWSTR>(Localization::get_text(line.type).data()));

		UpdateWindow(hwnd_);
	}

	void ListView::update_entry(const edit_state& data, std::wstring_view str) {
		
		const auto& [lv, lvidx, lvsidx, vcidx, _] { data };

		if (static_cast<std::size_t>(vcidx) >= line_data_.size()) 
			return;

		auto& [domains, ip, type] { line_data_[vcidx] };

		std::wstring temp_dom{ domains };
		std::wstring temp_ip{ ip };

		if (lvsidx == 3) {

			if (domains == str)
				return;

			domains.assign(str);
		}
		else if (lvsidx == 1) {

			if (ip == str)
				return;

			ip.assign(str);

		}
		else [[unlikely]]
			return;

		auto res{ FileInternals::revalidate_line_data(line_data_[vcidx]) };

		if (!res) {
			ip = temp_ip;
			domains = temp_dom;
			Message::display(Message::Type::Warning, Localization::Text::Warning_LineRevalidationFailed);
			return;
		}

		line_data_[vcidx] = res.value();

		ListView_SetItemText(hwnd_, lvidx, 1, ip.data());
		ListView_SetItemText(hwnd_, lvidx, 2, const_cast<LPWSTR>(Localization::get_text(type).data()));
		ListView_SetItemText(hwnd_, lvidx, 3, domains.data());

		if (entry_is_filtered(type))
			ListView_DeleteItem(hwnd_, lvidx);

		PostMessageW(GetParent(hwnd_), static_cast<UINT>(WmId::Custom::ListMutated), 0, 0);
	}

	const std::vector<FileInternals::Line::Data>& ListView::data() const {
		return line_data_;
	}

	void ListView::resize(Screen::Data screen) {
		using namespace Screen;
			
		HDWP hDwp{ BeginDeferWindowPos(1) };

		if (!hDwp) {
			resize_fallback(screen);
			return;
		}

		hDwp = DeferWindowPos(
			hDwp, hwnd_, nullptr,
			screen | From::Window | Dimension::Width | Percentage(5),
			screen | From::Window | Dimension::Height | Percentage(5),
			screen | From::Window | Dimension::Width | Percentage(90),
			screen | From::Window | Dimension::Height | Percentage(80),
			SWP_NOZORDER | SWP_NOACTIVATE
		);
		
		if (hDwp != nullptr)
			EndDeferWindowPos(hDwp);
		else
			resize_fallback(screen);

		if (edit_ && active_edit_)
			calculate_edit_control(*active_edit_);
	}

	void ListView::resize_fallback(Screen::Data screen) {
		using namespace Screen;

		MoveWindow(
			hwnd_,
			screen | From::Window | Dimension::Width | Percentage(5),
			screen | From::Window | Dimension::Height | Percentage(5),
			screen | From::Window | Dimension::Width | Percentage(90),
			screen | From::Window | Dimension::Height | Percentage(80),
			TRUE
		);
	}

	void ListView::insert_internal_entry(std::int64_t shownidx, const FileInternals::Line::Data& element) {

		const auto& [domains, ip, type] { element };

		auto cnt_text{ std::to_wstring(shownidx) };

		LVITEMW lvItem{ };
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = list_idx_++;
		lvItem.iSubItem = 0;
		lvItem.pszText = cnt_text.data();

		int inserted_idx{ ListView_InsertItem(hwnd_, &lvItem) };

		if (inserted_idx == -1)
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_UIFailed);

		ListView_SetItemText(hwnd_, inserted_idx, 1, const_cast<LPWSTR>(ip.data()));

		auto type_text{ Localization::get_text(type).data() };
		ListView_SetItemText(hwnd_, inserted_idx, 2, const_cast<LPWSTR>(type_text));

		std::wstring domain_range{ domains };

		if (domain_range.size() > 255)
			domain_range = std::format(L"{:.255s}...", domain_range);

		ListView_SetItemText(hwnd_, inserted_idx, 3, domain_range.data());
	}

	bool ListView::entry_is_filtered(FileInternals::Line::Type type) {
		switch (type) {
			case FileInternals::Line::Type::INVALID:	return filter_modes_ & 1;
			case FileInternals::Line::Type::BLANK:		return filter_modes_ & (1 << 1);
			case FileInternals::Line::Type::COMMENT:	return filter_modes_ & (1 << 2);
			default: return false;
		}
	}

	void ListView::delete_selected_entry() {

		int highest_selected{ -1 };

		int current_search{ -1 };
		while ((current_search = ListView_GetNextItem(hwnd_, current_search, LVNI_SELECTED)) != -1)
			highest_selected = current_search;
		
		if (highest_selected == -1)
			return;

		SendMessageW(hwnd_, WM_SETREDRAW, FALSE, 0);

		for (auto item{ highest_selected }; item > -1; item--) {
			if (ListView_GetItemState(hwnd_, item, LVIS_SELECTED) & LVIS_SELECTED) {

				std::int32_t idx{ cell_to_vector_index(item) };
				line_data_[idx].type = FileInternals::Line::Type::DELETED;

				ListView_DeleteItem(hwnd_, item);
			}
		}

		SendMessageW(hwnd_, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hwnd_, NULL, TRUE);

		PostMessageW(GetParent(hwnd_), static_cast<UINT>(WmId::Custom::ListMutated), 0, 0);
	}

	void ListView::insert_new_entry() {
		line_data_.push_back({ .ip = L"0.0.0.0", .type = FileInternals::Line::Type::IPV4 });
		ListView_SetItemState(hwnd_, -1, 0, LVIS_SELECTED);

		if (entry_is_filtered(line_data_.back().type))
			Message::display(Message::Type::Info, Localization::Text::Info_NewEntryAddedWithFilters);
		else
			insert_internal_entry(line_data_.size(), line_data_.back());

		const std::int32_t idx{ ListView_GetItemCount(hwnd_) };

		ListView_SetItemState(hwnd_, idx - 1, LVIS_SELECTED, LVIS_SELECTED);
		ListView_EnsureVisible(hwnd_, idx - 1, FALSE);

		PostMessageW(GetParent(hwnd_), static_cast<UINT>(WmId::Custom::ListMutated), 0, 0);
	}

	LRESULT CALLBACK ListView::edit_sub_class_procc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, 
		UINT_PTR, DWORD_PTR dwRefData) {

		edit_state* obj{ reinterpret_cast<edit_state*>(dwRefData) };

		if (!obj)
			DestroyWindow(hwnd);

		auto commit_changes{
			[&] {
				
				if (!obj || !obj->instance_ || obj->is_commit_active)
					return;
				
				obj->is_commit_active = true;

				const int buffer_size = GetWindowTextLengthW(hwnd);
				std::wstring buffer;

				if (buffer_size > 0) {
					buffer.resize(buffer_size + 1);

					GetWindowTextW(hwnd, buffer.data(), buffer_size + 1);
					buffer.resize(buffer_size);
				}

				obj->instance_->update_entry(*obj, buffer);

				obj->is_commit_active = false;
			}
		};

		auto modify_control{
			[&](List::EditControlPress dir) {

				using enum List::EditControlPress;

				if (!obj || !obj->instance_)
					return;

				auto& [instance, item, sbitem, idx, _] { *obj };

				if (dir == Tab) {
					sbitem ^= 2;
				}
				else if (dir == UpArrow || dir == DownArrow) {
					item = std::clamp(item + static_cast<int>(dir), 
						0, ListView_GetItemCount(instance->hwnd()) - 1);
				}

				idx = obj->instance_->cell_to_vector_index(item);
				instance->calculate_edit_control(*obj);	
			}
		};

		switch (msg) {

			case WM_CHAR: {
				if (wParam == VK_TAB)
					return 0;

				break;
			}
			case WM_KEYDOWN: {
					
				if (wParam == VK_RETURN) {
					commit_changes();

					SetFocus(GetParent(hwnd));
					return 0;
				}
				else if (wParam == VK_ESCAPE) {
					DestroyWindow(hwnd);
					return 0;
				}
				else if (wParam == VK_TAB) {
					commit_changes();

					modify_control(List::EditControlPress::Tab);
					return 0;
				}
				else if (wParam == VK_UP) {
					commit_changes();

					modify_control(List::EditControlPress::UpArrow);
					return 0;
				}
				else if (wParam == VK_DOWN) {
					commit_changes();

					modify_control(List::EditControlPress::DownArrow);
					return 0;
				}

				break;
			}
			case WM_KILLFOCUS: {
				DestroyWindow(hwnd);
				break;
			}
			case WM_DESTROY: {

				RemoveWindowSubclass(hwnd, ListView::edit_sub_class_procc, 0);

				if (obj && obj->instance_) {
					const HWND lv{ obj->instance_->hwnd() };

					obj->instance_->edit_ = NULL;

					ListView_SetItemState(lv, obj->item_, LVIS_SELECTED | LVIS_FOCUSED, 
						LVIS_SELECTED | LVIS_FOCUSED);
					ListView_EnsureVisible(lv, obj->item_, FALSE);

					obj->instance_->active_edit_ = nullptr;

					delete obj;
				}

				return 0;
			}
		}

		return DefSubclassProc(hwnd, msg, wParam, lParam);
	}
}