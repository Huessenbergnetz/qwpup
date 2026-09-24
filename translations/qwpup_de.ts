<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de" sourcelanguage="en">
<context>
    <name></name>
    <message id="qwpup_cli_app_desc">
        <source>Qt based wrapper for WP CLI to automate WordPress updates with some extras.</source>
        <extracomment>Application description for the CLI help overview</extracomment>
        <translation>Auf Qt basierendes Werkzeug zur automatisierten Aktualisierung von WordPress, dass WP CLI um einige Extras erweitert.</translation>
    </message>
    <message id="qwpup_cli_opt_log_level">
        <source>Log level and higher for that messages are shown. Available: %1. Default: %2</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Protokollierungsebene und höher für die Meldungen ausgegeben werden. Verfügbar: %1. Standard: %2</translation>
    </message>
    <message id="qwpup_cli_opt_log_level_val">
        <source>level</source>
        <extracomment>Option value name in the CLI help for the log level</extracomment>
        <translation>Ebene</translation>
    </message>
    <message id="qwpup_cli_opt_skip_comp">
        <source>Skip compressing JS and CSS assets.</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Überspringe das Komprimieren von JS- und CSS-Dateien.</translation>
    </message>
    <message id="qwpup_cli_opt_yes">
        <source>Say yes to everything.</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Sage zu allem Ja.</translation>
    </message>
    <message id="qwpup_cli_opt_up_wp_maj">
        <source>Update WordPress to a new major version. By default only minor version udpates will be performed.</source>
        <oldsource>Update WordPress to a new major version instead to just a minor version update.</oldsource>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Aktualisiere WordPress auf eine neue Hauptversion. Standardmäßig werden nur Aktualisierungen auf Unterversionen durchgeführt.</translation>
    </message>
    <message id="qwpup_cli_opt_plug_ver">
        <source>Only perform plugin updates for major, minor or patch releases: Default: major.</source>
        <extracomment>Option description in the CLI help, DO NOT TRANSLATE the terms major, minor and patch</extracomment>
        <translation>Führe Plugin-Aktualisierungen nur für major, minor oder patch Versionen durch: Standard: major.</translation>
    </message>
    <message id="qwpup_cli_opt_value_ver_part">
        <source>part</source>
        <extracomment>Option value name in the CLI help vor version number part like major, minor</extracomment>
        <translation>Versionsteil</translation>
    </message>
    <message id="qwpup_cli_opt_themes_ver">
        <source>Only perform theme updates for major, minor or patch releases: Default: major.</source>
        <extracomment>Option description in the CLI help, DO NOT TRANSLATE the terms</extracomment>
        <translation>Führe Theme-Aktualisierungen nur für major, minor oder patch Versionen durch: Standard: major.</translation>
    </message>
    <message id="qwpup_cli_opt_wp_cli">
        <source>Path to the WP CLI executable. By default, this will be searched in the PATH.</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Pfad zur ausführbaren WP-CLI-Datei (wp). Standardmäßig wird diese in den Verzeichnissen der PATH-Variablen gesucht.</translation>
    </message>
    <message id="qwpup_cli_opt_val_path">
        <source>path</source>
        <extracomment>Option value name in the cli help for file and directory paths</extracomment>
        <translation>Pfad</translation>
    </message>
    <message id="qwpup_cli_opt_wp_dir">
        <source>Path to the WordPress root directory. If omitted, the current directory will be used.</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Pfad zum WordPress-Wurzelverzeichnis. Falls ausgelassen wird das aktuelle Verzeichnis verwendet.</translation>
    </message>
    <message id="qwpup_err_inv_ll">
        <source>Invalid log level.</source>
        <oldsource>Invalid log level</oldsource>
        <extracomment>Error message</extracomment>
        <translation>Ungültige Protokollierungsebene.</translation>
    </message>
    <message id="qwpup_err_user_unknown">
        <source>Failed to get current user.</source>
        <oldsource>Failed to get current user</oldsource>
        <translation>Konnte den aktuellen Benutzer nicht abfragen.</translation>
    </message>
    <message id="qwpup_err_user_root">
        <source>Do not run this command as super user (root).</source>
        <oldsource>Do not run this command as super user (root)</oldsource>
        <translation>Führen Sie diesen Befehl nicht als Super User (root) aus.</translation>
    </message>
    <message id="qwpup_err_wp_config_owner_mismatch">
        <source>Current user it not the owner of the wp-config.php file. Please run the command as owner of the WordPress files.</source>
        <translation>Der aktuelle Nutzer ist nicht Eigentümer der Datei wp-config.php. Bitte führen Sie diesen Befehl als Eigentümer der WordPress-Dateien aus.</translation>
    </message>
    <message id="qwpup_err_wp_exe_path_wrong">
        <source>Can not find WP CLI executable at “%1“.</source>
        <oldsource>No file found at “%1“.</oldsource>
        <extracomment>Error message, %1 will be replaced by the full file path</extracomment>
        <translation>Kann unter „%1“ keine ausführbare Datei für WP CLI finden.</translation>
    </message>
    <message id="qwpup_err_wp_exe_not_found">
        <source>Can not find WP CLI executable (wp or wp-cli). Check your PATH or explicitely set the path to the executable with “%1”.</source>
        <oldsource>Can not find WP CLI executable (wp or wp-cli). Check your PATH or explicitely set the path to the executable
