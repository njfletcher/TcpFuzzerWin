#pragma once

#include <cstdint>
#include "packet.h"
#include <utility>

#define Unspecified 0
#define keyLen 16 //128 bits = 16 bytes recommended by RFC 6528

typedef pair<uint32_t, uint16_t> LocalPair;
typedef pair<uint32_t, uint16_t> RemotePair;
typedef unordered_map<LocalPair, unordered_map<RemotePair, Tcb> > ConnectionMap;

#define dynPortStart 49152
#define dynPortEnd 65535

enum class Code{

  Success = 0,
  RawSock = -1,
  ProgramError = -2,
  BadIncPacket = -3,
  ActiveUnspec = -20,
  Resources = -21,
  DupConn = -22,
  ConnRst = -23

};

enum class State{
  Listen,
  SynSent,
  SynRec,
  Established,
  FinWait1,
  FinWait2,
  CloseWait,
  Closing,
  LastAck,
  TimeWait,
  Closed
};

enum class Event{
  None,
  //user events
  Open,
  Send,
  Receive,
  Close,
  Abort,
  Status,
  //inner events
  Segment
};

class Tcb{

  public:
    Tcb() = default;
    Tcb(LocalPair l, RemotePair r, uint8_t passive);
    
    int id;
    
    //all multi-byte fields are guaranteed to be in host order.
    LocalPair lP;
    RemotePair rP;

    std::vector<TcpPacket> retransmit;
    
    uint32_t sUna; // first seq num of data that has not been acknowledged by my peer.
    uint32_t sNxt; // first seq num of data that has not been sent by me.
    uint32_t sWnd; // window specified by my peer. how many bytes they can hold in buffer.
    uint32_t iss; //initial sequence number i chose for my data.
    
    uint32_t sUp; 
    uint32_t sWl1;
    uint32_t sWl2;


    uint32_t rNxt; // first seq num of data I have not received from my peer.
    uint32_t rWnd; // window advertised by me to my peer. how many bytes i can hold in buffer.
    uint32_t rUp;
    uint32_t irs; // initial sequence number chosen by peer for their data.
    
    std::function<int(Tcb&, TcpPacket&, int)> stateLogic;
    //0 active; 1 passive
    uint8_t passiveOpen;
  
};


class Event{};

class OpenEv : public Event{
  public:
    uint8_t passive;
};
class SegmentEv : public Event{
  public:
    IpPacket& packet;
};

class State{
  public:
    virtual Code processEvent(int socket, Tcb& b, Event& e);
    virtual Code processEvent(int socket, Tcb& b, OpenEv& oe);
    virtual Code processEvent(int socket, Tcb& b, SegmentEv& se);
};

class ListenS : State{
  public:
    Code processEvent(int socket, Tcb& b, Event& e);
    Code processEvent(int socket, Tcb& b, OpenEv& oe);
    Code processEvent(int socket, Tcb& b, SegmentEv& se);
    Code processEvent(int socket, Tcb& b, SendEv& se);
    Code processEvent(int socket, Tcb& b, RecEv& se);
    Code processEvent(int socket, Tcb& b, CloseEv& se);
    Code processEvent(int socket, Tcb& b, AbortEv& se);
    Code processEvent(int socket, Tcb& b, StatusEv& se);
};












































