#ifndef GLOBALS_H
#define GLOBALS_H

#include <QList>
#include <list>
#include <QColor>

#include "idfeature.h"
#include "knowndevice.h"

class BleDeviceManager;
class MainWindow;

extern QList<IdFeature>   g_IdFeatures;
extern QList<KnownDevice> g_KnownDevices;
extern BleDeviceManager  *g_BleDeviceManager;
extern MainWindow        *g_MainWindow;


int   getIncrementalTime();
void idleAway(int millisecs);
void setBackgroundColour(QWidget *widget, const QColor& colour);
void setForegroundColour(QWidget *widget, const QColor& colour);


#endif // GLOBALS_H


