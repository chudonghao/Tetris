//
// Created by chudonghao on 22-7-9.
//

#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("chudonghao");
  QCoreApplication::setOrganizationDomain("chudonghao.com");
  QCoreApplication::setApplicationName("Tetris");

  MainWindow mw;
  mw.show();

  return QApplication::exec();
}
