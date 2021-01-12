#ifndef JSON_H
#define JSON_H

void create_directories(tm* now);
void update_json_general(int entradas, int salidas, tm* now);
void update_json_year(int entradas, int salidas, tm* now);
void update_json_month(int entradas, int salidas, tm* now);
void update_json_day(int entradas, int salidas, tm* now);

#endif