#include <iostream>
#include <string>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/stats-module.h"

using namespace ns3;
using namespace std;


#define DEFAULT_RETRY 0
#define DROP_AND_REBUFFER_TAIL 1
#define DROP_AND_REBUFFER_HEAD 2

#define DEFAULT_FIFO 0
#define ROUND_ROBIN_DOWN 1
#define ROUND_ROBIN_UP   2
#define ROUND_ROBIN_BOTH 3
#define ROUND_ROBIN_UP_DEBUG 4


#define UDP_CBR 0
#define TCP_RENO_BULK 1

NS_LOG_COMPONENT_DEFINE ("WiFiDistanceExperiment");
double firstRxTime = -1.0, lastRxTime;
uint32_t bytesTotal = 0;

void SinkRxTrace(Ptr<const Packet> pkt, const Address &addr)
{
if (firstRxTime < 0)
firstRxTime = Simulator::Now().GetSeconds();
lastRxTime = Simulator::Now().GetSeconds();
bytesTotal += pkt->GetSize();
}

void TxCallback (Ptr<CounterCalculator<uint32_t> > datac,std::string path,Ptr<const Packet> packet) {

  NS_LOG_INFO ("Sent frame counted in " << datac->GetKey ());
  datac->Update ();
	
  // end TxCallback
}

