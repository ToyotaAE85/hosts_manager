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
        {L"Filter blank", L"Filtrar líneas vacías"},
        {L"Filter invalid", L"Filtrar líneas no válidas"},
        {L"Invert filters", L"Invertir filtros"},

        {L"Language", L"Idioma"},
        {L"Text size", L"Tamaño del texto"},
        {L"Reload from hosts file", L"Recargar desde el archivo hosts"},
        {L"Filter...", L"Filtrar..."},

        {L"Settings", L"Configuración"},
        {L"File", L"Archivo"},

        {L"Degree Project | Hosts File Manager", L"Proyecto de titulación | Administrador del archivo hosts"},

        {L"Load backup file ", L"Cargar archivo de copia de seguridad"},
        {L"Backup files...", L"Copias de seguridad..."},

        {L"Line", L"Línea"},
        {L"IP", L"IP"},
        {L"Type", L"Tipo"},
        {L"Domain(s)", L"Dominio(s)"},

        {L"IPV4", L"IPv4"},
        {L"IPV6", L"IPv6"},
        {L"Comment", L"Comentario"},
        {L"Blank", L"Vacío"},
        {L"Invalid", L"No válido"},

        {L"Critical Error", L"Error crítico"},
        {L"Error", L"Error"},
        {L"Warning", L"Advertencia"},
        {L"Info", L"Información"},
        {L"Confirm Action", L"Confirmar acción"},

        // misc_
        {L"The application cannot continue and will close", L"La aplicación no puede continuar y se cerrará"},
        {L"The application may not function as expected", L"Es posible que la aplicación no funcione correctamente"},
        {L"Continue?", L"¿Desea continuar?"},

        // criterror_
        {L"UI element failed to initialize", L"No se pudo inicializar un elemento de la interfaz de usuario"},
        {L"Main window handle retrieval failed", L"No se pudo obtener el identificador de la ventana principal"},

        // error_
        {L"Screen dimensions retrieval failed", L"No se pudieron obtener las dimensiones de la pantalla"},
        {L"Requested font couldn't be applied to UI", L"No se pudo aplicar la fuente solicitada a la interfaz de usuario"},
        {L"Requested font couldn't be initialized", L"No se pudo inicializar la fuente solicitada"},

        {L"The Roaming AppData folder could not be found", L"No se pudo encontrar la carpeta Roaming de AppData"},
        {L"Backup directory failed expected type, not a directory", L"La ruta de las copias de seguridad no corresponde a un directorio"},

        {L"List index parsing failed", L"No se pudo interpretar el índice de la lista"},

        // info_
        {L"Language updated", L"Idioma actualizado"},
        {L"List rows filters updated", L"Filtros de las filas de la lista actualizados"},
        {L"The list has been reloaded from the hosts file", L"La lista se ha recargado desde el archivo hosts"},

        {L"You won't be able to perform/load backups without a valid backup directory", L"No podrá crear ni cargar copias de seguridad sin un directorio válido"},
        {L"Hosts file has been backed up", L"Se ha creado una copia de seguridad del archivo hosts"},
        {L"Hosts file has been cleared", L"Se ha vaciado el archivo hosts"},
        {L"Hosts file is currently empty, requested operation ignored", L"El archivo hosts está vacío; se ha omitido la operación solicitada"},
        {L"The backup directory is missing", L"No se encuentra el directorio de copias de seguridad"},

        {L"Operation aborted by user", L"El usuario ha cancelado la operación"},

        {L"The selected backup file has been loaded into memory", L"Se ha cargado en memoria el archivo de copia de seguridad seleccionado"},

        {L"Couldn't write to hosts file, file might be busy", L"No se pudo escribir en el archivo hosts; es posible que esté en uso"},
        {L"Couldn't open requested/hosts file, check program permissions", L"No se pudo abrir el archivo solicitado o el archivo hosts; compruebe los permisos de la aplicación"},
        {L"Couldn't find requested file", L"No se pudo encontrar el archivo solicitado"},
        {L"Data successfully saved into the hosts file", L"Los datos se han guardado correctamente en el archivo hosts"},

        {L"Hosts file rules have been applied successfully, note: applications might store previous hosts rules.", L"Las reglas del archivo hosts se han aplicado correctamente. Nota: algunas aplicaciones podrían conservar reglas anteriores en caché."},
        {L"A new entry has been added into memory, however, current filter selection won't render it", L"Se ha agregado una nueva entrada a la memoria; sin embargo, los filtros actuales impedirán que se muestre"},

        // warning_
        {L"Reloading the list will overwrite any unsaved changes", L"Al recargar la lista, se perderán los cambios que no se hayan guardado"},
        {L"Restore from a backup will overwrite any unsaved changes performed on the list", L"Al restaurar una copia de seguridad, se perderán los cambios no guardados de la lista"},
        {L"Pending changes detected, they will be permanently lost if you don't save", L"Se detectaron cambios pendientes que se perderán permanentemente si no los guarda"},
        {L"This operation will erase the contents of the hosts file, this is irreversible", L"Esta operación eliminará todo el contenido del archivo hosts. Esta acción es irreversible"},

        {L"Couldn't locate hosts file in its default directory", L"No se pudo encontrar el archivo hosts en su directorio predeterminado"},
        {L"Couldn't copy the hosts file, verify file permissions", L"No se pudo copiar el archivo hosts; compruebe los permisos del archivo"},
        {L"Couldn't clear the hosts file, verify file permissions", L"No se pudo vaciar el archivo hosts; compruebe los permisos del archivo"},
        {L"No backups have been located, proceed with caution", L"No se encontraron copias de seguridad. Proceda con precaución"},

        {L"Create process operation failed, current hosts file rules couldn't be applied, verify program permissions", L"No se pudo iniciar el proceso; no fue posible aplicar las reglas actuales del archivo hosts. Compruebe los permisos de la aplicación"},

        {L"Line revalidation failed, component might not show the accurate line contents", L"No se pudo revalidar la línea; es posible que el componente no muestre su contenido correctamente"},

        // confirm_
        {L"Backup directory not found, would you like to create one?", L"No se encontró el directorio de copias de seguridad. ¿Desea crearlo?"},
        {L"", L""},
        {L"", L""},

        // control buttons
        {L"Delete entry", L"Eliminar entrada"},
        {L"New entry", L"Nueva entrada"},
        {L"Save changes", L"Guardar cambios"},
        {L"Apply filter", L"Aplicar filtros"},
        {L"Discard changes", L"Descartar cambios"},
    }};


	std::wstring_view get_text(Text id) {
		if (id >= Text::COUNT)
			return L"Invalid String ID Requested!";

		return TextData[static_cast<std::size_t>(id)][static_cast<std::size_t>(Language::get())];
	}
}
