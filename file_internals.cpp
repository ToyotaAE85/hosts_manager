#include "./file_internals.hpp"

#include "./auxiliary.hpp"

#include <ShlObj.h>
#include <system_error>
#include <format>

namespace FileInternals {

	const std::filesystem::path hosts{ LR"(C:\Windows\System32\drivers\etc\hosts)" };

	ReturnType<std::filesystem::path> get_backup_dir() {
		
		std::filesystem::path dir_name{ "DegreeProjectBackup" };
		std::filesystem::path backup_dir{ };

		LPWSTR expanded_dir{ };

		if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, static_cast<DWORD>(0), nullptr, &expanded_dir) != S_OK) {
			
			CoTaskMemFree(expanded_dir);
			return std::unexpected(ErrorData{ Message::Type::Error, Error::ROAMING_APPDATA_NOT_FOUND });
		}

		backup_dir = std::filesystem::path(expanded_dir) / dir_name;
		CoTaskMemFree(expanded_dir);

		if (!std::filesystem::exists(backup_dir)) {
			if (Message::display(Message::Type::Confirm, Localization::Text::Confirm_CreateBackupDir) == IDOK) {
				std::filesystem::create_directory(backup_dir);
			}
			else {
				return std::unexpected(ErrorData{ Message::Type::Info, Error::USER_DECLINED_CREATION });
			}
		}

		return backup_dir;
	}


	ReturnType<void> backup_exists() {
		return {};
	}


	ReturnType<void> backup_file() {

		if (!std::filesystem::exists(hosts))
			return std::unexpected(ErrorData{ Message::Type::Warning, Error::HOSTS_FILE_MISSING });

		auto backup_dir{ get_backup_dir() };

		if (!backup_dir)
			return std::unexpected(backup_dir.error());

		std::error_code ec{ };
		std::filesystem::copy_file(hosts, backup_dir.value() 
			/ std::format(L"hosts{}.bak", Auxiliary::Time::stamp()), ec);

		if (ec)
			return std::unexpected(ErrorData{ Message::Type::Warning, Error::HOSTS_FILE_COPY_FAILED });

		return {};
	}


	ReturnType<void> delete_file_contents() {

		if (!std::filesystem::exists(hosts))
			return std::unexpected(ErrorData{ Message::Type::Warning, Error::HOSTS_FILE_MISSING });
		
		std::error_code ec{ };
		std::filesystem::resize_file(hosts, 0, ec);

		if (ec) 
			return std::unexpected(ErrorData{ Message::Type::Warning, Error::HOSTS_FILE_MISSING });

		return {};
	}
}