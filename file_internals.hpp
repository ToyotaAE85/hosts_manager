#include "./win32api.hpp"

#include "./localization.hpp"
#include "./component.hpp"

#include <expected>

namespace FileInternals {

	enum class Error {
		USER_DECLINED_CREATION,
		ROAMING_APPDATA_NOT_FOUND,

		HOSTS_FILE_MISSING,
		HOSTS_FILE_COPY_FAILED,
		HOSTS_FILE_CLEAR_FAILED,
	};

	struct ErrorData {
		Message::Type type{ };
		Error err{ };
	};

	template<typename T>
	using ReturnType = std::expected<T, ErrorData>;

	ReturnType<void> backup_exists();

	ReturnType<void> backup_file();

	ReturnType<void> delete_file_contents();
}

template<>
struct Localization::TextMap<FileInternals::Error> {
	static constexpr Localization::Text map(FileInternals::Error type) noexcept {
		switch (type) {
			case FileInternals::Error::ROAMING_APPDATA_NOT_FOUND:	return Localization::Text::Error_RoamingAppDataNotFound;
			case FileInternals::Error::USER_DECLINED_CREATION:		return Localization::Text::Info_NoBackupsWithoutDir;
			case FileInternals::Error::HOSTS_FILE_MISSING:			return Localization::Text::Warning_HostsFileMissing;
			case FileInternals::Error::HOSTS_FILE_COPY_FAILED:		return Localization::Text::Warning_HostsFileCopyFailed;
			case FileInternals::Error::HOSTS_FILE_CLEAR_FAILED:		return Localization::Text::Warning_HostsFileClearFailed;
			
			default:												return Localization::Text::NO_TEXT;
		}

	}
};