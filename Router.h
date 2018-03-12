#ifndef ROUTER_H
#define ROUTER_H

#define ROUTE_ALL 	0
#define ROUTE_SHP	1
#define ROUTE_SDP	2
#define ROUTE_STP	3
#define ROUTE_FTP	4
#define ROUTE_MGP	5

void route(int code);

void routeAll();
void routeSHP();
void routeSDP();
void routeSTP();
void routeFTP();
void routeMGP();

#endif