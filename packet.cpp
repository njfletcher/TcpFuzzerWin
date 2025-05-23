#include "packet.h"
#include <iostream>

using namespace std;


TcpOption::TcpOption(TcpOptionKind k, uint8_t len, uint8_t hasLen): kind(k), length(len), hasLength(hasLen){};

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
  buff.push_back(static_cast<uint8_t>(kind));
  if(hasLength){
	  buff.push_back(length);
  }

  for(size_t i = 0; i < data.size(); i++){
	  buff.push_back(data[i]);
  }
}


IpOption::IpOption(IpOptionType k, uint8_t len, uint8_t hasLen): type(k), length(len), hasLength(hasLen){};

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
  buff.push_back(static_cast<uint8_t>(type));
  if(hasLength){
	  buff.push_back(length);
  }

  for(size_t i = 0; i < data.size(); i++){
	  buff.push_back(data[i]);
  }
  
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

uint16_t TcpPacket::getDestPort(){

	return destPort;
}

uint16_t TcpPacket::getSrcPort(){
	
	return sourcePort;
}

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
  return *this;
}
TcpPacket& TcpPacket::setPayload(vector<uint8_t> data){
  payload = data;
  return *this;
}

void TcpPacket::toBuffer(vector<uint8_t>& buff){
	buff.push_back(((sourcePort & 0xFF00) >> 8) & 0xFF);
	buff.push_back(sourcePort & 0x00FF);
	
	buff.push_back(((destPort & 0xFF00) >> 8) & 0xFF);
	buff.push_back(destPort & 0x00FF);

	buff.push_back(((seqNum & 0xFF000000) >> 24) & 0xFF);
	buff.push_back(((seqNum & 0x00FF0000) >> 16) & 0xFF);
	buff.push_back(((seqNum & 0x0000FF00) >> 8) & 0xFF);
	buff.push_back(seqNum & 0x000000FF);

	buff.push_back(((ackNum & 0xFF000000) >> 24) & 0xFF);
	buff.push_back(((ackNum & 0x00FF0000) >> 16) & 0xFF);
	buff.push_back(((ackNum & 0x0000FF00) >> 8) & 0xFF);
	buff.push_back(ackNum & 0x000000FF);
	
	buff.push_back(dataOffReserved);
	buff.push_back(flags);
	
	buff.push_back(((window & 0xFF00) >> 8) & 0xFF);
	buff.push_back(window & 0x00FF);
	
	buff.push_back(((checksum & 0xFF00) >> 8) & 0xFF);
	buff.push_back(checksum & 0x00FF);
	
	buff.push_back(((urgPointer & 0xFF00) >> 8) & 0xFF);
	buff.push_back(urgPointer & 0x00FF);
	
	for(size_t i = 0; i < optionList.size(); i++) optionList[i].toBuffer(buff);
		
	for(size_t i = 0; i < payload.size(); i++) buff.push_back(payload[i]);
      
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

IpPacket& IpPacket::setDstAddr(uint32_t addr){
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
	
	buff.push_back(((totalLength & 0xFF00) >> 8) & 0xFF);
	buff.push_back(totalLength & 0x00FF);
	
	buff.push_back(((identification & 0xFF00) >> 8) & 0xFF);
	buff.push_back(identification & 0x00FF);
	
	buff.push_back(((flagsFragOffset & 0xFF00) >> 8) & 0xFF);
	buff.push_back(flagsFragOffset & 0x00FF);
	
	buff.push_back(ttl);
	buff.push_back(protocol);
	
	buff.push_back(((headerChecksum & 0xFF00) >> 8) & 0xFF);
	buff.push_back(headerChecksum & 0x00FF);

	buff.push_back(((sourceAddress & 0xFF000000) >> 24) & 0xFF);
	buff.push_back(((sourceAddress & 0x00FF0000) >> 16) & 0xFF);
	buff.push_back(((sourceAddress & 0x0000FF00) >> 8) & 0xFF);
	buff.push_back(sourceAddress & 0x000000FF);

	buff.push_back(((destAddress & 0xFF000000) >> 24) & 0xFF);
	buff.push_back(((destAddress & 0x00FF0000) >> 16) & 0xFF);
	buff.push_back(((destAddress & 0x0000FF00) >> 8) & 0xFF);
	buff.push_back(destAddress & 0x000000FF);
		
	for(size_t i = 0; i < optionList.size(); i++) optionList[i].toBuffer(buff);	
	
        tcpPacket.toBuffer(buff);

}
