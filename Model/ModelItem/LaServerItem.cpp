#include "LaServerItem.h"

LaServerItem::LaServerItem(int id, QString name, QString address)
{
    setId(id);
    setName(name);
    setLatency(0);
    setAddress(address);
    setConnected(false);
}
