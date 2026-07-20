_keyd() {
    local cur="${COMP_WORDS[$COMP_CWORD]}"
    COMPREPLY=()

    if [[ "$COMP_CWORD" -eq 1 ]]; then
        local opts
        case "$cur" in
            -*)
                opts="--help -h 
		    --version -v"
                ;;
            *)

                opts="monitor 
		      bind
		      check 
		      input
		      do
            	      listen
            	      reload
            	      list-keys"
                ;;
        esac
        COMPREPLY=($(compgen -W "$opts" -- "$cur"))

    elif [[ "$COMP_CWORD" -gt 1 ]]; then
        local subcmd="${COMP_WORDS[1]}"

        case "$subcmd" in
            check | -c | --check)
                COMPREPLY=($(compgen -f -d -- "$cur"))
                ;;

            monitor | -m | --monitor | input | do)
                if [[ "$COMP_CWORD" -eq 2 ]]; then
                    COMPREPLY=($(compgen -W "-t" -- "$cur"))
                elif [[ "$COMP_CWORD" -eq 3 && "${COMP_WORDS[2]}" = "-t" ]]; then
                    COMP_WORDS=($(complete -W "10 100 500" -- "$cur"))
                fi
                ;;
        esac
    fi
}

complete -F _keyd keyd -o bashdefault -o default -o filenames
