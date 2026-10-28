#pragma once

#include "./localization.hpp"
#include "./message_handler.hpp"

#include <expected>
#include <string>
#include <vector>
#include <filesystem>


namespace FileInternals {

	inline const std::filesystem::path dir_name{ "DegreeProjectBackup" };

	inline std::filesystem::path hosts{ LR"(C:\Windows\System32\drivers\etc\hosts)" };


	enum class CreateDirectoryPrompt {
		Yes,
		No
	};

	enum class Error {
		USER_DECLINED_CREATION,
		ROAMING_APPDATA_NOT_FOUND,

		HOSTS_FILE_MISSING,
		HOSTS_FILE_COPY_FAILED,
		HOSTS_FILE_CLEAR_FAILED,
		HOSTS_FILE_EMPTY,

		BACKUP_NOT_DIRECTORY,
		BACKUP_NOT_FOUND,
		BACKUP_DIRECTORY_MISSING,

		USER_CLEANUP_ABORTED,

		FILE_WRITE_FAILED,
		FILE_OPEN_FAILED,
		FILE_MISSING,
	};

	struct ErrorData {
		Message::Type type{ };
		Error err{ };
	};

	namespace Line {
		enum class Type {
			IPV4,
			IPV6,
			COMMENT,
			BLANK,
			INVALID,
			DELETED
		};

		struct Data {
			std::wstring domains;
			std::wstring ip;
			Type type;
		};
	}

	template<typename T>
	using ReturnType = std::expected<T, ErrorData>;

	ReturnType<void> validate_hosts();

	ReturnType<void> backup_file();

	ReturnType<void> delete_file_contents();

	ReturnType<std::filesystem::path> get_backup_dir(CreateDirectoryPrompt obj = CreateDirectoryPrompt::Yes);

	// might need to refactor this weirdness..
	// set a target file then use this as a load_data parameter... ????
	// this function should be strictly used as a parameter for load_data mind you(us)
	std::filesystem::path from_target_file();

	ReturnType<void> set_target_file(std::filesystem::path new_path);

	ReturnType<Line::Data> revalidate_line_data(const Line::Data& data);

	ReturnType<std::vector<Line::Data>> load_data(std::filesystem::path path = L"");

	ReturnType<void> save_data(const std::vector<Line::Data>& data);
}


namespace Localization {

	template <>
	struct TextMap<FileInternals::Line::Type> {
		static constexpr Text map(FileInternals::Line::Type type) noexcept {
			switch (type) {
			
				case FileInternals::Line::Type::IPV4:    return Text::Ht_Ipv4;
				case FileInternals::Line::Type::IPV6:    return Text::Ht_Ipv6;
				case FileInternals::Line::Type::COMMENT: return Text::Ht_Comment;
				case FileInternals::Line::Type::BLANK:   return Text::Ht_Blank;
				case FileInternals::Line::Type::INVALID: return Text::Ht_Invalid;
			
				[[unlikely]] default:            return Text::NO_TEXT;
			}
		}
	};

	template<>
	struct TextMap<FileInternals::Error> {
		static constexpr Text map(FileInternals::Error type) noexcept {
			switch (type) {

				case FileInternals::Error::ROAMING_APPDATA_NOT_FOUND:	return Text::Error_RoamingAppDataNotFound;
				case FileInternals::Error::BACKUP_NOT_DIRECTORY:		return Text::Error_BackupNotDirectory;
				case FileInternals::Error::USER_DECLINED_CREATION:		return Text::Info_NoBackupsWithoutDir;
				case FileInternals::Error::HOSTS_FILE_EMPTY:			return Text::Info_HostsFileEmpty;
				case FileInternals::Error::BACKUP_DIRECTORY_MISSING:	return Text::Info_BackupDirectoryMissing;
				case FileInternals::Error::HOSTS_FILE_MISSING:			return Text::Warning_HostsFileMissing;
				case FileInternals::Error::HOSTS_FILE_COPY_FAILED:		return Text::Warning_HostsFileCopyFailed;
				case FileInternals::Error::HOSTS_FILE_CLEAR_FAILED:		return Text::Warning_HostsFileClearFailed;
				case FileInternals::Error::BACKUP_NOT_FOUND:			return Text::Warning_BackupNotFound;
				case FileInternals::Error::USER_CLEANUP_ABORTED:		return Text::Info_UserCleanUpAborted;

				case FileInternals::Error::FILE_WRITE_FAILED:			return Text::Info_FileWriteFailed;
				case FileInternals::Error::FILE_OPEN_FAILED:			return Text::Info_FileOpenFailed;
				case FileInternals::Error::FILE_MISSING:				return Text::Info_FileMissing;

				[[unlikely]] default:									return Text::NO_TEXT;
			}

		}
	};
}