int main (int argc, char *argv[]){



		int t = 0;
		int count = 1;
		DataRate linkRate("100Mbps");

		//string phyMode = "OfdmRate6Mbps";

		// experiment variables
		int nCsma = 4;
		int nWifi = 4;
		float base_dist = 20.0;  // 20.0
		float u_dist = 25.0;  //15.0
		int retryLimit = 1;   //7

		// Drop&Rebuffer Mode ON
		int dropRebufferMode = DEFAULT_RETRY;


		// Queue Plicy : RR ON?
		int rrQueueMode = DEFAULT_FIFO;


		// Transport / App 
		string tp="UDP";

		// Wire Link delay  
		//int wireLinkDealy = 10;  // 10
		int queueSizePerNode = 10000000;
		float delayPerNode = 3.0;
		float simulationTime = 40.0;  // 10

		// traffic parameters
  		uint32_t MaxPacketSize = 1024;
  		uint32_t interPacketInterval = 3000;  // 3000 in MicroSeconds
  		uint32_t maxPacketCount = 10000000;
//sang add
		//rateAdaptation
		string rateAdaptation = "Arf";

		CommandLine cmd;
		cmd.AddValue("drop","DropRebuffer - ",dropRebufferMode);
		cmd.AddValue("retry","Retry Limit ",retryLimit);
		cmd.AddValue("rrq","RRQueue Mode",rrQueueMode);
		cmd.AddValue("tp","Trans APP Type",tp);
		cmd.AddValue("interval","Trans APP Type",interPacketInterval);
		cmd.AddValue("simul","simulation Time : 10s(default)",simulationTime);
		cmd.AddValue("rate","rate Adaptation",rateAdaptation);
		cmd.AddValue("count","experiment count",count);
		cmd.Parse(argc,argv);


		if ( (rrQueueMode == ROUND_ROBIN_UP_DEBUG) && tp.compare("UDP")==0 )
			simulationTime = 5.0;

		std::cout<<"D&R = " << dropRebufferMode << " Retrylimt = " << retryLimit <<std::endl;
		std::cout<<"RR Queue = " << rrQueueMode <<std::endl;
		std::cout<<"Transport Protocol = " << tp <<std::endl;
		std::cout<<"Simulation Time = " << simulationTime <<std::endl;
		std::cout<<"Packet Interval = " << interPacketInterval <<std::endl;
		std::cout<<"rate Adaptation = ns3::" << rateAdaptation<<"WifiManager" <<std::endl;
		srand((unsigned int)time(NULL));
		ns3::RngSeedManager::SetSeed((unsigned int)time(NULL));

		//rate adaptation
		rateAdaptation="ns3::"+rateAdaptation+"WifiManager";
		
		//Experiment Start
		while(t<count){
			std::cout<<"("<<t+1<<"/"<<count<<")th Experiment"<<std::endl;

			//CSMA node(wired Node create)
			NodeContainer p2pNodes;
			p2pNodes.Create(2);

			PointToPointHelper pointToPoint;
  			pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
			pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (5)));
			
			NetDeviceContainer p2pDevices;
  			p2pDevices = pointToPoint.Install (p2pNodes);

			NodeContainer csmaNodes;
			csmaNodes.Add(p2pNodes.Get(1));
			csmaNodes.Create(nCsma);			

			CsmaHelper csma;
			csma.SetChannelAttribute("DataRate", DataRateValue(linkRate));
			csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds (6560)));

			NetDeviceContainer csmaDevices;
			csmaDevices = csma.Install(csmaNodes);

			NodeContainer wifiStaNodes;
			wifiStaNodes.Create(nWifi);
			NodeContainer wifiApNode = p2pNodes.Get(0);
			
			// set Retry Limit
			Config::SetDefault("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue(retryLimit));
			Config::SetDefault("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue(nWifi*queueSizePerNode));
			Config::SetDefault("ns3::WifiMacQueue::MaxDelay", TimeValue(Seconds ((nWifi*delayPerNode))));
			
			// set Drop and Rebuffer 
			Config::SetDefault("ns3::WifiRemoteStationManager::DropAndRebuffer", UintegerValue(dropRebufferMode));
			
			// set Queue Policy 
			Config::SetDefault("ns3::WifiRemoteStationManager::RRQueue", UintegerValue(rrQueueMode));


			YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  			YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  			phy.SetChannel (channel.Create ());
				
			
			WifiHelper wifi = WifiHelper::Default();

  			wifi.SetRemoteStationManager(rateAdaptation);
			//wifi.SetRemoteStationManager(rateAdaptation,"DataMode",StringValue(phyMode),"ControlMode",StringValue(phyMode));


			NqosWifiMacHelper mac = NqosWifiMacHelper::Default();
  			Ssid ssid = Ssid ("ns-3-ssid");
  			mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));

			NetDeviceContainer staDevices;
			staDevices = wifi.Install (phy, mac, wifiStaNodes);

			mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));

			NetDeviceContainer apDevices;
			apDevices = wifi.Install (phy, mac, wifiApNode);
			
			Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSinkHelper/Rx",MakeCallback(&SinkRxTrace));

			MobilityHelper mobility;
			Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
			positionAlloc->Add(Vector(0.0,4.0,0.0));
			positionAlloc->Add(Vector(-5.0,-15.0,0.0));
			positionAlloc->Add(Vector(-5.0,-10.0,0.0));
			positionAlloc->Add(Vector(-5.0,-5.0,0.0));
			positionAlloc->Add(Vector(-5.0,0.0,0.0));
			positionAlloc->Add(Vector(5.0,8.0,0.0));//5.0,8.0
			
			positionAlloc->Add(Vector(base_dist,8.0,0.0));  // for the best case
			positionAlloc->Add(Vector(base_dist+2*u_dist,13.0,0.0));
			positionAlloc->Add(Vector(base_dist+3*u_dist,18.0,0.0));
			positionAlloc->Add(Vector(base_dist+4*u_dist,23.0,0.0));
			mobility.SetPositionAllocator(positionAlloc);
			mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");//Static position
			mobility.Install(csmaNodes);
			mobility.Install (wifiApNode);
			mobility.Install(wifiStaNodes);

			InternetStackHelper stack;
 			stack.Install (csmaNodes);
 			stack.Install (wifiApNode);
 			stack.Install (wifiStaNodes);

			Ipv4AddressHelper address;
			address.SetBase ("10.1.1.0", "255.255.255.0");
  			Ipv4InterfaceContainer p2pInterfaces;
 			p2pInterfaces = address.Assign (p2pDevices);

			address.SetBase ("10.1.2.0", "255.255.255.0");
			Ipv4InterfaceContainer csmaInterfaces;
			csmaInterfaces = address.Assign (csmaDevices);

			address.SetBase ("10.1.3.0", "255.255.255.0");
			Ipv4InterfaceContainer wifiInterfaces;
			wifiInterfaces = address.Assign (apDevices);
			wifiInterfaces.Add(address.Assign (staDevices));

			Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
			
			std::cout<<"Node Config is done"  <<std::endl;

			uint16_t port = 1010;


			// in MicroSeconds
			double start_time = 10.0 * 1000000 + static_cast<double> (rand()%100) ;
			double time[nCsma];
			
			ApplicationContainer sourceApps[nCsma] ;
			ApplicationContainer sinkAppA[nWifi] ;

		 	if(tp.compare("UDP")==0) { // UDP / CBR
				std::cout<<"UDP/CBR Traffic  " <<std::endl;
				std::cout<<"Max Packet Count = " << maxPacketCount << "  Max Packet Size  = " << MaxPacketSize <<std::endl;

				UdpClientHelper * sources[nCsma] ;
		
				for(int k = 0 ; k < nCsma ; k++){
				//for(int k = nNode-1 ; k >= 0 ; k--){
 					sources[k] = new UdpClientHelper (wifiInterfaces.GetAddress(k+1), port);
  					(*sources[k]).SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  					(*sources[k]).SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
  					(*sources[k]).SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
					sourceApps[k] = (*sources[k]).Install (csmaNodes.Get(k+1));
				}

					
				//
				LogComponentEnable("UdpSocket", LOG_LEVEL_INFO);

	
				for(int k = 0 ; k < nWifi ; k++) {
			  		Address sinkAaddr(InetSocketAddress (wifiInterfaces.GetAddress(k+1), port));
			  		PacketSinkHelper sinkA ("ns3::UdpSocketFactory", sinkAaddr);
			   		sinkAppA[k] = sinkA.Install (wifiStaNodes.Get(k));
			   		sinkAppA[k].Start (Seconds (0.0));
			   		sinkAppA[k].Stop (Seconds (60.0));
				}
			
				std::cout<<"Sink App is created."  <<std::endl;

		   	}
			else { // TCP Reno(?) / Bulk
			
				std::cout<<"TCP/Bulk Traffic  " <<std::endl;
				BulkSendHelper * sources[nCsma] ;

				if(tp.compare("Vegas")==0){
					Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpVegas"));
					cout<<"TCP version: "<< tp<<endl;
				}
				else if(tp.compare("Westwood")==0){
					Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpWestwood"));
					cout<<"Default TCP version: "<< tp<<endl;
				}
				else if(tp.compare("NewReno")==0){
					Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpNewReno"));
					cout<<"Default TCP version: "<< tp<<endl;
				}
				else{
					cout<<"Default TCP version: input is "<< tp<<endl;
				}


				for(int k = 0 ; k < nCsma ; k++){
				//for(int k = nNode-1 ; k >= 0 ; k--){
 					sources[k] = new BulkSendHelper ("ns3::TcpSocketFactory",InetSocketAddress(wifiInterfaces.GetAddress(k+1), port));
  					(*sources[k]).SetAttribute ("MaxBytes", UintegerValue (0));
					(*sources[k]).SetAttribute ("SendSize", UintegerValue (1024));
					sourceApps[k] = (*sources[k]).Install (csmaNodes.Get(k+1));
				}

					
				//
				LogComponentEnable("TcpSocket", LOG_LEVEL_INFO);

	
				for(int k = 0 ; k < nWifi ; k++) {
			  		Address sinkAaddr(InetSocketAddress (wifiInterfaces.GetAddress(k+1), port));
			  		PacketSinkHelper sinkA ("ns3::TcpSocketFactory", sinkAaddr);
			   		sinkAppA[k] = sinkA.Install (wifiStaNodes.Get(k));
			   		sinkAppA[k].Start (Seconds (0.0));
			   		sinkAppA[k].Stop (Seconds (60.0));
				}
			
				std::cout<<"Sink App is created."  <<std::endl;
			 }

			// in MicroSeconds
			//for(int k = nCsma ; k >= 0 ; k--){
			for(int k = 0 ; k < nCsma ; k++){
			   //if ( (k == t) || (t>3) ) { 
			   if ( t>=0 ) { 
				int rv = rand()%(interPacketInterval/10);
				//int rv = 1001;
				time[k] = start_time + static_cast<double> (rv) ;
				sourceApps[k].Start (MicroSeconds (time[k]));
				sourceApps[k].Stop (MicroSeconds(time[k]+simulationTime*1000000));
				cout<<"Node " << k << " starts at " << rv <<endl;
			   } else {
				sourceApps[k].Stop (MicroSeconds(1));
				cout<<"Node " << k << " stops at 0" <<endl;
			   }
			}

				string filename;
			string nodename;	
			stringstream ss;
			stringstream ss2;
			stringstream ss3;
			ss<<t+1;
			filename = ss.str();
			DataCollector data;
			data.DescribeRun(filename,filename,filename,filename);
			filename += "csma-03.21.xml";


						
			Ptr<PacketCounterCalculator> totalRx1 = CreateObject<PacketCounterCalculator>();// #6
			Ptr<PacketCounterCalculator> totalRx2 = CreateObject<PacketCounterCalculator>();// #7
			Ptr<PacketCounterCalculator> totalRx3 = CreateObject<PacketCounterCalculator>();// #8
			Ptr<PacketCounterCalculator> totalRx4 = CreateObject<PacketCounterCalculator>();// #9
	 		totalRx1->SetKey ("wifi-Rx-frames");
 			totalRx2->SetKey ("wifi-Rx-frames");
			totalRx3->SetKey ("wifi-Rx-frames");
			totalRx4->SetKey ("wifi-Rx-frames");
		

			Ptr<CounterCalculator<uint32_t> > totalTx1 = CreateObject<CounterCalculator<uint32_t> >();// #2
			Ptr<CounterCalculator<uint32_t> > totalTx2 = CreateObject<CounterCalculator<uint32_t> >();// #3
			Ptr<CounterCalculator<uint32_t> > totalTx3 = CreateObject<CounterCalculator<uint32_t> >();// #4
			Ptr<CounterCalculator<uint32_t> > totalTx4 = CreateObject<CounterCalculator<uint32_t> >();// #5
			totalTx1->SetKey ("csma-Tx-frames");
			totalTx2->SetKey ("csma-Tx-frames");
			totalTx3->SetKey ("csma-Tx-frames");
			totalTx4->SetKey ("csma-Tx-frames");


			Ptr<PacketCounterCalculator> totalRx5 = CreateObject<PacketCounterCalculator>();//csma Aggregate receiver - #1
			Ptr<PacketCounterCalculator> totalRx6 = CreateObject<PacketCounterCalculator>();//p2p receiver - #0
			Config::Connect ("/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx6));
			Config::Connect ("/NodeList/1/DeviceList/*/$ns3::CsmaNetDevice/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx5));
	
			Ptr<CounterCalculator<uint32_t> > totalTx5 = CreateObject<CounterCalculator<uint32_t> >();//p2p sender - #1
			Ptr<CounterCalculator<uint32_t> > totalTx6 = CreateObject<CounterCalculator<uint32_t> >();//AP sender - #0
			Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",MakeBoundCallback (&TxCallback,totalTx6));
			Config::Connect ("/NodeList/1/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx5));
			totalTx5->SetContext ("node[1] Send");
			totalRx5->SetContext ("node[1] Recevied");
			
			totalTx6->SetContext ("node[0] Send");
			totalRx6->SetContext ("node[0] Recevied");

			totalRx5->SetKey("csma-Rx-frames");
			totalRx6->SetKey("p2p-Rx-frames");
			totalTx5->SetKey("p2p-Tx-frames");
			totalTx6->SetKey("Ap-Tx-frames");
			
			
			totalTx1->SetContext ("node[2]");
			Config::Connect ("/NodeList/2/DeviceList/*/$ns3::CsmaNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx1));
			
			
			totalTx2->SetContext ("node[3]");
			Config::Connect ("/NodeList/3/DeviceList/*/$ns3::CsmaNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx2));
			
			
			totalTx3->SetContext ("node[4]");
			Config::Connect ("/NodeList/4/DeviceList/*/$ns3::CsmaNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx3));

			
			totalTx4->SetContext ("node[5]");
			Config::Connect ("/NodeList/5/DeviceList/*/$ns3::CsmaNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx4));
		

			totalRx1->SetContext("node[6]");
			Config::Connect ("/NodeList/6/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx1));
			
			totalRx2->SetContext("node[7]");
 			Config::Connect ("/NodeList/7/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx2));

			totalRx3->SetContext("node[8]");
 			Config::Connect ("/NodeList/8/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx3));

			totalRx4->SetContext("node[9]");
			Config::Connect ("/NodeList/9/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx4));

				
			data.AddDataCalculator (totalTx1);
			data.AddDataCalculator (totalTx2);
			data.AddDataCalculator (totalTx3);
			data.AddDataCalculator (totalTx4);

			data.AddDataCalculator (totalRx5);		
			data.AddDataCalculator (totalTx5);

			data.AddDataCalculator (totalRx6);		
			data.AddDataCalculator (totalTx6);

			data.AddDataCalculator (totalRx1);
			data.AddDataCalculator (totalRx2);
			data.AddDataCalculator (totalRx3);
			data.AddDataCalculator (totalRx4);
		
		
			//NetAnimation Setting
			AnimationInterface anim(filename);
			anim.SetMaxPktsPerTraceFile(5000000);

		
			
			//Create Trace File
			AsciiTraceHelper ascii;
			filename = ss.str();
			filename += "csma-03.21";
			filename += "-r7.tr";
			Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (filename);
  			phy.EnableAsciiAll (stream);
 			csma.EnableAsciiAll (stream);
 			stack.EnableAsciiIpv4All (stream);

			// Run the simulation
			Simulator::Stop (Seconds (60.0));
			Simulator::Run ();
			std::cout << std::endl;

			Ptr<PacketSink> sinks[nWifi];
			double throughput[nWifi];
			int totalRx = 0;
			float rate[4]={0,};

			for(int k= 0 ; k < nWifi ; k++){
				sinks[k] = StaticCast<PacketSink> (sinkAppA[k].Get(0));

				rate[k] = sinks[k]->GetTotalRx();
				totalRx += rate[k];
				std::cout << "Node " << k << ": End-Point Rx Bytes : " << sinks[k]->GetTotalRx()  << "\n";
			}
			for(int k= 0 ; k < nWifi ; k++){
				throughput[k]  = ((sinks[k]->GetTotalRx()*8)/(1e6 * simulationTime));
				std::cout << (k+1)<<"node's Throughput : " << throughput[k] << "Mbps\n";					
								
			}

			float fi = ((rate[0]+rate[1]+rate[2]+rate[3])*(rate[0]+rate[1]+rate[2]+rate[3])) / (4 * (rate[0]*rate[0]+rate[1]*rate[1]+rate[2]*rate[2]+rate[3]*rate[3]));

			std::cout << std::endl;
			std::cout << "Total Received Bytes : " << totalRx <<std::endl;
			std::cout << "Fairness Index : " << fi <<std::endl;
			std::cout << std::endl;
	
			Ptr<DataOutputInterface> output = 0;
			output = CreateObject<SqliteDataOutput>();
			  if (output != 0)
			    output->Output (data);

			Simulator::Destroy ();
			t++;
			std::cout << std::endl<<std::endl;


		}
}






