Клиентское приложение системы видеоконференцсвязи VideoGrace
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Установка производится в домашнюю директорию пользователя 
/home/user/.vgclient/ sudo не требуется

Установка
~~~~~~~~~

1. Выполните: ./install.sh
2. Запустите ~/.vgclient/VideoGraceClient из терминала 
или при помощи иконки на рабочем столе. При первом запуске иконки 
разрешите ее выполнение.


Если программа не запускается, нужно доставить две библиотеки:
На debian-образных системах:

sudo apt install libxcb-cursor0
sudo apt install libxcb-ewmh2

На rpm-образных:

sudo yum install xcb-util-cursor 
sudo yum install xcb-util-wm

Обновление осуществляется автоматически с сервера

Удаление
~~~~~~~~

1. Выполните sudo ./uninstall.sh и/или rm -rf ~/.vgclient/

Контакты
~~~~~~~~

Сайт: https://videograce.ru/
Email: contact@videograce.com
Telegram: @udattsk
GitHub: https://github.com/ud84
