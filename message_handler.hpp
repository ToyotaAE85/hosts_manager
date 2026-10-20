#pragma once

#include "./localization.hpp"

namespace Message {

	enum class Type {
		CriticalError,
		Error,
		Warning,
		Info,
		Confirm,
	};

	int display(Type type, Localization::Text message);

}

namespace Localization {
	template <>
	struct TextMap<Message::Type> {
		static constexpr Text map(Message::Type type) noexcept {
			switch (type) {
			case Message::Type::CriticalError:	return Text::MessageType_CriticalError;
			case Message::Type::Error:			return Text::MessageType_Error;
			case Message::Type::Warning:		return Text::MessageType_Warning;
			case Message::Type::Info:			return Text::MessageType_Info;
			case Message::Type::Confirm:		return Text::MessageType_Confirm;
			[[unlikely]] default:				return Text::NO_TEXT;
			}
		}
	};
}