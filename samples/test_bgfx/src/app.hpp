struct sapp_event;

void app_init(void);
void app_frame(void);
void app_cleanup(void);
void app_event(const sapp_event* );

void app_set_touch_point_count(int count);

void * app_get_window_hdl();

