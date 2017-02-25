#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QTimer>
#include <hidapi/hidapi.h>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void onConnectedChanged();
	void receiveMessages();
	void onTrigger();
	void updateTimes();
	void updateBarrierOkBlink();

private slots:
	void on_btnConnect_clicked();
	void on_btnStart_clicked();
	void on_btnStop_clicked();
	void on_btnClear_clicked();

private:
	void setBarrierOk( bool set );

private:
	Ui::MainWindow *ui;
	hid_device *handle_ = nullptr;
	QDateTime lastPong, startTime;
	size_t lap = 0;
	QTimer dataReceiver, barrierOkBlinkTimer;
	bool barrierIsOk_, barrierOkBlinkRed_ = false, isSignalOk_ = true;

};

#endif // MAINWINDOW_H
