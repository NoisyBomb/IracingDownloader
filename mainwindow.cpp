#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "authmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    AuthManager* auth = new AuthManager(this);
    connect(auth, &AuthManager::loginSucceeded, this, [](const QString& token){
        qDebug() << "Токен:" << token;
    });
    auth->startLogin();

    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}
