#!/bin/sh

rm -rf ~/.vgclient/

desktop_folder=`xdg-user-dir DESKTOP`
rm "$desktop_folder/vgclient.desktop"

echo "\033[1m\033[32mVideoGrace Client has been completely deleted\033[m"
echo "We hope to see you again at: \033[1m\033[36mhttps://videograce.com\033[m \033[1m\033[31m❤️\033[m"