</oldsource>
        <extracomment>Error message, %1 will be replaced with a CLI option name like --wp-cli</extracomment>
        <translation>Konnte die ausführbare WP-CLI-Datei (wp oder wp-cli) nicht finden. Prüfen Sie Ihre PATH-Variable oder setzen Sie den Pfad zur ausführebaren Datei explizit mit „%1“.</translation>
    </message>
    <message id="qwpup_err_invalid_tmp_dir">
        <source>Failed to create temporary directory: %1</source>
        <extracomment>Error message, %1 will be replaced by the error message</extracomment>
        <translation>Konnte kein temporäres Verzeichznis erstellen: %1</translation>
    </message>
    <message id="qwpup_inf_cur_wp_core_version">
        <source>Current WordPress core version: %1</source>
        <extracomment>Info message, %1 will be replaced by the version string like 6.8.3</extracomment>
        <translation>Aktuelle WordPress-Version: %1</translation>
    </message>
    <message id="qwpup_err_wp_version_info_failed">
        <source>Failed to get version information.</source>
        <translation>Konnte die Versionsinformationen nicht abfragen.</translation>
    </message>
    <message id="qwpup_info_check_core_updates">
        <source>Checking for core updates.</source>
        <translation>Prüfe auf Aktualisierungen des Kerns.</translation>
    </message>
    <message id="qwpup_info_skip_plug_ups">
        <source>Skipped plugin updates: %1.</source>
        <extracomment>%1 will be replaced by a comma separated list of plugin updates.</extracomment>
        <translation>Übersprungene Plugin-Aktualisierungen: %1.</translation>
    </message>
    <message id="qwpup_info_skip_theme_ups">
        <source>Skipped theme updates: %1.</source>
        <extracomment>%1 will be replaced by a comma separated list of theme updates.</extracomment>
        <translation>Übersprungene Theme-Aktualisierungen: %1.</translation>
    </message>
    <message id="qwpup_info_check_plugin_trans_updates">
        <source>Checking for plugin translation updates.</source>
        <translation>Prüfe auf Aktualisierungen für Plugin-Übersetzungen.</translation>
    </message>
    <message id="qwpup_info_up_plug_langs">
        <source>Updating languages for plugin „%1“: %2.</source>
        <extracomment>%1 will be replaced by the plugin name, %2 by a list of the languages</extracomment>
        <translation>Aktualisiere Sprache für Plugin „%1“: %2.</translation>
    </message>
    <message id="qwpup_err_plugs_lang_check_failed">
        <source>Failed to check for plugin translation updates.</source>
        <translation>Konnte nicht auf Aktualierungen der Plugin-Übersetzungen prüfen.</translation>
    </message>
    <message id="qwpup_info_upd_plug_lang_success">
        <source>Successfully updated plugin translations.</source>
        <translation>Plugin-Übersetzungen erfolgreich aktualisiert.</translation>
    </message>
    <message id="qwpup_err_upd_plug_langs_failed">
        <source>Failed to update plugin translations.</source>
        <translation>Konnte die Plugin-Übersetzungen nicht aktualisieren.</translation>
    </message>
    <message id="qwpup_err_json_parse_failed">
        <source>Failed to parse JSON data: %1</source>
        <extracomment>Error message, %1 will be replaced by the error message from the JSON parser.</extracomment>
        <translation>Konnte die JSON-Daten nicht verarbeiten: %1</translation>
    </message>
    <message id="qwpup_err_json_unexpected_type">
        <source>Unexpected JSON type.</source>
        <oldsource>Unexpected JSON type. Aborting.</oldsource>
        <translation>Unerwarteter JSON-Typ.</translation>
    </message>
    <message id="qpwup_info_no_core_ups_avail">
        <source>No core updates available.</source>
        <translation>Keine Aktualisierungen für den Kern verfügbar.</translation>
    </message>
    <message id="qwpup_cli_opt_dry_run">
        <source>Do not perform the real actions.</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Führe keine echten Aktionen durch.</translation>
    </message>
    <message id="qwpup_cli_opt_stats">
        <source>Write stats to file at path or stdout.</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation>Schreibe Statistiken in die Datei unter Pfad oder nach stdout.</translation>
    </message>
    <message id="qwpup_info_perform_dry_run">
        <source>Doing dry run without performing real actions.</source>
        <translation>Führe Probelauf ohne echte Aktionen durch.</translation>
    </message>
    <message id="qwpup_inf_min_core_ver_avail">
        <source>New minor core version available: %1</source>
        <translation>Neue Unterversion für den Kern verfügbar: %1</translation>
    </message>
    <message id="qwpup_inf_maj_core_ver_avail">
        <source>New major core version available: %1</source>
        <translation>Neue Hauptversion für den Kern verfügbar: %1</translation>
    </message>
    <message id="qwpup_info_skip_major_core_update">
        <source>Skipping major core update.</source>
        <translation>Überspringe Aktualisierung der Hauptversion des Kerns.</translation>
    </message>
    <message id="qwpup_err_ep_core_up_check_failed">
        <source>Failed to check for core updates.</source>
        <translation>Konnte nicht auf Aktualisierungen für den Kern prüfen.</translation>
    </message>
    <message id="qwpup_ask_update_core">
        <source>Do you want to update WordPress core from version %1 to %2?</source>
        <extracomment>%1 will be replaced by the current version number, %2 by the target version</extracomment>
        <translation>Möchten Sie WordPress von Version %1 auf %2 aktualisieren?</translation>
    </message>
    <message id="qwpup_info_update_core">
        <source>Updating WordPress core from version %1 to version %2.</source>
        <extracomment>Info message, %1 will be replaced by the current WordPress core version, %2 will be replaced by the newer version</extracomment>
        <translation>Aktualisiere WordPress von Version %1 auf %2.</translation>
    </message>
    <message id="qwpup_infi_update_core_success">
        <source>Successfully updated WordPres core from version %1 to version %2.</source>
        <oldsource>Successfully updated WordPres core.</oldsource>
        <extracomment>Info message, %1 will be replaced by the previous WordPress core version, %2 will be replaced by the now updated version</extracomment>
        <translation>WordPress erfolgreich von Version %1 auf Version %2 aktualisiert.</translation>
    </message>
    <message id="qwpup_err_wp_core_update_failed">
        <source>Failed to update WordPress core.</source>
        <translation>Konnte der WordPress-Kern nicht aktualisieren.</translation>
    </message>
    <message id="qwpup_info_check_plugin_updates">
        <source>Checking for plugin updates.</source>
        <translation>Prüfe auf Plugin-Aktualisierungen.</translation>
    </message>
    <message id="qwpup_info_no_plug_ups_avail">
        <source>No plugin updates available.</source>
        <translation>Keine Plugin-Aktualisierungen verfügbar.</translation>
    </message>
    <message id="qwpup_info_updates_none">
        <source>none</source>
        <extracomment>Used when no updates for e.g. plugins and themes are available, in a form like &quot;Available plugin updates: none&quot;</extracomment>
        <translation>keine</translation>
    </message>
    <message id="qwpup_dbg_avail_plug_up">
        <source>Available plugin update: %1 %2 =&gt; %3</source>
        <extracomment>%1 will be replaced by the plugin name, %2 by the current version, %3 by the udpate version</extracomment>
        <translation>Verfügbare Plugin-Aktualisierunge: %1 %2 =&gt; %3</translation>
    </message>
    <message id="qwpup_info_avail_plug_ups">
        <source>Available plugin updates: %1.</source>
        <extracomment>%1 will be replaced by a comma separated list of plugin updates or &quot;none&quot;.</extracomment>
        <translation>Verfügbare Plugin-Aktualisierungen: %1.</translation>
    </message>
    <message id="qwpup_dbg_skipped_plug_up">
        <source>Skipped plugin update: %1 %2 =&gt; %3</source>
        <extracomment>%1 will be replaced by the plugin name, %2 by the current version, %3 by the udpate version</extracomment>
        <translation>Übersprungene Plugin-Aktualisierung: %1 %2 =&gt; %3</translation>
    </message>
    <message id="qwpup_err_plug_check_failed">
        <source>Failed to check for plugin updates.</source>
        <translation>Konnte nicht auf Plugin-Aktualisierungen prüfen.</translation>
    </message>
    <message id="qwpup_warn_failed_open_asset">
        <source>Failed to open %1 for reading: %2</source>
        <extracomment>%1 will bereplaced by the absolute file path, %2 the error message</extracomment>
        <translation>Konnte %1 nicht zum lesen öffnen: %2</translation>
    </message>
    <message id="qwpup_warn_comp_brotli_too_large">
        <source>Required Brotli output buffer too large to compress input file of size %1: %2</source>
        <extracomment>%1 will be replaced by the size, %2 by the full path to the asset file</extracomment>
        <translation>Der für die Brotli-Kompression benötigte Ausgapepuffer ist zu groß für die Dateigröße %1: %2</translation>
    </message>
    <message id="qwpup_warn_comp_brotli_fail">
        <source>Failed to compress asset with Brotli: %1</source>
        <extracomment>%1 will be replaced by the full file path</extracomment>
        <translation>Konnte die Datei nicht mit Brotli komprimieren: %1</translation>
    </message>
    <message id="qwpup_warn_comp_brotli_open_out">
        <source>Failed to open %1 for writing: %2</source>
        <extracomment>%1 will be replaced by the file path, %2 by the error message</extracomment>
        <translation>Konnte Datei %1 nicht zum schreiben öffnen: %2</translation>
    </message>
    <message id="qwpup_warn_comp_brotli_write_out">
        <source>Failed to write compressed data to %1: %2</source>
        <extracomment>%1 will be replaced by the file path, %2 by the error message</extracomment>
        <translation>Konnte die komprimierten Daten nicht in %1 schreiben: %2</translation>
    </message>
    <message id="qwpup_ask_update_plugin">
        <source>Do you want to update the plugin “%1” from version %2 to %3?</source>
        <oldsource>Do you want to update plugin %1 from version %2 to %3?</oldsource>
        <extracomment>%1 will be replaced by the plugin’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Möchten Sie das Plugin „%1“ von Version %2 auf %3 aktualisieren?</translation>
    </message>
    <message id="qwpup_info_update_plugin">
        <source>Updating plugin %1 from version %2 to %3.</source>
        <extracomment>%1 will be replaced by the plugin’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Aktualisiere Plugin „%1“ von Version %2 auf %3.</translation>
    </message>
    <message id="qwpup_info_plug_up_success">
        <source>Successfully updated plugin %1 from version %2 to %3.</source>
        <oldsource>Successfully updated plugin %1 from version %2 to %3.
