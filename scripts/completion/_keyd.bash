_keyd() {
    local cur="${COMP_WORDS[$COMP_CWORD]}"
    COMPREPLY=()
    
    local opts="help --help -h
        version --version -v
        monitor -m --monitor
        bind -e --expression
        check -c --check
        input
        do
        listen
        reload
        list-keys"

    if [[ "$COMP_CWORD" -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "$opts" -- "$cur") )
    
    elif [[ "$COMP_CWORD" -gt 1 ]]; then 
        local subcmd="${COMP_WORDS[1]}"
        
        case "$subcmd" in 
            check|-c|--check)
                COMPREPLY=( $(compgen -f -d -- "$cur") )
                ;;
            
            monitor|-m|--monitor|input|do)
                if [[ "$COMP_CWORD" -eq 2 ]]; then
                    COMPREPLY=( $(compgen -W "-t" -- "$cur") )
		elif [[ "$COMP_CWORD" -eq 3 && "${COMP_WORDS[2]}" = "-t" ]];then 
		    COMP_WORDS=( $(complete -W "10 100 500" -- "$cur"))
		fi	
                ;;
        esac
    fi
}

complete -F _keyd keyd -o bashdefault -o default -o filenames 

