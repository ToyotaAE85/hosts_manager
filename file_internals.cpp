#include "./file_internals.hpp"

#include "./auxiliary.hpp"
#include "./message_handler.hpp"

#include <ShlObj.h>
#include <system_error>
#include <format>
#include <filesystem>

namespace FileInternals {

	static std::filesystem::path hosts{ LR"(C:\Windows\System32\drivers\etc\hosts)" };

	enum class CreatePrompt {
		Yes,
		No
	};

	ReturnType<void> validate_hosts() {
		if (!std::filesystem::exists(hosts))
			return std::unexpected(ErrorData{ Message::Type::Warning, Error::HOSTS_FILE_MISSING });

		if (std::filesystem::file_size(hosts) == 0)
			return std::unexpected(ErrorData{ Message::Type::Info, Error::HOSTS_FILE_EMPTY });

		return {};
	}


	ReturnType<std::filesystem::path> get_backup_dir(CreatePrompt obj = CreatePrompt::Yes) {
		
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
			if (obj == CreatePrompt::Yes && 
				Message::display(Message::Type::Confirm, Localization::Text::Confirm_CreateBackupDir) == IDOK) {
				std::filesystem::create_directory(backup_dir);
			}
			else {
				return std::unexpected(ErrorData{ Message::Type::Info, Error::USER_DECLINED_CREATION });
			}

			return std::unexpected(ErrorData{ Message::Type::Info, Error::BACKUP_DIRECTORY_MISSING });
		}

		return backup_dir;
	}


	ReturnType<void> backup_exists() {

		auto dir{ get_backup_dir(CreatePrompt::No) };

		if (!dir)
			return std::unexpected(dir.error());

		if (!std::filesystem::is_directory(dir.value())) [[unlikely]]
			return std::unexpected(ErrorData{ Message::Type::Error, Error::BACKUP_NOT_DIRECTORY });

		for (const auto& entry : std::filesystem::directory_iterator(dir.value())) {
			if (entry.is_regular_file()) {
				std::wstring file_name{ entry.path().filename() };

				if (file_name.starts_with(L"hosts") && file_name.ends_with(L".bak") && entry.file_size() > 0)
					return {};
			}
		}
		
		return std::unexpected(ErrorData{ Message::Type::Warning, Error::BACKUP_NOT_FOUND });
	}


	ReturnType<void> backup_file() {

		auto hosts_res{ validate_hosts() };

		if (!hosts_res)
			return std::unexpected(hosts_res.error());

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

		auto hosts_res{ validate_hosts() };

		if (!hosts_res)
			return std::unexpected(hosts_res.error());

		if (Message::display(Message::Type::Warning,
			Localization::Text::Warning_FileDeletionIrrevertible) == IDNO) {
			return std::unexpected(ErrorData{Message::Type::Info, Error::USER_CLEANUP_ABORTED});
		}

		auto backups{ backup_exists() };

		if (!backups && backups.error().err == Error::BACKUP_NOT_FOUND) {
			auto& [type, err] { backups.error() };
			if (Message::display(type, Localization::TextMap<decltype(err)>::map(err)) == IDNO) {
				return std::unexpected(ErrorData{ Message::Type::Info, Error::USER_CLEANUP_ABORTED });
			}
		}

		std::error_code ec{ };
		std::filesystem::resize_file(hosts, 0, ec);

		if (ec) 
			return std::unexpected(ErrorData{ Message::Type::Warning, Error::HOSTS_FILE_MISSING });

		return {};
	}
}