#compdef keyd

_keyd() {
    local context state state_descr line
    typeset -A opt_args

    _arguments -C  \
        '1:keyd comamands:->keyd_command' \
        '*:: :->args' \
	'(- :)'{-h,--help}'[Print help]' \
        '(- :)'{-v,--version}'[Print the current version]' 
    case $state in
        args)
            case $line[1] in
                monitor | input | do)
                    _arguments '-t[timeout in ms]'
                    ;;
                check)
		    _alternative \
                        'conf-files:config files:_path_files -g "*.conf"' \
                        'directories:directories:_path_files -/'
                    ;;
            esac
            ;;
        keyd_command)
            local -a commands=(
                'monitor:Print key events in real time'
                'check:Check the supplied config files for errors If no files are supplied, all conf files in the config directory will be checked'
                'list-keys:Print a list of valid key names'
                'reload:Trigger a reload'
                'listen:Print layer state changes of the running keyd daemon to stdout'
                'bind:Add the supplied bindings to all loaded configs'
            )
            _describe -t commands keyd-commands commands
            ;;
    esac

}

_keyd "$@"
