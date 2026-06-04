#ifndef USB_HANDLER_H
#define USB_HANDLER_H

#include <libserialport.h>
int send_and_get_reply(const char* port_name, const char* data, char **reply);
int get_device_json(char** return_json);

#endif // USB_HANDLER_H