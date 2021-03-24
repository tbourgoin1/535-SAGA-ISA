#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pipe.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_ram_cache()
{
    memory mem = get_mem();
    std::string ram_list = mem.get_ram()[0];
    for(int i = 1; i < 256; i++){
        ram_list = ram_list + "\n\n" + mem.get_ram()[i];
    }
    std::string cache_list = "";
    for(int i = 0; i < 16; i++){
        string txt = mem.get_cache()[i];
        string tag = txt.substr(0, 2); // 2 bits for tag
        string index = txt.substr(2, 4); // 4 bits for index
        string offset = txt.substr(6, 2);
        string dirty = txt.substr(8, 1);
        string valid = txt.substr(9, 1);
        string word_1 = txt.substr(10, 32);
        string word_2 = txt.substr(42, 32);
        string word_3 = txt.substr(74, 32);
        string word_4 = txt.substr(106, 32);
        cache_list = cache_list + tag + " " + index + " " + offset + " " + dirty + " " + valid + "\n" + word_1 + "\n" + word_2 + "\n" + word_3 + "\n" + word_4 + "\n\n";
    }
    QString ram = QString::fromStdString(ram_list);
    QString cache = QString::fromStdString(cache_list);
    ui->ram_browser->setText(ram);
    ui->cache_browser->setText(cache);
}

void MainWindow::update_register()
{
    string *reg = get_reg();
    std::string reg_list = reg[0];
    for(int i = 1; i < 16; i++){
        reg_list = reg_list + "\n\n" + reg[i];
    }
    QString regstr = QString::fromStdString(reg_list);
    ui->register_browser->setText(regstr);
}



void MainWindow::on_step_clicked()
{
    run_pipe();
    update_ram_cache();
    update_register();
}
