struct sapp_event;

void appInitialize();
void appUpdate();
void appShutdown();

void appNotifyEvent(const sapp_event* );
void appNotifyGestureTouchCount(int count);
void appNotifyGestureScroll(int touch_count, float deltax, float deltay);

void * appGetWindowHdl();

void appTriggerGpuCapture();
void appStopGpuCapture();

