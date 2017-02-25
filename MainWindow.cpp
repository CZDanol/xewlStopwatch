#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "ScopeExit.h"
#include "usbProtocol.h"

#include <QMessageBox>
#include <QFontDatabase>
#include <QDebug>

using namespace usbSensor_protocol;

QString msecsToString( unsigned long msecs ) {
	return QString("%3:%2:%1").arg( QString::number( msecs % 1000 ), 3, '0' ).arg( QString::number( msecs / 1000 % 60 ), 2, '0' ).arg( QString::number( msecs / 60000 ), 3, '0' );
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	connect( &dataReceiver, SIGNAL(timeout()), this, SLOT(receiveMessages()) );
	connect( &dataReceiver, SIGNAL(timeout()), this, SLOT(updateTimes()) );
	connect( &barrierOkBlinkTimer, SIGNAL(timeout()), this, SLOT(updateBarrierOkBlink()) );

	dataReceiver.setInterval( 16 );
	dataReceiver.start();

	barrierOkBlinkTimer.setInterval( 500 );

	emit onConnectedChanged();

	{
		int fontId = QFontDatabase::addApplicationFont(":/digital_7_mono.ttf");
		QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);

		{
			QFont fnt( family );
			fnt.setPointSize(50);
			ui->lblLapTime->setFont( fnt );
		}

		{
			QFont fnt( family );
			fnt.setPointSize(20);
			ui->lstTimes->setFont( fnt );
		}
	}

	on_btnConnect_clicked();
	setBarrierOk( true );
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onConnectedChanged()
{
	static const QPixmap connStat[3] = {
		QPixmap(":/icon/16x/cross_16x.png"),
		QPixmap(":/icon/16x/tick_16x.png"),
		QPixmap(":/icon/16x/warning_16x.png")
	};

	int index = ( handle_ != nullptr );
	if( index == 1 && !isSignalOk_ )
		index = 2;

	ui->lblConnectionStatus->setPixmap( connStat[ index ] );
}

void MainWindow::receiveMessages()
{
	if( !handle_ )
		return;

	if( lastPong.secsTo( QDateTime::currentDateTime() ) >= 2 ) {
		QMessageBox::critical( this, tr("Chyba"), tr("Senzor přestal odpovídat") );
		hid_close( handle_ );
		handle_ = nullptr;
		onConnectedChanged();
		return;
	}

	uint8_t reportBuf[packetSize + 1];
	// Report ID we are expecting (always 0)
	reportBuf[0] = 0;

	if( hid_get_feature_report( handle_, reportBuf, sizeof(reportBuf) ) == 0 )
		return;

	switch( reportBuf[1] ) {

	case pktIAmStillAlive:
		{
			auto pkt = (Packet_IAmStillAlive*) ( reportBuf + 2 );

			lastPong = QDateTime::currentDateTime();
			setBarrierOk( pkt->isBarrierOk );
			isSignalOk_ = pkt->isSignalQualityOk;
			onConnectedChanged();

			qDebug() << pkt->convolutionAvg;
		}
		break;

	case pktTrigger:
		emit onTrigger();
		break;

	}
}

void MainWindow::onTrigger()
{
	if( startTime.isNull() ) {
		startTime = QDateTime::currentDateTime();
		lap = 1;
		return;
	}

	unsigned long time = startTime.msecsTo( QDateTime::currentDateTime() );
	ui->lstTimes->addItem( QString("%1.%2").arg( lap, 2 ).arg( msecsToString( time ) ) );
	ui->lstTimes->scrollToBottom();

	lap ++;
	startTime = QDateTime::currentDateTime();
}

void MainWindow::updateTimes()
{
	unsigned long time = 0;

	if( !startTime.isNull() )
		time = startTime.msecsTo( QDateTime::currentDateTime() );

	ui->lblLapTime->setText( msecsToString( time ) );
}

void MainWindow::updateBarrierOkBlink()
{
	ui->lblLapTime->setStyleSheet( QString( "padding: 5px; color: black; background: %1;" ).arg( barrierOkBlinkRed_ ? "red" : "white" ) );
	barrierOkBlinkRed_ = !barrierOkBlinkRed_;
}

void MainWindow::on_btnConnect_clicked()
{
	hid_device_info *devices = hid_enumerate( 0x16c0, 0x27d9 );
	SCOPE_EXIT( hid_free_enumeration( devices ) );

	if( handle_ )
		hid_close( handle_ );

	for( hid_device_info *dev = devices; dev; dev = dev->next ) {
		const QString manufacturer = QString::fromWCharArray( dev->manufacturer_string );
		const QString product = QString::fromWCharArray( dev->product_string );

		if( manufacturer != "straw-solutions.cz" || product != "Times Machine Light barrier" )
			continue;

		handle_ = hid_open( dev->vendor_id, dev->product_id, dev->serial_number );
		if( !handle_ )
			break;

		hid_set_nonblocking( handle_, 1 );

		lastPong = QDateTime::currentDateTime();

		emit onConnectedChanged();
		return;
	}

	emit onConnectedChanged();
	QMessageBox::critical( this, tr("Chyba"), tr("Senzor nebyl nalezen. Zkontrolujte, zda existuje. Pokud jste na Linuxu, přidejte udev pravidlo.") );
}

void MainWindow::on_btnStart_clicked()
{
	onTrigger();
}

void MainWindow::on_btnStop_clicked()
{
	startTime = QDateTime();
}

void MainWindow::on_btnClear_clicked()
{
	ui->lstTimes->clear();
	lap = 1;
}

void MainWindow::setBarrierOk(bool set)
{
	if( barrierIsOk_ == set )
		return;

	barrierIsOk_ = set;

	static const QPixmap pixmaps[2] {
		QPixmap(":/icon/16x/line_split_16x.png"),
		QPixmap(":/icon/16x/draw_line_16x.png")
	};

	ui->lblInterruptStatus->setPixmap( pixmaps[ set ] );

	static const char *texts[2] = {
		QT_TR_NOOP("Paprsek přerušen"),
		QT_TR_NOOP("Ok")
	};

	ui->lblInterruptStatusText->setText( tr( texts[ set ] ) );

	if( set ) {
		barrierOkBlinkTimer.stop();
		barrierOkBlinkRed_ = false;
		updateBarrierOkBlink();
	}
	else {
		barrierOkBlinkTimer.start();
		barrierOkBlinkRed_ = true;
	}
}
