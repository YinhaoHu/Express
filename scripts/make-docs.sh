#!/bin/bash

# Easily make documentations.

FONT_RED=\\033[031m
FONT_GREEN=\\033[032m
FONT_YELLOW=\\033[033m
FONT_BLUE=\\033[034m

FONT_BOLD=\\033[1m
FONT_UNDERLINE=\\033[4m

FONT_RESET=\\033[0m

pwd_has_express=$(echo $PWD | grep Express)
cur_not_in_express=$(echo $PWD | grep Express/)

echo "$PWD"

if [ $pwd_has_express ]; then
    if [ $cur_not_in_express ]; then
        printf "$FONT_RED \rYou should run this commond in Express root directory. $FONT_RESET\n"
        exit 1
    else
        doxygen ./docs/Doxyfile
        printf "\n$FONT_YELLOW \rDocumentation was generated successfully.\n"
        printf "The index html file is in ./docs/html/index.html. You can see it using \n"
        printf "firefox ./docs/html/index.html $FONT_RESET\n"
        exit 0
    fi
else
    printf "$FONT_RED \rYou should run this commond in Express directory. $FONT_RESET\n"
    exit 2
fi
