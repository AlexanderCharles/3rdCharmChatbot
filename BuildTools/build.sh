#!/bin/bash

mkdir -p Build/;

CC="gcc"
STD_LIBS=""
LIBS="-lX11 -lXtst -lsqlite3"
OPTS="-Wextra -Wall -o Build/3CCB-Debug"
CFLAGS="-std=c89 -pedantic -g"

$CC main.c                                         \
    utils/String.c utils/Time.c utils/Keyboard.c   \
    input/Parse.c input/Stream.c                   \
    message_builder/Reactions.c                    \
    $CFLAGS $STD_LIBS $LIBS $OPTS;
