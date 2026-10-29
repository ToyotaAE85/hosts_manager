#include "./localization.hpp"

#include <array>

namespace Language {

	Type current{ Type::English };

	Type get() noexcept {
		return current;
	}

	void set(Type lang) noexcept {
		if (lang >= Type::COUNT)
			return;

		current = lang;
	}
}

namespace Localization {

	using TranslationSet = std::array<std::wstring_view, static_cast<std::size_t>(Language::Type::COUNT)>;

	// The indices for this lookup table must remain THE exact same as that of Localization::Text
	// roughly speaking -> TextData[Localization::Text][Language::Type]

	constexpr static std::array<TranslationSet, static_cast<std::size_t>(Text::COUNT)> TextData{ {
		{L"", L""}, // NO_TEXT

		{L"Clear file", L"Vaciar archivo"},
		{L"Backup file", L"Crear copia de seguridad"},
		{L"Restore from backup...", L"Restaurar desde una copia de seguridad..."},
		{L"Exit", L"Salir"},

		{L"Size: ", L"Tamaño: "},
		{L"Increase", L"Aumentar"},
		{L"Decrease", L"Disminuir"},

		{L"Spanish", L"Español"},
		{L"English", L"Inglés"},

		{L"Filter comments", L"Filtrar comentarios"},
		{L"Filter blank", L"Filtrar vacias"},
		{L"Filter invalid", L"Filtrar no válidas"},
		{L"Invert filters", L"Invertir filtros"},

		{L"Language", L"Idioma"},
		{L"Text size", L"Tamaño del texto"},
		{L"Reload from hosts file", L"Recargar desde el archivo hosts"},
		{L"Filter...", L"Filtrar..."},

		{L"Settings", L"Configuración"},
		{L"File", L"Archivo"},

		{L"Degree Project | Hosts File Manager", L"Proyecto de titulación | Administrador del archivo hosts"},

		{L"Load backup file ", L"Cargar un archivo de respaldo"},
		{L"Backup files...", L"Copias de seguridad"},

		{L"Line", L"Línea"},
		{L"IP", L"IP"},
		{L"Type", L"Tipo"},
		{L"Domain(s)", L"Dominio(s)"},

		{L"IPV4", L"IPV4"},
		{L"IPV6", L"IPV6"},
		{L"Comment", L"Comentario"},
		{L"Blank", L"Vacio"},
		{L"Invalid", L"No valido"},

		{L"Critical Error", L"Error Critico"},
		{L"Error", L"Error"},
		{L"Warning", L"Advertencia"},
		{L"Info", L"Info"},
		{L"Confirm Action", L"Confirmar Accion"},

		//misc_
		{L"The application cannot continue and will close", L"El programa no puede continuar y se cerrará"},
		{L"The application may not function as expected", L"Es posible que la aplicación no funcione como se espera"},
		{L"Continue?", L"¿Continuar?"},

		//criterror_
		{L"UI element failed to initialize", L"Elemento de IU no se ha podido inicializar"},
		{L"Main window handle retrieval failed", L"No se pudo obtener el identificador de la ventana principal"},

		//error_
		{L"Screen dimensions retrieval failed", L"No se pudo obtener las dimensiones de la pantalla"},
		{L"Requested font couldn't be applied to UI", L"El tipo de letra solicitado no pudo ser aplicado a la IU"},
		{L"Requested font couldn't be initialized", L"El tipo de letra solicitado no pudo iniciarse"},

		{L"The Roaming AppData folder could not be found", L"No se pudo encontrar la carpeta Roaming de AppData"},
		{L"Backup directory failed expected type, not a directory", L"No se esperaba el tipo de la carpeta para copias de seguridad, se esperaba una carpeta"},

		//info_
		{L"Language updated", L"Idioma actualizado"},
		{L"List rows filters updated", L"Filtros de las filas de la lista actualizadas"},
		{L"The list has been reloaded from the hosts file", L"La lista ha sido recargada desde el archivo hosts."},

		{L"You won't be able to perform/load backups without a valid backup directory", L"No podrá realizar/cargar copias de seguridad sin un directorio de copias de seguridad válido"},
		{L"Hosts file has been backed up", L"Se le ha elaborado una copia de seguridad al archivo hosts"},
		{L"Hosts file has been cleared", L"El archivo hosts ha sido vaciado"},
		{L"Hosts file is currently empty, requested operation ignored", L"El archivo hosts esta vacio, operacion solicitada ignorada"},
		{L"The backup directory is missing", L"No se encuentra el directorio de copias de seguridad"},

		{L"Operation aborted by user", L"La operacion ha sido abortada por el usuario"},

		{L"The selected backup file has been loaded into memory", L"La copia de seguridad seleccionada ha sido cargada en la memoria"},

		{L"Couldn't write to hosts file, file might be busy", L"No se pudo escribir al archivo hosts, el archivo puede estar ocupado"},
		{L"Couldn't open requested/hosts file, check program permissions", L"No se pudo abrir el archivo hosts/seleccionado, verifique los permisos de la aplicacion"},
		{L"Couldn't find requested file", L"No se pudo encontrar el archivo seleccionado"},
		{L"Data successfully saved into the hosts file", L"Se ha guardado correctamente la informacion al archivo hosts"},

		{L"Hosts file rules have been applied successfully, note: applications might store previous hosts rules.", L"Se han aplicado con éxito las reglas del archivo hosts, nota: es posible que las aplicaciones almacenen reglas de hosts anteriores."},

		//warning_
		{L"Reloading the list will overwrite any unsaved changes", L"Recargar la lista sobreescribira cambios no guardados"},
		{L"Restore from a backup will overwrite any unsaved changes performed on the list", L"Restaurar desde una copia de seguridad sobreescribira cambios no guardados hechos a la lista"},
		{L"Pending changes detected, they will be permanently lost if you don't save", L"Se detectaron cambios pendientes, se perderán permanentemente si no se guardan"},
		{L"This operation will erase the contents of the hosts file, this is irreversible", L"Esta operación borrará el contenido del archivo hosts, esto es irreversible"},

		{L"Couldn't locate hosts file in its default directory", L"No se pudo localizar el archivo hosts en su directorio por default"},
		{L"Couldn't copy the hosts file, verify file permissions", L"No se pudo copiar el archivo hosts, revise los permisos del archivo"},
		{L"Couldn't clear the hosts file, verify file permissions", L"No se pudo vaciar el archivo hosts, revise los permisos del archivo"},
		{L"No backups have been located, proceed with caution", L"No se han encontrado copias de seguridad, proceda con precaucion"},

		{L"Create process operation failed, current hosts file rules couldn't be applied, verify program permissions", L"Error en la operación de creación del proceso, no se pudieron aplicar las reglas actuales del archivo hosts, verifique los permisos del programa."},

		//confirm_
		{L"Backup directory not found, would you like to create one?", L"No se encontró el directorio para copias de seguridad, ¿desea crear uno?"},
		{L"", L""},
		{L"", L""},

		//control buttons
		{L"Delete entry", L"Eliminar entrada"},
		{L"New entry", L"Nueva entrada"},
		{L"Save changes", L"Guardar cambios"},
		{L"Apply filter", L"Aplicar filtro"},
		{L"Discard changes", L"Descartar cambios"},
	}};


	std::wstring_view get_text(Text id) {
		if (id >= Text::COUNT)
			return L"Invalid String ID Requested!";

		return TextData[static_cast<std::size_t>(id)][static_cast<std::size_t>(Language::get())];
	}
}
