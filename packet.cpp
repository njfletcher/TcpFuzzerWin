#include "packet.h"
#include "network.h"
#include <iostream>

using namespace std;

IpPacket::IpPacket(): tcpPacket() {};

uint16_t TcpOption::calcSize(){
  uint16_t sz = 1; //kind
  if(hasLength){
    sz = sz + 1; //len byte
  }
  sz = sz + data.size();
  return sz;
}

uint16_t TcpOption::getSize(){
  return size;
}


TcpOption::TcpOption(uint8_t k, uint8_t len, uint8_t hasLen, vector<uint8_t> d): kind(k), length(len), hasLength(hasLen), data(d){
  size = calcSize();
};

void TcpOption::print(){

  cout << "==TcpOption==" << endl;
  cout << "kind: " << static_cast<unsigned int>(kind) << endl;
  cout << "length: " << static_cast<unsigned int>(length) << endl;
  cout << "hasLength: " << static_cast<unsigned int>(hasLength) << endl;
  cout << "data: [";
  for(int i = 0; i < data.size(); i++) cout << " " << static_cast<unsigned int>(data[i]);
  cout << "]" << endl;
  cout << "==========" << endl;


}

void TcpOption::toBuffer(vector<uint8_t>& buff){
  buff.push_back(kind);
  if(hasLength){
	  buff.push_back(length);
  }

  for(size_t i = 0; i < data.size(); i++){
	  buff.push_back(data[i]);
  }
}

//returns number of bytes read, -1 if error.
//numBytesRemaining val is assumed to be greater than 0.
int TcpOption::fromBuffer(uint8_t* buffer, int numBytesRemaining){
  
  int numBytesRead = 0;
  uint8_t k = buffer[0];
  numBytesRead = numBytesRead + 1;
  
  kind = k;
  if( k == static_cast<uint8_t>(TcpOptionKind::end)){
      hasLength = 0;
      size = calcSize();
      return numBytesRemaining; //even if dataoffset claims there are more bytes, end means that reading needs to stop.
  }

  if(k == static_cast<uint8_t>(TcpOptionKind::noOp) || numBytesRemaining < 2){
      hasLength = 0;
      size = calcSize();
      return numBytesRead;
  }
  
  uint8_t len = buffer[1];
  numBytesRead = numBytesRead + 1;
  length = len;
  hasLength = 1;
  
  uint8_t dataLength = length -2; // to account for length and type field
  
  if(dataLength > (numBytesRemaining-2)) return -1;
  
  for(uint8_t i = 0; i < dataLength; i++){
    data.push_back(buffer[2 + i]);
  }
  numBytesRead = numBytesRead + dataLength;
  size = calcSize();
  return numBytesRead;
  
}


IpOption::IpOption(uint8_t t, uint8_t len, uint8_t hasLen): type(t), length(len), hasLength(hasLen){};

void IpOption::print(){

  cout << "==IpOption==" << endl;
  cout << "type: " << static_cast<unsigned int>(type) << endl;
  cout << "length: " << static_cast<unsigned int>(length) << endl;
  cout << "hasLength: " << static_cast<unsigned int>(hasLength) << endl;
  cout << "data: [";
  for(int i = 0; i < data.size(); i++) cout << " " << static_cast<unsigned int>(data[i]);
  cout << "]" << endl;
  cout << "==========" << endl;


}

void IpOption::toBuffer(vector<uint8_t>& buff){
  buff.push_back(type);
  if(hasLength){
	  buff.push_back(length);
  }

  for(size_t i = 0; i < data.size(); i++){
	  buff.push_back(data[i]);
  }
  
}

