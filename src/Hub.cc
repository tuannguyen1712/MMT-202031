//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Hub.h"

Define_Module(Hub);

void Hub::initialize() {
    x = par("x").doubleValue();
    y = par("y").doubleValue();
    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);
}

void Hub::handleMessage(cMessage *msg) {
    int arrivalGate = msg->getArrivalGate()->getIndex();
    EV << "Received message from port " << arrivalGate << "\n";
    for (int i = 0; i < 10; i++) {
        if (i != arrivalGate) {
            char hostname[10];
            sprintf(hostname, "host[%d]", i);
            host = getModuleByPath(hostname);
            double hostX = host->par("x").doubleValue();
            double hostY = host->par("y").doubleValue();
            double dist = std::sqrt(
                    (x - hostX) * (x - hostX)
                            + (y - hostY) * (y - hostY));
            propDelay = (simtime_t) (dist / 200000000);
            sendDelayed(msg->dup(), propDelay, "port$o", i);
            EV << "Sent " << msg->str() << "message to port " << i << "at "<< simTime() + propDelay << "\n";
        }
    }
    delete msg;

}
