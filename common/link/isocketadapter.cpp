#include "isocketadapter.h"

ISocketAdapter::ISocketAdapter()
{
    connect(this, &ISocketAdapter::trSendByteArray,
            this, &ISocketAdapter::sendByteArray);
}

ISocketAdapter::~ISocketAdapter()
{

}
