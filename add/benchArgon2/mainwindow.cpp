#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../../DCPCryptCpp/3rdparty/Argon2/argon2.h"

#include <chrono>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton,  SIGNAL(clicked()), this, SLOT(onClick()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

class StopWatch
{
public:
    chrono::time_point<chrono::high_resolution_clock> a, b;
    void start() { a = chrono::high_resolution_clock::now(); }
    void stop() { b = chrono::high_resolution_clock::now(); }
    double duration()
    {
        chrono::duration<double> elapsed_seconds = b - a;
        return elapsed_seconds.count();
    }
};

void MainWindow::onClick()
{
  unsigned char digest[256/8],password[256/8];
  typedef char* pchar;
  strcpy_s(pchar(password), sizeof(password), "password");
  int nLoops = ui->lineEdit->text().toInt();
  int nKiB = ui->lineEdit_2->text().toInt();
  int nThr = ui->lineEdit_3->text().toInt();
  argon2::Argon2_Context context(digest, sizeof(digest), password, strlen(pchar(password)),
                                 nullptr, 0, nullptr, 0, nullptr, 0,
                      nLoops, nKiB, nThr, nThr,nullptr,nullptr,false,false, false,false);
  StopWatch st;
  st.start();
  argon2::Argon2id(&context);
  st.stop();
  QMessageBox::information(this, "time",QString::number(st.duration()));
}
