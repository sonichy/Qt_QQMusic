s="[Desktop Entry]\nName=QQ音乐\nComment=Media player\nExec=`pwd`/QQMusic\nIcon=`pwd`/icon.png\nPath=`pwd`\nTerminal=false\nType=Application\nCategories=AudioVideo;"
echo -e $s > QQMusic.desktop
cp `pwd`/QQMusic.desktop ~/.local/share/applications/QQMusic.desktop
