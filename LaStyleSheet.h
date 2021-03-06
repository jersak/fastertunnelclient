#ifndef LASTYLESHEET_H
#define LASTYLESHEET_H

#include <QString>

static QString LaLoginStyleSheet(
        "QFrame#GlossyFrame {"\
         "background: transparent;"\
        "}"\

        "#GlossyFrame QLabel{"\
         "background: transparent;"\
         "color: white;"\
        "}"\

        "#GlossyFrame QPushButton {"\
         "color: white;"\
         "border: 1px solid black;"\
         "border-radius: 3px;"\
         "padding: 1px;"\
         "background: qlineargradient("\
           "x1:0, y1:0, x2:0, y2:1,"\
           "stop:0 #bec1d2,"\
           "stop: 0.4 #717990,"\
           "stop: 0.5 #5c637d,"\
           "stop:1 #68778e"\
         ");"\
        "min-width: 68px;"\
        "min-height: 24px;"\
        "}"\

        "#GlossyFrame QPushButton:pressed {"\
        "background: qlineargradient("\
          "x1:0, y1:0, x2:0, y2:1,"\
          "stop:0 #68778e,"\
          "stop: 0.4 #5c637d,"\
          "stop: 0.5 #717990,"\
          "stop:1 #bec1d2"\
        ");"\
         "color: black;"\
       "}"\

       "QLineEdit {"\
         "background: qlineargradient("\
          "x1:0, y1:0, x2:0, y2:1,"\
          "stop:0 gray,"\
          "stop: 0.2 white"\
          "stop:1 white"\
          ");"\
         "border-radius: 1px;"\
         "border: 1px solid black;"\
         "min-height: 24px;"\
         "color: black;"\
        "}"\

        "#GlossyFrame QCheckBox {"\
             "color:  white;"\
        "}");

/*
        "qradialgradient("\
        "cx: 0.5, cy: -1.8,"\
        "fx: 0.5, fy: 0,"\
        "radius: 2,"\
        "stop: 0 #9aa9be,"\
        "stop: 1 #293859);"\
       "font: bold;"\
*/

static QString LaMainStyleSheet(
        "QFrame#GlossyFrame {"\
         "background: transparent;"\
        "}"\

        "#GlossyFrame QLabel{"\
         "background: transparent;"\
         "color: white;"\
        "}"\

        "#GlossyFrame QPushButton {"\
         "color: white;"\
         "border: 1px solid black;"\
         "border-radius: 3px;"\
         "padding: 1px;"\
         "background: qlineargradient("\
           "x1:0, y1:0, x2:0, y2:1,"\
           "stop:0 #bec1d2,"\
           "stop: 0.4 #717990,"\
           "stop: 0.5 #5c637d,"\
           "stop:1 #68778e"\
         ");"\
        "min-width: 68px;"\
        "min-height: 36px;"\
        "}"\

        "#GlossyFrame QPushButton:pressed {"\
        "background: qlineargradient("\
          "x1:0, y1:0, x2:0, y2:1,"\
          "stop:0 #68778e,"\
          "stop: 0.4 #5c637d,"\
          "stop: 0.5 #717990,"\
          "stop:1 #bec1d2"\
        ");"\
         "color: black;"\
       "}"\

       "#GlossyFrame QLineEdit {"\
         "background: qlineargradient("\
          "x1:0, y1:0, x2:0, y2:1,"\
          "stop:0 gray,"\
          "stop: 0.2 white"\
          "stop:1 white"\
          ");"\
         "border-radius: 1px;"\
         "border: 1px solid black;"\
         "min-height: 24px;"\
         "color: black;"\
        "}"\

        "#GlossyFrame QCheckBox {"\
             "color:  white;"\
        "}");

/*
           "qradialgradient("\
           "cx: 0.5, cy: 2.8,"\
           "fx: 0.5, fy: 1,"\
           "radius: 2,"\
           "stop: 0 #9aa9be,"\
           "stop: 1 #293859);"\
          "font: bold;"\
*/

static QString LaMainWindowStyleSheet(
        "QMainWindow#GlossyFrame {"\
         "background: qradialgradient("\
           "cx: 0.5, cy: -1.8,"\
           "fx: 0.5, fy: 0,"\
           "radius: 2,"\
           "stop: 0 #9aa9be,"\
           "stop: 1 #293859);"\
          "font: bold;"\
        "}");

static QString LaConnectButtonStyleSheet(
        "QPushButton {"\
         "color: white;"\
         "border: 1px solid black;"\
         "border-radius: 3px;"\
         "padding: 1px;"\
         "background: qlineargradient("\
           "x1:0, y1:0, x2:0, y2:1,"\
           "stop:0 #bec1d2,"\
           "stop: 0.4 #717990,"\
           "stop: 0.5 #5c637d,"\
           "stop:1 #68778e"\
         ");"\
        "min-width: 120px;"\
        "min-height: 22px;"\
        "}"\

        "QPushButton:pressed {"\
        "background: qlineargradient("\
          "x1:0, y1:0, x2:0, y2:1,"\
          "stop:0 #68778e,"\
          "stop: 0.4 #5c637d,"\
          "stop: 0.5 #717990,"\
          "stop:1 #bec1d2"\
        ");"\
         "color: black;"\
       "}");

#endif // LASTYLESHEET_H