//returns number of bytes read, -1 if error.
//numBytesRemaining val is assumed to be greater than 0.
int IpOption::fromBuffer(uint8_t* buffer, int numBytesRemaining){
  
  int numBytesRead = 0;
  uint8_t t = buffer[0];
  numBytesRead = numBytesRead + 1;
  
  type = t;
  if( t == static_cast<uint8_t>(IpOptionType::eool)){
      hasLength = 0;
      return numBytesRemaining; //even if ihl claims there are more bytes, eool means that reading needs to stop.
  }

  if(t == static_cast<uint8_t>(IpOptionType::nop) || numBytesRemaining < 2){
      hasLength = 0;
      return numBytesRead;
  }
  
  uint8_t len = buffer[1];
  numBytesRead = numBytesRead + 1;
  length = len;
  hasLength = 1;
  
  uint8_t dataLength = length -2; // to account for length and type field
  
  if(dataLength > (numBytesRemaining-2)) return -1;
  
  for(uint8_t i = 0; i < dataLength; i++){
    data.push_back(buffer[2 + i]);
  }
  numBytesRead = numBytesRead + dataLength;
  return numBytesRead;
  
}


void onesCompAdd(uint16_t& num1, uint16_t num2){

  uint32_t res = num1 + num2;
  if(res > 0xffff){
  
    res = (res & 0xffff) + 1;
  }
  num1 = static_cast<uint16_t>(res);
}

uint16_t TcpPacket::calcSize(){
  
  uint16_t sz = 20; //standard header
  for(size_t i = 0; i < optionList.size(); i++){
    sz = sz + optionList[i].getSize();
  }
  sz = sz + payload.size();
  return sz;
}

TcpPacket& TcpPacket::setRealChecksum(uint32_t sourceAddress, uint32_t destAddress){
  
  uint16_t accum = 0;
  onesCompAdd(accum, (sourceAddress & 0x0000FFFF));
  onesCompAdd(accum, (sourceAddress & 0xFFFF0000) >> 16);
  onesCompAdd(accum, (destAddress & 0x0000FFFF));
  onesCompAdd(accum, (destAddress & 0xFFFF0000) >> 16); 
  onesCompAdd(accum, 0x0006);
  
  onesCompAdd(accum,size);
  onesCompAdd(accum,sourcePort);
  onesCompAdd(accum,destPort);
  onesCompAdd(accum, seqNum & 0x0000FFFF);
  onesCompAdd(accum, (seqNum & 0xFFFF0000) >> 16);
  
  onesCompAdd(accum, ackNum & 0x0000FFFF);
  onesCompAdd(accum, (ackNum & 0xFFFF0000) >> 16);
  
  onesCompAdd(accum, (dataOffReserved << 8) | flags);
  onesCompAdd(accum, window);
  onesCompAdd(accum, 0x0000);// checksum is replaced by zeros
  onesCompAdd(accum, urgPointer);
  
  vector<uint8_t> optionsAndPayload;
  for(size_t i = 0; i < optionList.size(); i++){
    optionList[i].toBuffer(optionsAndPayload);
  }
  for(size_t i = 0; i < payload.size(); i++){
    optionsAndPayload.push_back(payload[i]);
  }
  if(optionsAndPayload.size() & 0x1){
    optionsAndPayload.push_back(0x00);
  }
  
  for(size_t i = 0; i < optionsAndPayload.size(); i+=2){
    uint8_t firstByte = optionsAndPayload[i];
    uint8_t secondByte = optionsAndPayload[i+1];
    uint16_t word = (firstByte << 8) | secondByte;
    onesCompAdd(accum,word);
  }
  
  checksum = ~accum;
  return *this;
}



TcpPacket& TcpPacket::setFlags(uint8_t cwr, uint8_t ece, uint8_t urg, uint8_t ack, uint8_t psh, uint8_t rst, uint8_t syn, uint8_t fin){
  
  uint8_t byte = 0;
  byte = byte | ((cwr & 0x1) << static_cast<int>(TcpPacketFlags::cwr));
  byte = byte | ((ece & 0x1) << static_cast<int>(TcpPacketFlags::ece));
  byte = byte | ((urg & 0x1) << static_cast<int>(TcpPacketFlags::urg));
  byte = byte | ((ack & 0x1) << static_cast<int>(TcpPacketFlags::ack));
  byte = byte | ((psh & 0x1) << static_cast<int>(TcpPacketFlags::psh));
  byte = byte | ((rst & 0x1) << static_cast<int>(TcpPacketFlags::rst));
  byte = byte | ((syn & 0x1) << static_cast<int>(TcpPacketFlags::syn));
  byte = byte | ((fin & 0x1) << static_cast<int>(TcpPacketFlags::fin));

  flags = byte;
  return *this;
}

