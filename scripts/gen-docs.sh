#!/usr/bin/bash

doxygen ./docs/Doxyfile


if [ $? -eq 0 ]; then
    printf "\n\033[0;37mHTML document was generated in ./docs/html/index.html\n"
    printf "You can see it using this commond: firefox ./docs/html/index.html\033[0m\n"
else
    printf "\n\033[0;31mDocument generation fails.\033[0m\n"
fi