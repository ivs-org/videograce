#!/bin/sh

SCRIPT=$(readlink -f "$0")
DISTRPATH=$(dirname "$SCRIPT")

mkdir -p ~/.vgclient/
mkdir -p ~/.vgclient/res
cp -n $DISTRPATH/vgclient/vgclient.conf ~/.vgclient/
chmod 666 ~/.vgclient/vgclient.conf

cp -R $DISTRPATH/vgclient/res/* ~/.vgclient/res/
cp $DISTRPATH/vgclient/VideoGraceClient ~/.vgclient/
chmod 777 ~/.vgclient/VideoGraceClient

currentuser=${SUDO_USER:-$USER}
desktop_folder=`xdg-user-dir DESKTOP`
cp $DISTRPATH/vgclient/vgclient.desktop "$desktop_folder/vgclient.desktop"
chown $currentuser:$currentuser "$desktop_folder/vgclient.desktop"
chmod +x "$desktop_folder/vgclient.desktop"

echo "Exec=$HOME/.vgclient/VideoGraceClient" >> "$desktop_folder/vgclient.desktop"
echo "Icon=$HOME/.vgclient/res/main_icon.png" >> "$desktop_folder/vgclient.desktop"

echo "\033[1m\033[32mVideoGrace Client installed successfully\033[m"
echo "Run the application: \033[1m\033[33m~/.vgclient/VideoGraceClient\033[m or from the desktop's icon"
echo "More information on the our site: \033[1m\033[36mhttps://videograce.com\033[m \033[1m\033[31m🤝\033[m"