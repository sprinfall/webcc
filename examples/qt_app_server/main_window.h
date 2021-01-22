#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>

// Some global QObject with public signals for ApiView to inform the handling
// of client requests.
// "Context" is a bad name, you should change it.
class Context : public QObject {
  Q_OBJECT

public:
  explicit Context(QObject* parent = nullptr) : QObject(parent) {
  }

  // Public signals emitted from ApiView:
signals:
  void SomeApiSignal(const QString& text);
  // Add other signals here...
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(Context* context, QWidget* parent = nullptr);

private:
  // You should encapsulate the required information as the signal parameters.
  // I use QString just for demostrating how to transfer data from ApiView.
  void OnSomeApiSignal(const QString& text);
};

#endif  // MAIN_WINDOW_H_
