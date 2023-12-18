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

#include "Host.h"
#include "math.h"

Define_Module(Host);

Host::~Host() {
    delete bgn;
    delete end;
    cancelAndDelete(bgnRq);
    cancelAndDelete(endRq);

}

void Host::initialize() {
    slotTime = par("slotTime");
    iaTime = &par("iaTime");
    hub = getModuleByPath("hub");

    TxRequest = 0;
    txMss = 0;
    sccMss = 0;
    backoffTimes = 0;
    Abort = 0;

    attempt = 0;
    k = 0;
    x = par("x").doubleValue();
    y = par("y").doubleValue();
    bgn = new cMessage("BeginMss");
    end = new cMessage("EndMss");
    bgnRq = new cMessage("BeginRequest");
    endRq = new cMessage("EndRequest");
    jamRq = new cMessage("JamRequest");
    jam = new cMessage("JamSignal");
    rcvbgn = 0;
    rcvjam = 0;
    state = 0;          // idle
    Tx = 0;             // not in Transmit

    double hubX = hub->par("x").doubleValue();
    double hubY = hub->par("y").doubleValue();
    double dist = std::sqrt((x - hubX) * (x - hubX) + (y - hubY) * (y - hubY));
    propDelay = (simtime_t) (dist / 200000000);
    double h = 12000;                    // choose pk len (bit)
    procDelay = (simtime_t) (h / 1000000);
    // display host in stimulation plane
    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);

    schedule = getRandomTransmissionTime();
    scheduleAt(schedule, bgnRq);
    EV << "Host " << this->getIndex() << " start transmit at " << schedule
              << "Prop delay: " << propDelay << " Proc delay: " << procDelay
              << " jam duration:" << double(48) / 1000000 << " slot Time " << slotTime <<  endl;
    Debugss();
}

void Host::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        if (strcmp(msg->getName(), "BeginRequest") == 0) {
            TxRequest++;
            if (state) {
                backoffTimes++;
                backoff = getNextTransmissionTime();
                if (bgnRq != NULL && backoff > 0) {
                    bgnRq = new cMessage("BeginRequest");
                    cancelEvent(bgnRq);
                    scheduleAt(backoff, bgnRq);
                    EV << "Host " << this->getIndex()
                              << " Busy channel(state = 1), back off "
                              << backoff << endl;
                } else {
                    k = 0;
                    Abort++;
                    schedule = getRandomTransmissionTime();
                    scheduleAt(schedule, bgnRq);
                    EV << "Host " << this->getIndex()
                              << " abort frame, transmit again at " << schedule
                              << endl;
                }
                Debugss();
            } else {
                Tx = 1;
                k = 0;
                cancelEvent(endRq);
                scheduleAt(simTime() + procDelay, endRq);
                bgnRq = new cMessage("BeginRequest");
//                send(bgn->dup(), "port$o");
                sendDelayed(bgn->dup(), propDelay, "port$o");
                txMss++;
                EV << "Host " << this->getIndex()
                          << " Start Transmit! and will transmit end bit at "
                          << simTime() + procDelay << "\n";
                Debugss();
            }
        }
        if (strcmp(msg->getName(), "EndRequest") == 0) {
            Tx = 0;
            backoff = getRandomTransmissionTime();
            if (bgnRq != NULL) {
                cancelEvent(bgnRq);
                scheduleAt(backoff, bgnRq);
                EV << "Host " << this->getIndex()
                          << " Complete Transmit! Transmit again at " << backoff
                          << "\n";
//                send(end->dup(), "port$o");
                sendDelayed(end->dup(), propDelay, "port$o");
                sccMss++;
                Debugss();
            } else
                EV << "bgnRq = NULL\n";
        }
        if (strcmp(msg->getName(), "JamRequest") == 0) {

            rcvbgn = 0;
            rcvjam = 0;
            sendDelayed(jam->dup(), propDelay, "port$o");
            EV << "Host " << this->getIndex() << " Start transmit jam signal "
                      << endl;
            Debugss();
        }
    } else {
        if (strcmp(msg->getName(), "BeginMss") == 0) {
            if (!Tx) {
                rcvjam = 0;
                state = 1;
                EV << "Host " << this->getIndex()
                          << " Receive begin message \n";
                Debugss();
            } else if (Tx && !rcvbgn) {
                rcvbgn = 1;
                cancelEvent(endRq);
                scheduleAt(simTime() + simtime_t(48 / 1000000), jamRq);
                EV << "Host " << this->getIndex() << " Collision!" << endl;
                Debugss();
            }
        } else if (strcmp(msg->getName(), "EndMss") == 0 && state) {
            state = 0;
            EV << "Host " << this->getIndex() << " Receive end message \n";
            Debugss();
        } else if (strcmp(msg->getName(), "JamSignal") == 0 && !rcvjam && Tx) {
            Tx = 0;
            state = 0;
            rcvjam = 1;
            backoff = ScheduleTransmissionTime();
            cancelEvent(endRq);
            cancelEvent(bgnRq);
            if (attempt < 11) {
                scheduleAt(backoff, bgnRq);
                EV << "Host " << this->getIndex()
                          << " receive jam signal. Transmit again at "
                          << backoff << endl;
                Debugss();
            } else {
                backoff = getRandomTransmissionTime();
                EV << "Host " << this->getIndex()
                          << " abort frame! and will transmit another frame at "
                          << backoff << endl;
                Debugss();
            }
        }
    }
}
simtime_t Host::getNextTransmissionTime() {        // back off when channel busy
    if (k < 16) {
        k++;
        simtime_t t = simTime() + slotTime * intrand(pow(2, k) + 1, 0);
        return slotTime * ceil(t / slotTime);
    } else
        return 0;
}

simtime_t Host::ScheduleTransmissionTime() {          // back off when collision
    if (attempt <= 10) {
        attempt++;
        simtime_t t = simTime() + slotTime * intrand(pow(2, attempt) + 1, 0);
        return slotTime * ceil(t / slotTime);
    } else
        attempt = 11;
    return 0;
}

simtime_t Host::getRandomTransmissionTime() {
    simtime_t t = simTime() + iaTime->doubleValue();
    return slotTime * ceil(t / slotTime);
}

void Host::Debugss() {
    EV << "Tx: " << Tx << " state: " << state << " attempt: " << attempt
              << " k: " << k << endl;
}
void Host::finish() {
    recordScalar("Back off", backoffTimes);
    recordScalar("Transmit", txMss);
    recordScalar("Success", sccMss);
    recordScalar("Abort", Abort);
    recordScalar("Attempt", attempt);
    recordScalar("Transmit Request", TxRequest);
}
