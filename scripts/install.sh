#!/bin/bash

echo -e "\n\033[1mWhatsAScript Installer\033[0m"

installpath="${HOME}/bin/was"

if [ -f "${HOME}/bin/was" ]; then
    echo -n -e "WhatsAScript is already installed. \033[4mR\033[0meinstall, \033[4mU\033[0mninstall, or \033[4mC\033[0mancel? [ ]"
    read -n1 confirm
    confirm=${confirm,,}
    echo -e "\b\b\b$confirm] "

    case $confirm in
        "r")
            ;;
        "u")
            rm $installpath
            echo -e "Uninstalled WhatsAScript.\n"
            exit 0
            ;;
        "c")
            echo -e "Cancelled.\n"
            exit 0
            ;;
        *)
            echo -e "Invalid option.\n"
            exit 1
            ;;
    esac
fi

echo -n "Install from beta branch? [ ]"
read -n1 confirm
confirm=${confirm,,}
echo -e "\b\b\b$confirm] "

if [ $confirm = "y" ]; then
    branch="beta"
else
    branch="main"
fi

echo -e "\n\033[2mcurl \u00b7\u00b7\u00b7/anpang54/whatsascript/$branch/release/was-linux -o $installpath\033[0m"
curl "https://raw.githubusercontent.com/anpang54/whatsascript/$branch/release/was-linux" -o $installpath
echo ""

chmod +x $installpath

echo "WhatsAScript is now installed in $installpath."
echo -e "You can try using "was --version" to check if WhatsAScript's really installed.\n"

exit 0