TcpPacketFlags& operator++(TcpPacketFlags& p, int i){
	
  switch(p){	
	  case TcpPacketFlags::cwr:
		  p = TcpPacketFlags::ece;
		  break;
	  case TcpPacketFlags::ece:
		  p = TcpPacketFlags::urg;
		  break;
	  case TcpPacketFlags::urg:
		  p = TcpPacketFlags::ack;
		  break;
	  case TcpPacketFlags::ack:
		  p = TcpPacketFlags::psh;
		  break;
	  case TcpPacketFlags::psh:
		  p = TcpPacketFlags::rst;
		  break;
	  case TcpPacketFlags::rst:
		  p = TcpPacketFlags::syn;
		  break;
	  case TcpPacketFlags::syn:
		  p = TcpPacketFlags::fin;
		  break;
	  case TcpPacketFlags::fin:
		  p = TcpPacketFlags::none;
		  break;
	  case TcpPacketFlags::none:
		  p = TcpPacketFlags::none;
		  break;	

  }
  return p;

}

uint8_t TcpPacket::getFlag(TcpPacketFlags flag){

	if (flag == TcpPacketFlags::none) return 0;
	
	return (flags >> static_cast<int>(flag)) & 0x1;
}

uint8_t TcpPacket::getDataOffset(){

	return (dataOffReserved & 0xf0) >> 4;
}
uint8_t TcpPacket::getReserved(){

        return (dataOffReserved & 0xf);
}
uint16_t TcpPacket::getDestPort(){return destPort;}
uint16_t TcpPacket::getSrcPort(){return sourcePort;}
uint32_t TcpPacket::getSeqNum(){return seqNum;}
uint32_t TcpPacket::getAckNum(){return ackNum;}
uint16_t TcpPacket::getWindow(){return window;}

void TcpPacket::print(){

	cout << "--------TcpPacket--------" << endl;
	cout << "sourcePort: " << sourcePort << endl;
	cout << "destPort: " << destPort  << endl;
	cout << "seqNum: " << seqNum  << endl;
	cout << "ackNum: " << ackNum  << endl;
	cout << "dataOffset: " << static_cast<unsigned int>(getDataOffset()) << endl;
	cout << "reserved: " << static_cast<unsigned int>(getReserved())  << endl;
	cout << "+++Flags+++" << endl;
	for(TcpPacketFlags p = TcpPacketFlags::cwr; p != TcpPacketFlags::none; p++) cout << "flag " << static_cast<unsigned int>(p) << ": " << static_cast<unsigned int>(getFlag(p)) << endl;
	cout << "+++++++++++" << endl;
	cout << "window: " << window << endl;
	cout << "checksum: " << checksum  << endl;
	cout << "urgPointer: " << urgPointer << endl;
	cout << "TcpOptionList: " << endl;
	for(size_t i = 0; i < optionList.size(); i++) optionList[i].print();
	cout << "payload: [" << endl;
	for(size_t i = 0; i < payload.size(); i++) cout << static_cast<unsigned int>(payload[i]) << " ";
	cout << " ]" << endl;
	cout << "----------------------" << endl;

}

uint32_t TcpPacket::getSegSize(){
  return payload.size() + getFlag(TcpPacketFlags::syn) + getFlag(TcpPacketFlags::fin);
}