</oldsource>
        <extracomment>%1 will be replaced by the plugin’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Pugin „%1“ erfolgreich von Version %2 auf %3 aktualisiert.</translation>
    </message>
    <message id="qwpup_info_plug_compr_assets">
        <source>Start compressing assets for plugin %1.</source>
        <extracomment>%1 will be replaced by the plugin name</extracomment>
        <translation>Beginne mit dem Komprimieren der JS- und CSS-Datin des Plugins „%1“.</translation>
    </message>
    <message id="qwpup_info_plug_compr_assets_finished">
        <source>Finished compressing assets for plugin %1 in %2 ms.</source>
        <oldsource>Finished compressing assets for plugin %1.</oldsource>
        <extracomment>%1 will be replaced by the plugin name, %2 by the duration the compression took in miliseconds</extracomment>
        <translation>Komprimieren der JS- und CSS-Dateien des Plugins „%1“ in %2 ms beendet.</translation>
    </message>
    <message id="qwpup_err_plug_up_failed">
        <source>Failed to update plugin %1 from version %2 to %3.</source>
        <extracomment>%1 will be replaced by the plugin’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Konnte das Plugin „%1“ nicht von Version %2 auf %3 aktualisieren.</translation>
    </message>
    <message id="qwpup_info_check_theme_updates">
        <source>Checking for theme updates.</source>
        <translation>Prüfe auf Theme-Aktualisierungen.</translation>
    </message>
    <message id="qwpup_info_no_theme_ups_avail">
        <source>No theme updates available.</source>
        <translation>Keine Theme-Aktualisierungen verfügbar.</translation>
    </message>
    <message id="qwpup_dbg_avail_theme_up">
        <source>Available theme update: %1 %2 =&gt; %3</source>
        <extracomment>%1 will be replaced by the theme name, %2 by the current version, %3 by the update version</extracomment>
        <translation>Verfügbare Theme-Aktualisierung: %1 %2 =&gt; %3</translation>
    </message>
    <message id="qwpup_info_avail_theme_ups">
        <source>Available theme updates: %1.</source>
        <extracomment>%1 will be replaced by a comma separated list of theme updates or &quot;none&quot;.</extracomment>
        <translation>Verfügbare Theme-Aktualsierungen: %1.</translation>
    </message>
    <message id="qwpup_dbg_skipped_theme_up">
        <source>Skipped theme update: %1 %2 =&gt; %3</source>
        <extracomment>%1 will be replaced by the theme name, %2 by the current version, %3 by the update version</extracomment>
        <translation>Übersprungene Theme-Aktualisierung: %1 %2 =&gt; %3</translation>
    </message>
    <message id="qwpup_err_theme_check_failed">
        <source>Failed to check for theme updates.</source>
        <translation>Konnte nicht auf Them-Aktualisierungen prüfen.</translation>
    </message>
    <message id="qwpup_ask_update_theme">
        <source>Do you want to update the theme “%1” from version %2 to %3?</source>
        <oldsource>Do you want to update theme %1 from version %2 to %3?</oldsource>
        <extracomment>%1 will be replaced by the themes’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Möchten Sie das Theme „%1“ von Version %2 auf %3 aktualisieren?</translation>
    </message>
    <message id="qwpup_info_update_theme">
        <source>Updating theme %1 from version %2 to %3.</source>
        <extracomment>%1 will be replaced by the themes’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Aktualisiere Theme „%1“ von Version %2 auf %3.</translation>
    </message>
    <message id="qwpup_info_theme_up_success">
        <source>Successfully updated theme %1 from version %2 to %3.</source>
        <extracomment>%1 will be replaced by the themes’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Theme „%1“ erfolgreich von Version %2 auf %3 aktualisiert.</translation>
    </message>
    <message id="qwpup_info_theme_compr_assets">
        <source>Start compressing assets for theme %1.</source>
        <extracomment>%1 will be replaced by the theme name</extracomment>
        <translation>Beginne mit dem Komprimieren der JS- und CSS-Datin des Themes „%1“.</translation>
    </message>
    <message id="qwpup_info_theme_compr_assets_finished">
        <source>Finished compressing assets for theme %1 in %2 ms.</source>
        <extracomment>%1 will be replaced by the theme name, %2 by the duration the compression took in miliseconds</extracomment>
        <translation>Komprimieren der JS- und CSS-Dateien des Themes „%1“ in %2 ms beendet.</translation>
    </message>
    <message id="qwpup_err_theme_up_failed">
        <source>Failed to update theme %1 from version %2 to %3.</source>
        <extracomment>%1 will be replaced by the plugin’s name, %2 by the current version and %3 by the update version</extracomment>
        <translation>Konnte das Theme „%1“ nicht von Version %2 auf %3 aktualisieren.</translation>
    </message>
    <message id="qwpup_info_check_core_trans_updates">
        <source>Checking for core translation updates.</source>
        <translation>Prüfe auf Aktualisierungen der Kernübersetzungen.</translation>
    </message>
    <message id="qwpup_info_no_core_lang_ups_avaqil">
        <source>No core language updates availabe.</source>
        <translation>Keine Aktualisierungen für Kernüberseztungen verüfgbar.</translation>
    </message>
    <message id="qwpup_err_core_lang_check_failed">
        <source>Failed to check for core translation updates.</source>
        <oldsource>Failed to check for core language updates.</oldsource>
        <translation>Konnte nicht auf Aktualisierungen der Kernübersetzungen prüfen.</translation>
    </message>
    <message id="qwpup_info_update_core_translations">
        <source>Updating core translations: %1.</source>
        <oldsource>Updating core translations.</oldsource>
        <extracomment>%1 will be replaced by a list comma separated list of native language names</extracomment>
        <translation>Aktualisiere Kernübersetzungen: %1.</translation>
    </message>
    <message id="qwpup_info_upd_core_lang_success">
        <source>Successfully updated core translations.</source>
        <translation>Kernübersetzungen erfolgreich aktualisiert.</translation>
    </message>
    <message id="qwpup_err_upd_core_langs_failed">
        <source>Failed to update core translations.</source>
        <translation>Aktualisierung der Kernübersetzungen fehlgeschlagen.</translation>
    </message>
    <message id="qwpup_question_answers_yesnocancel">
        <source>(Y)es/(N)o/(C)ancel</source>
        <extracomment>Answer options to a confirmation question</extracomment>
        <translation>(J)a/(N)ein/(A)bbruch</translation>
    </message>
    <message id="qwpup_question_answers_yesno">
        <source>(Y)es/(N)o</source>
        <extracomment>Answer options to a confirmation question</extracomment>
        <translation>(J)a/(N)ein</translation>
    </message>
    <message id="qwpup_quest_answer_yes_short">
        <source>Y</source>
        <extracomment>Answer to a confirmation question, abbreviation for &quot;Yes&quot;</extracomment>
        <translation>J</translation>
    </message>
    <message id="qwpup_warn_failed_read_asset">
        <source>Failed to read file: %1: %2</source>
        <extracomment>%1 will be replaced by the full file path, %2 by the error string</extracomment>
        <translation>Konnte Datei nicht lesen: %1: %2</translation>
    </message>
    <message id="qwpup_dbg_skip_empty_asset">
        <source>Skipping empty file: %1</source>
        <extracomment>%1 will be replaced by the full file path</extracomment>
        <translation>Überpspringe leere Datei: %1</translation>
    </message>
    <message id="qwpup_info_no_plug_lang_ups-avail">
        <source>No plugin language updates available.</source>
        <translation>Keine Aktualisierungen für Plugin-Überseztungen verfügbar.</translation>
    </message>
    <message id="qwpup_info_check_theme_trans_updates">
        <source>Checking for theme translation updates.</source>
        <translation>Prüfe auf Aktualisierungen für Theme-Übersetzungen.</translation>
    </message>
    <message id="qwpup_info_no_theme_lang_ups-avail">
        <source>No theme language updates available.</source>
        <translation>Keine Aktualisierungen für Theme-Übersetzungen verfügbar.</translation>
    </message>
    <message id="qwpup_info_up_theme_langs">
        <source>Updating languages for theme „%1“: %2.</source>
        <extracomment>%1 will be replaced by the theme name, %2 by a list of the languages</extracomment>
        <translation>Aktualisiere Übersetzungen für Theme „%1“: %2.</translation>
    </message>
    <message id="qwpup_err_theme_lang_check_failed">
        <source>Failed to check for theme translation updates.</source>
        <translation>Konnte nicht auf Aktualisierungen für Theme-Übersetzungen prüfen.</translation>
    </message>
    <message id="qwpup_info_upd_theme_lang_success">
        <source>Successfully updated theme translations.</source>
        <translation>Theme-Übersetzungen erfolgreich aktualisiert.</translation>
    </message>
    <message id="qwpup_err_upd_theme_langs_failed">
        <source>Failed to update theme translations.</source>
        <translation>Konnte Theme-Übersetzungen nicht aktualisieren.</translation>
    </message>
    <message id="qwpup_info_compr_all_assets">
        <source>Start compressing assets for the whole installation.</source>
        <translation>Beginne mit dem Komprimieren aller JS- und CSS-Dateien der gesamten Installation.</translation>
    </message>
    <message id="qwpup_info_compr_all_assets_finished">
        <source>Finished compressing all assets in %1 ms.</source>
        <extracomment>%1 will be replaced by the duration the compression took in miliseconds</extracomment>
        <translation>Komprimieren aller JS- und CSS-Dateien in %1 ms beendet.</translation>
    </message>
    <message id="qwpup_quest_answer_yes">
        <source>Yes</source>
        <extracomment>Answer to a confirmation question, full word</extracomment>
        <translation>Ja</translation>
    </message>
    <message id="qwpup_quest_answer_no_short">
        <source>N</source>
        <extracomment>Answer to a confirmation question, abbreviation for &quot;No&quot;</extracomment>
        <translation>N</translation>
    </message>
    <message id="qwpup_quest_answer_no">
        <source>No</source>
        <extracomment>Answer to a confirmation question, full word</extracomment>
        <translation>Nein</translation>
    </message>
    <message id="qwpup_quest_answer_cancel_short">
        <source>C</source>
        <extracomment>Answer to a confirmation question, abbreviation for &quot;Cancel&quot;</extracomment>
        <translation>A</translation>
    </message>
    <message id="qwpup_quest_answer_cancel">
        <source>Cancel</source>
        <extracomment>Answer to a confirmation question, full word&quot;</extracomment>
        <translation>Abbruch</translation>
    </message>
    <message id="qwpup_err_wp_dir_not_exists">
        <source>The directory “%1” does not exist.</source>
        <oldsource>The directory “%1” does not exist.
