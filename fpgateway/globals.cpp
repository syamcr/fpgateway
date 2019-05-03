
#include <QApplication>
#include <QWidget>
#include <QPalette>

#include "globals.h"

QList<IdFeature>   g_IdFeatures;
QList<KnownDevice> g_KnownDevices;
BleDeviceManager  *g_BleDeviceManager;
MainWindow        *g_MainWindow = nullptr;


/**
 * @brief  Get the incremental PC time
 * @return Returns the time elapsed (in milliseconds) between the first call and the current call to this function
 *
 * When called for the first time, this function returns 0 or negative integer on failure.
 * Subsequent calls to this function will return the number of milliseconds elapsed from that first call.
 * @note This function uses clock_gettime() with CLOCK_MONOTONIC.
 * @see clock_gettime()
 */
int getIncrementalTime()
{

    static timespec start_time;
    static bool first_time = true;

    if(first_time)
    {
        first_time = false;
        int ret = clock_gettime(CLOCK_MONOTONIC, &start_time);
        return ret;
    }
    else
    {
        timespec curr_time;
        (void)clock_gettime(CLOCK_MONOTONIC, &curr_time);

        return static_cast<int>((curr_time.tv_sec - start_time.tv_sec)*1000 + (curr_time.tv_nsec - start_time.tv_nsec)/1000000);
    }

}



/**
   @brief Idle for specified time, handling messages if any
   @param millisecs The time duration (in milliseconds) to spend idling

   This function is a mechanism to 'sleep' while still handling all event messages.
   The function will return only after the specified time is elapsed.<br>

   @note If millisecs is zero or negative, then this function will return immediately.
**/
void idleAway(int millisecs)
{
    if(millisecs <= 0) return;
    const int start_time = getIncrementalTime();
    while(true)
    {
        int elapsed_time = getIncrementalTime() - start_time;
        if(elapsed_time >= millisecs) break;
        QApplication::processEvents(QEventLoop::AllEvents, millisecs - elapsed_time);
    }

}


/**
  @brief Change the background colour of a widget
  @param [in] widget The widget's whose colour is to be changed
  @param colour The desired background colour
**/
void setBackgroundColour(QWidget *widget, const QColor& colour)
{
    widget->setAutoFillBackground(true);
    QPalette palette(widget->palette());
    palette.setColor(widget->backgroundRole(), colour);
    widget->setPalette(palette);
}



/**
  @brief Change the foreground colour of a widget
  @param [in] widget The widget's whose colour is to be changed
  @param colour The desired foreground colour
**/
void setForegroundColour(QWidget *widget, const QColor& colour)
{
    widget->setAutoFillBackground(true);
    QPalette palette(widget->palette());
    palette.setColor(widget->foregroundRole(), colour);
    widget->setPalette(palette);
}