TcpPacket& TcpPacket::setSrcPort(uint16_t source){
  sourcePort = source;
  return *this;
}
TcpPacket& TcpPacket::setDestPort(uint16_t dest){
  destPort = dest;
  return *this;
}
TcpPacket& TcpPacket::setSeq(uint32_t seq){
  seqNum = seq;
  return *this;
}
TcpPacket& TcpPacket::setAck(uint32_t ack){
  ackNum = ack;
  return *this;
}
TcpPacket& TcpPacket::setDataOffset(uint8_t dataOffset){
  dataOffReserved = (dataOffReserved & 0x0f) | ((dataOffset & 0xf) << 4);
  return *this;
}
TcpPacket& TcpPacket::setReserved(uint8_t reserved){
  dataOffReserved = (dataOffReserved & 0xf0) | (reserved & 0xf);
  return *this;
}
TcpPacket& TcpPacket::setWindow(uint16_t win){
  window = win;
  return *this;
}
TcpPacket& TcpPacket::setChecksum(uint16_t check){
  checksum = check;
  return *this;
}
TcpPacket& TcpPacket::setUrgentPointer(uint16_t urg){
  urgPointer = urg;
  return *this;
} 
TcpPacket& TcpPacket::setOptions(vector<TcpOption> list){
  optionList = list;
  size = calcSize();
  return *this;
}
TcpPacket& TcpPacket::setPayload(vector<uint8_t> data){
  payload = data;
  size = calcSize();
  return *this;
}

void TcpPacket::toBuffer(vector<uint8_t>& buff){

        loadBytes<uint16_t>(toAltOrder<uint16_t>(sourcePort), buff);
	loadBytes<uint16_t>(toAltOrder<uint16_t>(destPort), buff);
        loadBytes<uint32_t>(toAltOrder<uint32_t>(seqNum), buff);  
        loadBytes<uint32_t>(toAltOrder<uint32_t>(ackNum), buff);

	buff.push_back(dataOffReserved);
	buff.push_back(flags);
	
	loadBytes<uint16_t>(toAltOrder<uint16_t>(window), buff);
	loadBytes<uint16_t>(toAltOrder<uint16_t>(checksum), buff);
	loadBytes<uint16_t>(toAltOrder<uint16_t>(urgPointer), buff);
	
	for(size_t i = 0; i < optionList.size(); i++) optionList[i].toBuffer(buff);
	for(size_t i = 0; i < payload.size(); i++) buff.push_back(payload[i]);
}

int TcpPacket::fromBuffer(uint8_t* buffer, int numBytes){
  
  if(numBytes < tcpMinHeaderLen){
    return -1;
  }

  sourcePort = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,0));
  destPort = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,2));
  seqNum = toAltOrder<uint32_t>(unloadBytes<uint32_t>(buffer,4));
  ackNum = toAltOrder<uint32_t>(unloadBytes<uint32_t>(buffer,8));
  dataOffReserved = buffer[12];
  flags = buffer[13];
  window = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,14));
  checksum = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,16));
  urgPointer = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,18));

  uint8_t offsetConv = getDataOffset() * 4;
  if(offsetConv < 20 || offsetConv > numBytes) return -1;
  
  uint8_t* currPointer = buffer + tcpMinHeaderLen;
  
  vector<TcpOption> options;
  
  if(offsetConv > 20){
    int optionBytesRemaining = offsetConv - tcpMinHeaderLen;
    while(optionBytesRemaining > 0){
        TcpOption o;
        int numBytesRead = o.fromBuffer(currPointer, optionBytesRemaining);
        if(numBytesRead == -1) return -1;
        currPointer = currPointer + numBytesRead;
        optionBytesRemaining = optionBytesRemaining - numBytesRead;
        options.push_back(o);
    }
    optionList = options;
  }
  
  int dataBytesRemaining = numBytes - offsetConv;
  for(int i =0; i < dataBytesRemaining; i++){
    payload.push_back(currPointer[i]);
  }
  
  size = calcSize();

  return 0;
}


IpPacket& IpPacket::setVersion(uint8_t vers){
  versionIHL = (versionIHL & 0x0F) | ((vers & 0xf) << 4);
  return *this;
}

