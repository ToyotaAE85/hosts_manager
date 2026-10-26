#pragma once

// Both MIN and MAX indices should not
// be used

namespace WmId {

	enum class MenuBar {
		Min = 0x1000,

		File_EmptyFile,
		File_BackupFile,
		File_RestoreFile,
		File_Exit,

		Text_ActualSize,
		Text_IncreaseSize,
		Text_DecreaseSize,

		// filter flags in this order
		Filter_Invalid,
		Filter_Blank,
		Filter_Comments,
		Filter_All,

		// adding more settings past Language_English
		// requires adjusting
		// MainWindow::change_setting checks
		Language_Spanish,
		Language_English,

		Settings_ReloadFile,

		Max,
	};

	enum class Control {
		Min = 0x1100,

		DeleteEntry,
		NewEntry,
		SaveEntry,
		ApplyFilter,
		DiscardChanges,

		Max,
	};

	enum class Custom {
		Min = 0x8000, // WM_APP

		Request_Language_Change,
		Request_Filter_Change,
		Request_TextSize_Change,

		Request_List_Reload,
		Request_Clear_File,
		Request_Backup_File,
		Request_Restore_File,
		
		Max = 0xBFFF
	};
}