</oldsource>
        <extracomment>Error message, %1 will be replaced by the absolute path to the directory</extracomment>
        <translation>Das Verzeichznis %1 existiert nicht.</translation>
    </message>
    <message id="qwpup_err_wp_config_not_found">
        <source>Can not find wp-config.php configuration file. We seem not to be inside the root directory of a WordPress installation. Either run this command inside a WordPress root directory or use “%1” to specify the path to a WordPress root directory.</source>
        <oldsource>Can not find wp-config.php configuration file. We seem not to be inside the root directory of a WordPress installation. Either run this command inside a WordPress root directory or use %1 to specify the path to a WordPress root directory.</oldsource>
        <extracomment>Error message, %1 will be replaced with a CLI option name like --wp-dir</extracomment>
        <translation>Kann die Konfigurationsdatei wp-config.php nicht finden. Wir scheinen uns nicht im Wurzelverzeichnis einer WordPress-Installation zu befinden. Führen Sie diesen Befehl entweder in einem WordPress-Wurzelverzeichnis aus oder benutzen Sie „%1“ um den absoluten Pfad zu einem WordPress-Wurzelverzeichnis anzugeben.</translation>
    </message>
    <message id="qwpup_err_wp_invalid_version_part">
        <source>Invalid version part identifier. Only major, minor or patch are allowed.</source>
        <extracomment>Error message, DO NOT TRANSLATE the terms major, minor and patch</extracomment>
        <translation>Ungülter Name für einen Versionsteil, es sind nur die folgenden gültig: major, minor oder patch.</translation>
    </message>
</context>
</TS>
