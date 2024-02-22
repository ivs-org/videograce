# Установка VideoGrace Client на Linux

Установка производится в пользовательскую папку пользователя ~/.vgclient/ sudo не требуется

## Установка

Скачайте дистрибутив VideoGrace Client для Linux

    wget https://videograce.ru/download/VideoGraceClient-2.0.240220-x64.tar.bz2

Распакуйте архив

    tar vxf VideoGraceClient-2.0.240220-x64.tar.bz2

Перейдите в него и запустите установщик 

    cd VideoGraceClient-2.0.240220-x64
    ./install.sh

Запустите 

    ~/.vgclient/VideoGraceClient 
    
из терминала или при помощи иконки на рабочем столе. При первом запуске иконки 
разрешите ее выполнение.


Если программа не запускается, нужно доставить две библиотеки:
На debian-образных системах:

    sudo apt install libxcb-cursor0
    sudo apt install libxcb-ewmh2

На rpm-образных:

    sudo yum install xcb-util-cursor 
    sudo yum install xcb-util-wm

## Обновление
Обновление осуществляется автоматически с сервера

## Удаление

Выполните 
    
    rm -rf ~/.vgclient/
