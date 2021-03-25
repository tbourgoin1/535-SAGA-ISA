#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "memory.h"
#include "pipe.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_step_clicked();
    void update_ram_cache(pipe p);
    void update_register(pipe p);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