IpPacket& IpPacket::setIHL(uint8_t ihl){
  versionIHL = (versionIHL & 0xF0) | (ihl & 0xf);
  return *this;
}

IpPacket& IpPacket::setDSCP(uint8_t dscp){
  dscpEcn = (dscpEcn & 0x03) | ((dscp & 0x3f) << 2);
  return *this;
}

IpPacket& IpPacket::setEcn(uint8_t ecn){
  dscpEcn = (dscpEcn & 0xFC) | (ecn & 0x3);
  return *this;
}

IpPacket& IpPacket::setTotLen(uint16_t len){
  totalLength = len;
  return *this;
}

IpPacket& IpPacket::setIdent(uint16_t ident){
  identification = ident;
  return *this;
}

IpPacket& IpPacket::setFlags(uint8_t r, uint8_t df, uint8_t mf){
  uint16_t flags = 0;
  flags = flags | ((r & 0x1) << static_cast<int>(IpPacketFlags::reserved));
  flags = flags | ((df & 0x1) << static_cast<int>(IpPacketFlags::dontFrag));
  flags = flags | ((mf & 0x1) << static_cast<int>(IpPacketFlags::moreFrag));
  
  flagsFragOffset = (flagsFragOffset & 0x01FFF) | (flags << (16-numIpPacketFlags));
  return *this;
}

IpPacket& IpPacket::setFragOff(uint16_t frag){
  flagsFragOffset = (flagsFragOffset & 0xE000) | (frag & 0x01FFF);
  return *this;
}

IpPacket& IpPacket::setTtl(uint8_t ttl){
  this->ttl = ttl;
  return *this;
}

IpPacket& IpPacket::setProto(uint8_t proto){
  protocol = proto;
  return *this;
}

IpPacket& IpPacket::setHeadCheck(uint16_t check){
  headerChecksum = check;
  return *this;
}

IpPacket& IpPacket::setSrcAddr(uint32_t addr){
  sourceAddress = addr;
  return *this;
}

IpPacket& IpPacket::setDestAddr(uint32_t addr){
  destAddress = addr;
  return *this;
}

IpPacket& IpPacket::setOptions(std::vector<IpOption> list){
  optionList = list;
  return *this;
}

IpPacket& IpPacket::setTcpPacket(TcpPacket& packet){
  tcpPacket = packet;
  return *this;
}


uint32_t IpPacket::getSrcAddr(){ return sourceAddress;}
uint32_t IpPacket::getDestAddr(){ return destAddress;}

uint8_t IpPacket::getVersion(){
  return (versionIHL & 0xF0) >> 4;
}
uint8_t IpPacket::getIHL(){
  return (versionIHL & 0x0F);
}
uint8_t IpPacket::getDscp(){
  return (dscpEcn & 0xFC) >> 2;
}
uint8_t IpPacket::getEcn(){
  return (dscpEcn & 0x3);
}
uint8_t IpPacket::getFlag(IpPacketFlags flag){
  if (flag == IpPacketFlags::none) return 0;
  uint8_t flags = ((flagsFragOffset & 0xE000) >> (16 - numIpPacketFlags)) & 0xFF;
  return (flags >> static_cast<int>(flag)) & 0x1;
}
uint16_t IpPacket::getFragOffset(){
  return (flagsFragOffset & 0x01FFF);
}

IpPacketFlags& operator++(IpPacketFlags& p, int i){
	
  switch(p){	
	  case IpPacketFlags::moreFrag:
		  p = IpPacketFlags::dontFrag;
		  break;
	  case IpPacketFlags::dontFrag:
		  p = IpPacketFlags::reserved;
		  break;
	  case IpPacketFlags::reserved:
		  p = IpPacketFlags::none;
		  break;
	  case IpPacketFlags::none:
		  p = IpPacketFlags::none;
		  break;
  }
  return p;

}

