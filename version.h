#ifndef APP_DEFINES_H
#define APP_DEFINES_H

#define MAJOR_VER        0
#define MINOR_VER        2
#define RELEASE_VER      0
#define BUILD_VER        8
#define APP_VER          MAJOR_VER,MINOR_VER,RELEASE_VER,BUILD_VER
#define VER_STR_(VER)    #VER
#define VER_STR(VER)     VER_STR_(VER) "\0"

#define  ORIGFILENAME    "ColorPicker.exe"
#define  APPNAME         "ColorPicker"
#define  FILEDESCRIPTION "ColorPicker"
#define  LEGALCOPYRIGHT  "2020, Konstantin A. Pankov"
#define  PRODUCTSUPPORT  "info@kapankov.ru"

#endif
