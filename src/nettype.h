#ifndef NETTYPE_H
#define NETTYPE_H

#include <Arduino.h>

struct NetworkConn {
    String ssid;
    String passwd;

    NetworkConn(String ssid, String passwd) : ssid(ssid), passwd(passwd) {}
};

#endif