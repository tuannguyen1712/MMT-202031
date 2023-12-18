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

#ifndef __TESSSTTT_HOST_H_
#define __TESSSTTT_HOST_H_

#include <omnetpp.h>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Host : public cSimpleModule
{
private:
    simtime_t slotTime;
    simtime_t propDelay;
    simtime_t procDelay;
    double x, y;
    simtime_t schedule;
    simtime_t backoff;                      // back off
    unsigned int attempt;                            // for collision back off;
    unsigned int k;                                  // for wait idle channel
    unsigned int txMss, sccMss, Abort, TxRequest;
    unsigned int backoffTimes;
    cPar *iaTime;
    cMessage *bgn;
    cMessage *end;
    cMessage *bgnRq;
    cMessage *endRq;
    cMessage *jamRq;
    cMessage *jam;
    bool Tx, state, rcvjam, rcvbgn;
    cModule *hub;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    simtime_t getNextTransmissionTime();
    simtime_t ScheduleTransmissionTime();
    simtime_t getRandomTransmissionTime();
    void Debugss();
  public:
    virtual ~Host();
};

#endif
