#include "wallp.hpp"

#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QTemporaryFile>
#include <QImageWriter>
#include <QColorSpace>

void set_wallpaper(QImage img) {
	QProcess awesome_1, awesome_2;
	awesome_1.start("dbus-send --dest=org.awesomewm.awful --type=method_call --print-reply / org.awesomewm.awful.Remote.Eval string:\"coords = mouse.coords()\"");
	awesome_1.waitForFinished();
	qDebug() << awesome_1.readAllStandardOutput() << awesome_1.readAllStandardError();
	QTemporaryFile img_tmp {QDir::tempPath()+QDir::separator()+"f2_XXXXXX.bmp"};
	img_tmp.open();
	qDebug() << img_tmp.fileName();
	
	QImageWriter pngw;
	pngw.setFormat("BMP");
	pngw.setDevice(&img_tmp);
	pngw.write(img);
	
	awesome_2.start(QString("dbus-send --dest=org.awesomewm.awful --type=method_call --print-reply / org.awesomewm.awful.Remote.Eval string:\"local awful = require('awful') \n local gears = require('gears') \n gears.wallpaper.fit('") + img_tmp.fileName() + "', awful.screen.getbycoord(coords.x, coords.y), false)\"");
	awesome_2.waitForFinished();
	qDebug() << awesome_2.readAllStandardOutput() << awesome_2.readAllStandardError();
}
