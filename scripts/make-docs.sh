#!/bin/bash
# brief : Easily make documentations.


# Ancillary Variables
FONT_RED=\\033[031m
FONT_GREEN=\\033[032m
FONT_YELLOW=\\033[033m
FONT_BLUE=\\033[034m

FONT_BOLD=\\033[1m
FONT_UNDERLINE=\\033[4m

FONT_RESET=\\033[0m

# Functions
MakeDocs() { 
    pwd_has_express=$(echo $PWD | grep Express)
    cur_not_in_express=$(echo $PWD | grep Express/)

    if [ $pwd_has_express ]; then
        if [ $cur_not_in_express ]; then
            printf "$FONT_RED \rYou should run this commond in Express root directory. $FONT_RESET\n"
            exit 1
        else
            doxygen ./docs/Doxyfile
            printf "\n$FONT_YELLOW \rDocumentation was generated successfully.\n"
            printf "The index html file is in ./docs/html/index.html. You can see it using \n"
            printf "firefox ./docs/html/index.html $FONT_RESET\n" 
        fi
    else
        printf "$FONT_RED \rYou should run this commond in Express directory. $FONT_RESET\n"
        exit 2
    fi
}

ShowHelp(){
    echo "Express make-docs shell script.";
    echo "-p : make docs and then open it in the brownser.";
    echo "-h : show help information and exists.";
}

# Default behaviour
if [ $# -eq 0 ]; then
    MakeDocs
    exit 0
fi

# Commond option version.

set -- $(getopt ph "$@") 

while [ -n "$1" ]; do
    case $1 in
    -p) 
        MakeDocs
        firefox ./docs/html/index.html 
        ;;
    -h) 
        ShowHelp
        ;;
    --) 
        shift
        ;;
    *) 
        echo "$1 is not a valid option"
        ;;
    esac
    shift
done

