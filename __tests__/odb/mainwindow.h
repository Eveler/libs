#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QSharedPointer>

namespace Ui {
class MainWindow;
}

class Declar;
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  /*QSharedPointer<*/Declar*/*>*/ declar;
};

#endif // MAINWINDOW_H