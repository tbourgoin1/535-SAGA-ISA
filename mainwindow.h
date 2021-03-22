/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QListView *ram_view;
    QLabel *label;
    QListView *cache_view;
    QLabel *label_2;
    QLabel *label_3;
    QListView *register_view;
    QTextEdit *data_input;
    QLabel *label_4;
    QComboBox *memory_select;
    QLabel *label_5;
    QTextEdit *address_input;
    QLabel *label_6;
    QPushButton *write_button;
    QPushButton *run_button;
    QPushButton *step_button;
    QPushButton *reset_button;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1201, 658);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        ram_view = new QListView(centralwidget);
        ram_view->setObjectName(QStringLiteral("ram_view"));
        ram_view->setGeometry(QRect(0, 40, 251, 331));
        label = new QLabel(centralwidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(100, 10, 41, 21));
        cache_view = new QListView(centralwidget);
        cache_view->setObjectName(QStringLiteral("cache_view"));
        cache_view->setGeometry(QRect(280, 40, 251, 331));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(370, 10, 61, 16));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(640, 10, 91, 16));
        register_view = new QListView(centralwidget);
        register_view->setObjectName(QStringLiteral("register_view"));
        register_view->setGeometry(QRect(560, 40, 251, 331));
        data_input = new QTextEdit(centralwidget);
        data_input->setObjectName(QStringLiteral("data_input"));
        data_input->setGeometry(QRect(110, 480, 291, 31));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(20, 400, 90, 26));
        memory_select = new QComboBox(centralwidget);
        memory_select->setObjectName(QStringLiteral("memory_select"));
        memory_select->setGeometry(QRect(110, 400, 100, 34));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(20, 440, 90, 26));
        address_input = new QTextEdit(centralwidget);
        address_input->setObjectName(QStringLiteral("address_input"));
        address_input->setGeometry(QRect(110, 440, 291, 31));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(20, 480, 90, 26));
        write_button = new QPushButton(centralwidget);
        write_button->setObjectName(QStringLiteral("write_button"));
        write_button->setGeometry(QRect(110, 520, 114, 34));
        run_button = new QPushButton(centralwidget);
        run_button->setObjectName(QStringLiteral("run_button"));
        run_button->setGeometry(QRect(560, 400, 114, 34));
        step_button = new QPushButton(centralwidget);
        step_button->setObjectName(QStringLiteral("step_button"));
        step_button->setGeometry(QRect(560, 440, 114, 34));
        reset_button = new QPushButton(centralwidget);
        reset_button->setObjectName(QStringLiteral("reset_button"));
        reset_button->setGeometry(QRect(560, 480, 114, 34));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 1201, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindow", "RAM", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindow", "Cache", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", "Registers", Q_NULLPTR));
        data_input->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:7.875pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">00000000000000000000000000000000</p></body></html>", Q_NULLPTR));
        label_4->setText(QApplication::translate("MainWindow", "Write to", Q_NULLPTR));
        memory_select->clear();
        memory_select->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "RAM", Q_NULLPTR)
         << QApplication::translate("MainWindow", "CACHE", Q_NULLPTR)
         << QApplication::translate("MainWindow", "REGISTER", Q_NULLPTR)
        );
        label_5->setText(QApplication::translate("MainWindow", "Address", Q_NULLPTR));
        address_input->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:7.875pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">00000000</p></body></html>", Q_NULLPTR));
        label_6->setText(QApplication::translate("MainWindow", "Data", Q_NULLPTR));
        write_button->setText(QApplication::translate("MainWindow", "WRITE", Q_NULLPTR));
        run_button->setText(QApplication::translate("MainWindow", "RUN", Q_NULLPTR));
        step_button->setText(QApplication::translate("MainWindow", "STEP", Q_NULLPTR));
        reset_button->setText(QApplication::translate("MainWindow", "RESET", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // MAINWINDOW_H
