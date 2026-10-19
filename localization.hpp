#pragma once

#include <string>
#include <cstddef>
#include <concepts>

namespace Language {

	enum class Type : std::size_t {
		English,
		Spanish,
		COUNT
	};

	Type get() noexcept;
	void set(Type lang) noexcept;
}

namespace Localization {

	// Keeping this ORDERED AS IS, is extremely important
	// for the strings, DO NOT CHANGE IT, unless you
	// want to change the array of strings that uses this
	// specific ordering.

	enum class Text : std::size_t {
		NO_TEXT,

		File_EmptyFile,
		File_BackupFile,
		File_RestoreFile,
		File_Exit,

		Text_ActualSize,
		Text_IncreaseSize,
		Text_DecreaseSize,

		Language_Spanish,
		Language_English,

		Filter_Comments,
		Filter_Blank,
		Filter_Invalid,
		Filter_All,

		Settings_Language,
		Settings_Text,
		Settings_ReloadFile,
		Settings_FilterEntries,

		Menu_Settings,
		Menu_File,

		MainMenuWindow,

		Lv_Line,
		Lv_Ip,
		Lv_Type,
		Lv_Domain,

		Ht_Ipv4,
		Ht_Ipv6,
		Ht_Comment,
		Ht_Blank,
		Ht_Invalid,

		MessageType_CriticalError,
		MessageType_Error,
		MessageType_Warning,
		MessageType_Info,
		MessageType_Confirm,
		
		MessageMisc_CriticalLabel,
		MessageMisc_ErrorLabel,
		MessageMisc_ContinueLabel,

		CriticalError_UIFailed,
		CriticalError_HandleFailed,

		Error_ScreenDimensionFailed,
		Error_ApplyFontFailed,
		Error_FontCreationFailed,
		
		Error_RoamingAppDataNotFound,

		Info_LanguageUpdate,
		Info_ListRowsUpdate,
		Info_ListReloadUpdate,
		
		Info_NoBackupsWithoutDir,
		Info_HostsFileBacked,
		Info_HostsFileCleaned,

		Warning_ReloadWillOverWrite,
		Warning_ChangesPending,
		Warning_FileDeletionIrrevertible,

		Warning_HostsFileMissing,
		Warning_HostsFileCopyFailed,
		Warning_HostsFileClearFailed,

		Confirm_CreateBackupDir,

		COUNT,
	};


	std::wstring_view get_text(Text);


	template <typename T>
	struct TextMap;

	template <typename T>
	requires requires(T val) { { TextMap<T>::map(val) } -> std::same_as<Text>; }
	std::wstring_view get_text(T value) {
		return get_text(TextMap<T>::map(value));
	}
}