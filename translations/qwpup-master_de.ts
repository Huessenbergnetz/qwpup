<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de">
<context>
    <name></name>
    <message id="qwpup_master_cli_app_desc">
        <source>Application to orchestrate multiple WordPress installation updates using qwpup.</source>
        <extracomment>Application descrpition in the CLI help</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_cli_opt_config_file_path">
        <source>Path to configuration file. Default: %1</source>
        <extracomment>Option description in the CLI help, %1 will be replaced by the full default file path</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_cli_opt_val_path">
        <source>path</source>
        <extracomment>Option value name in the cli help for file and directory paths</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_cli_opt_log_level">
        <source>Log level and higher for that messages are shown. Available: %1. Default: %2</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_cli_opt_log_level_val">
        <source>level</source>
        <extracomment>Option value name in the CLI help for the log level</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_cli_opt_dry_run">
        <source>Do a dry run without performing real actions.</source>
        <extracomment>Option description in the CLI help</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_inv_ll">
        <source>Invalid log level.</source>
        <extracomment>Error message</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_info_start_site_update">
        <source>Starting update at %1.</source>
        <oldsource>Starting update.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_empty_name">
        <source>Empty site name. Set a site name in your configuration for better logging.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_path_empty">
        <source>Update failed: empty path to WordPress directory.</source>
        <oldsource>Can not update site “%1”: empty path to WordPress directory.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_path_not_exists">
        <source>Update failed: WordPress directory path “%1” does not exist.</source>
        <oldsource>Can not update site “%1”: path “%2” does not exist.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_invalid_username">
        <source>Unknown user set. Please check your configuration. Will try to get user from wp-config.php file owner.</source>
        <oldsource>Unknown user set for site “%1”. Please check your configuration. Will try to get user from wp-config.php file owner.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_no_uid">
        <source>Update failed: Can not obtain user id. Please specify user in site configuration.</source>
        <oldsource>Can not update site “%1“: Can not obtain user id. Please specify user in site configuration.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_invalid_groupname">
        <source>Unknown group set. Please check your configuration. Will try to get group from wp-config.php file group.</source>
        <oldsource>Unknown group set for site “%1”. Please check your configuration. Will try to get group from wp-config.php file group.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_no_gid">
        <source>Update failed: Can not obtain group id. Please specify group in site configuration.</source>
        <oldsource>Can not update site “%1“: Can not obtain group id. Please specify group in site configuration.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_site_no_stat_file">
        <source>Update failed: Failed to create temporary file for statistics.</source>
        <oldsource>Can not update site “%1”: Failed to create temporary file for statistics.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_update_failed">
        <source>Update failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_info_update_success">
        <source>Update successful.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_update_failed_process">
        <source>Update failed: %1</source>
        <extracomment>%1 will be replaced by the error string returned by QProcess</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_master_conf_not_found">
        <source>Can not find master configuration file at %1</source>
        <extracomment>%1 will be replaced by the file path</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_master_conf_fail_open">
        <source>Failed to open master configuration file at %1: %2</source>
        <extracomment>%1 will be replaced by the file path, %2 by the error message</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_read_config_error">
        <source>Failed to read master configuration file at %1: %2</source>
        <extracomment>%1 will be replaced by the file path, %2 by the error message</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_empty_config_file">
        <source>Empty configuration file at %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_empty_config">
        <source>Configuration is empty and does not contain any data.</source>
        <oldsource>Empty configuration file at %1.</oldsource>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_config_empty_sites">
        <source>No sites configured. Please add sites to update to your configuration.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_warn_config_emtpy_site">
        <source>Empty entry in “sites“ configuration.</source>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_wpconfig_not_found">
        <source>Can not find WordPress configurtion file at “%1”.</source>
        <oldsource>Can not find WordPress configurtion file at “%1”.
</oldsource>
        <extracomment>%1 will be replaced by the full path to the file</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_config_json_parse_failed">
        <source>Failed to parse configuration file JSON data: %1</source>
        <extracomment>Error message, %1 will be replaced by the error message from the JSON parser.</extracomment>
        <translation type="unfinished"></translation>
    </message>
    <message id="qwpup_err_config_json_unexpected_type">
        <source>Unexpected JSON root type in configuration file.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
