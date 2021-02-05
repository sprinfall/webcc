#include "examples/qt_app_server/main_window.h"

#include <QDebug>

MainWindow::MainWindow(Context* context, QWidget* parent)
    : QMainWindow(parent) {
  setWindowTitle("Qt App Server");

#if 1
  connect(context, &Context::SomeApiSignal, this, &MainWindow::OnSomeApiSignal);
#else
  // NOTE: Seems not necessary to use QueuedConnection (because we are using
  // std::thread instead of QThread?).
  connect(context, &Context::SomeApiSignal, this, &MainWindow::OnSomeApiSignal,
          Qt::QueuedConnection);
#endif
}

void MainWindow::OnSomeApiSignal(const QString& text) {
  qDebug() << text;
}
