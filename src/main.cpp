#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Modern Dark Theme QSS
    app.setStyleSheet(R"(
        QWidget {
            background-color: #1e1e1e;
            color: #d4d4d4;
            font-family: 'Segoe UI', sans-serif;
            font-size: 10pt;
        }
        QGroupBox {
            border: 2px solid #333333;
            border-radius: 6px;
            margin-top: 1.2em;
            font-weight: bold;
            color: #569cd6;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 3px 0 3px;
        }
        QPushButton {
            background-color: #333333;
            border: 1px solid #454545;
            border-radius: 4px;
            padding: 6px 12px;
            color: white;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #454545;
        }
        QPushButton:pressed {
            background-color: #2d2d2d;
        }
        QPushButton:disabled {
            color: #666666;
            background-color: #252525;
        }
        QTableWidget {
            background-color: #252525;
            alternate-background-color: #2d2d2d;
            gridline-color: #333333;
            border: none;
            selection-background-color: #264f78;
        }
        QHeaderView::section {
            background-color: #333333;
            color: #cccccc;
            padding: 4px;
            border: 1px solid #1e1e1e;
        }
        QListWidget {
            background-color: #252525;
            border: 1px solid #333333;
            padding: 5px;
        }
        QLabel#statusLabel {
            color: #4ec9b0;
        }
        QLabel#statValue {
            color: #ce9178;
            font-weight: bold;
        }
        QSlider::groove:horizontal {
            border: 1px solid #333333;
            height: 4px;
            background: #252525;
            margin: 2px 0;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #569cd6;
            border: 1px solid #569cd6;
            width: 14px;
            height: 14px;
            margin: -6px 0;
            border-radius: 7px;
        }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}