void IpPacket::print(){

	cout << "++++++++IpPacket++++++++" << endl;
	cout << "version: " << static_cast<unsigned int>(getVersion()) << endl;
	cout << "ihl: " << static_cast<unsigned int>(getIHL())  << endl;
	cout << "dscp: " << static_cast<unsigned int>(getDscp())   << endl;
	cout << "ecn: " <<  static_cast<unsigned int>(getEcn()) << endl;
	cout << "total length: " << totalLength << endl;
	cout << "identification: " << identification  << endl;
	cout << "///Flags///" << endl;
	for(IpPacketFlags p = IpPacketFlags::moreFrag; p != IpPacketFlags::none; p++) cout << "flag " << static_cast<unsigned int>(p) << ": " << static_cast<unsigned int>(getFlag(p)) << endl;
	cout << "///////////" << endl;
	cout << "fragment offset: " << getFragOffset() << endl;
	cout << "ttl: " << static_cast<unsigned int>(ttl)  << endl;
	cout << "protocol: " << static_cast<unsigned int>(protocol) << endl;
	cout << "header checksum: " << headerChecksum << endl;
	cout << "source address: " << sourceAddress << endl;
	cout << "dest address: " << destAddress << endl;
	cout << "IpOptionList: " << endl;
	for(size_t i = 0; i < optionList.size(); i++) optionList[i].print();
	tcpPacket.print();
	cout << "++++++++++++++++++++++++" << endl;

}

void IpPacket::toBuffer(vector<uint8_t>& buff){

	buff.push_back(versionIHL);
	buff.push_back(dscpEcn);
	
	loadBytes<uint16_t>(toAltOrder<uint16_t>(totalLength), buff); 
	loadBytes<uint16_t>(toAltOrder<uint16_t>(identification), buff);
	loadBytes<uint16_t>(toAltOrder<uint16_t>(flagsFragOffset), buff);
	
	buff.push_back(ttl);
	buff.push_back(protocol);
	
	loadBytes<uint16_t>(toAltOrder<uint16_t>(headerChecksum), buff);
	loadBytes<uint32_t>(toAltOrder<uint32_t>(sourceAddress), buff);
        loadBytes<uint32_t>(toAltOrder<uint32_t>(destAddress), buff);
		
	for(size_t i = 0; i < optionList.size(); i++) optionList[i].toBuffer(buff);	
        tcpPacket.toBuffer(buff);
}

TcpPacket& IpPacket::getTcpPacket(){ return tcpPacket;}

int IpPacket::fromBuffer(uint8_t* buffer, int numBytes){
  
  if(numBytes < ipMinHeaderLen){
    return -1;
  }
  
  versionIHL = buffer[0];
  dscpEcn = buffer[1];
  totalLength = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,2));
  identification = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,4));
  flagsFragOffset = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,6));
  ttl = buffer[8];
  protocol = buffer[9];
  headerChecksum = toAltOrder<uint16_t>(unloadBytes<uint16_t>(buffer,10));
  sourceAddress = toAltOrder<uint32_t>(unloadBytes<uint32_t>(buffer,12));
  destAddress = toAltOrder<uint32_t>(unloadBytes<uint32_t>(buffer,16));
  
  uint8_t ihlConv = getIHL() * 4;
  if(ihlConv < 20 || ihlConv > numBytes) return -1;
  
  uint8_t* currPointer = buffer + ipMinHeaderLen;
  
  vector<IpOption> options;
  
  if(ihlConv > 20){
    int optionBytesRemaining = ihlConv - ipMinHeaderLen;
    while(optionBytesRemaining > 0){
        IpOption o;
        int numBytesRead = o.fromBuffer(currPointer, optionBytesRemaining);
        if(numBytesRead == -1) return -1;
        currPointer = currPointer + numBytesRead;
        optionBytesRemaining = optionBytesRemaining - numBytesRead;
        options.push_back(o);
    }
    optionList = options;
  }
  
  int tcpBytesRemaining = numBytes - ihlConv;
  if(tcpPacket.fromBuffer(currPointer, tcpBytesRemaining) == -1) return -1;
  else return 0;
  
}
