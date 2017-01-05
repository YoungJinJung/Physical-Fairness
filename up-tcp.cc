#include <iostream>
#include <string>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;

int main (int argc, char *argv[]){
int t = 0;
		DataRate linkRate("100Mbps");

		// experiment variables
		int nNode = 4;
		float base_dist = 20.0;
		float u_dist = 25.0;
		int retryLimit = 7;
		int queueSizePerNode = 5000;
		float delayPerNode = 3.0;
		float simulationTime = 40.0;
		
		// traffic parameters
  		//uint32_t MaxPacketSize = 1024;
  		uint32_t interPacketInterval = 4000;  // in MicroSeconds
  		//uint32_t maxPacketCount = 100000;
		string tcpver = "TcpReno";


		float fiarray[5] = {0.0,};

		//
		srand((unsigned int)time(NULL));
		ns3::RngSeedManager::SetSeed((unsigned int)time(NULL));

		CommandLine cmd;
		cmd.AddValue("TcpVersion","Tcp version of each experiment",tcpver);
		cmd.Parse(argc,argv);
		
		if(tcpver.compare("TcpReno")==0){
			Config::SetDefault("ns3::TcpL4Protocol::SocketType",TypeIdValue(TcpReno::GetTypeId()));
		}
		else if(tcpver.compare("TcpWestwood")==0){
			Config::SetDefault("ns3::TcpL4Protocol::SocketType",TypeIdValue(TcpWestwood::GetTypeId()));
		}
		else{
			cout<<"Invalid TCP version"<<endl;
			exit(1);
		}
		while(t<5){
			if(t == 0){
				std::cout<<"1st Experiment"<<std::endl;
			}else if(t==1){
				std::cout<<"2nd Experiment"<<std::endl;
			}else if(t==2){
				std::cout<<"3rd Experiment"<<std::endl;
			}else{
				std::cout<<t+1<<"th Experiment"<<std::endl;
			}



			// Create the nodes required by the topology
			NodeContainer nodes;
			nodes.Create(2);
			NodeContainer wifiNodes;
			wifiNodes.Create(4);

			// Create the point-to-point link required by the topology(node0 ------ node1)
			PointToPointHelper pointToPoint;
			pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (linkRate));
			pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
			NetDeviceContainer devices;
			devices = pointToPoint.Install (nodes);

			// set Retry Limit
			Config::SetDefault("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue(retryLimit));
			Config::SetDefault("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue(nNode*queueSizePerNode));
			Config::SetDefault("ns3::WifiMacQueue::MaxDelay", TimeValue(Seconds (nNode*delayPerNode)));



			//Set Wifi Ap and Station Nodes(node 1 ............node2 ............node3.............node4..............node5
			YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
			YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
			wifiPhy.SetChannel(wifiChannel.Create());
			WifiHelper wifi = WifiHelper::Default();

			NqosWifiMacHelper mac = NqosWifiMacHelper::Default();

			Ssid ssid = Ssid("Sta1");
			mac.SetType("ns3::StaWifiMac","Ssid",SsidValue(ssid),"ActiveProbing",BooleanValue(false));
			NetDeviceContainer staDevices;
			staDevices = wifi.Install(wifiPhy,mac,wifiNodes);

			mac.SetType("ns3::ApWifiMac","Ssid",SsidValue(ssid));
			NetDeviceContainer apDevices;
			apDevices = wifi.Install(wifiPhy,mac,nodes.Get(1));

			//Set Position of each wireless nodes
			MobilityHelper mobility;

			Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
			positionAlloc->Add(Vector(0.0,4.0,0.0));
			positionAlloc->Add(Vector(5.0,8.0,0.0));
			positionAlloc->Add(Vector(base_dist,8.0,0.0));  // for the best case
			positionAlloc->Add(Vector(base_dist+2*u_dist,13.0,0.0));
			positionAlloc->Add(Vector(base_dist+3*u_dist,18.0,0.0));
			positionAlloc->Add(Vector(base_dist+4*u_dist,23.0,0.0));
			mobility.SetPositionAllocator(positionAlloc);
			mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");//Static position
			mobility.Install(nodes);
			mobility.Install(wifiNodes);

			// Install the internet stack on the nodes
			InternetStackHelper internet;
			internet.Install(nodes);
			internet.Install(wifiNodes);

			// We've got the "hardware" in place.  Now we need to add IP addresses.
			Ipv4AddressHelper ipv4;
			ipv4.SetBase ("10.1.1.0", "255.255.255.0");
			Ipv4InterfaceContainer i = ipv4.Assign (devices);
			ipv4.SetBase("10.1.2.0","255.255.255.0");
			i.Add(ipv4.Assign(apDevices));
			i.Add(ipv4.Assign(staDevices));

			Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

			uint16_t port = 1010;

			BulkSendHelper * sources[nNode] ;
			//OnOffHelper * sources[nNode] ;
			ApplicationContainer sourceApps[nNode] ;
		
			for(int k = 0 ; k < nNode ; k++){
			//for(int k = nNode-1 ; k >= 0 ; k--){
	
			sources[k] = new BulkSendHelper("ns3::TcpSocketFactory",InetSocketAddress(i.GetAddress(0), port));
			(*sources[k]).SetAttribute ("MaxBytes", UintegerValue (0));
			sourceApps[k] = (*sources[k]).Install (wifiNodes.Get(k));
			}		
			
			double start_time = 10.0 * 1000000 + static_cast<double> (rand()%100) ;
			double time[nNode];

			//unduplicated time setting of 4 nodes 
			for(int k = nNode-1 ; k >= 0 ; k--){
			//for(int k = 0 ; k < nNode ; k++){
			   if ( (k == t) || (t>3) ) { 
				int rv = rand()%(interPacketInterval);
				time[k] = start_time + static_cast<double> (rv) ;
				sourceApps[k].Start (MicroSeconds (time[k]));
				sourceApps[k].Stop (MicroSeconds(time[k]+simulationTime*1000000));
				cout<<"Node " << k << " starts at " << rv <<endl;
			   } else {
				sourceApps[k].Stop (MicroSeconds(1));
				cout<<"Node " << k << " stops at 0" <<endl;
			   }
			}
					
			//
			LogComponentEnable("UdpSocket", LOG_LEVEL_INFO);


			// Create a PacketSinkApplication and install it on wifinode
			Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), port));
			PacketSinkHelper sinkA ("ns3::TcpSocketFactory", sinkAaddr);
			ApplicationContainer sinkAppA = sinkA.Install (nodes.Get (0));
			sinkAppA.Start (Seconds (0.0));
			sinkAppA.Stop (Seconds (60.0));


			string filename;	
			stringstream ss;
			ss<<t+1;
			filename = ss.str();
			filename += "-up-tcp";
			filename += tcpver;
			filename += ".xml";
			AnimationInterface anim(filename);
			anim.SetMaxPktsPerTraceFile(5000000);

			FlowMonitorHelper flowmon;
			Ptr<FlowMonitor> monitor = flowmon.InstallAll();

			// Run the simulation
			Simulator::Stop (Seconds (50.0));
			Simulator::Run ();

			Ptr<PacketSink> sinks[nNode];
			for(int k= 0 ; k < nNode ; k++){
				sinks[k] = StaticCast<PacketSink> (sinkAppA.Get(0));
				std::cout << "TCP Rx Bytes : " << sinks[k]->GetTotalRx()  << ")\n";
			}



			monitor->CheckForLostPackets ();

			Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
			std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
			float rate[4]={0.0,};


			for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
			{

				Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
				if(t.destinationAddress=="10.1.1.1"&&(t.sourceAddress=="10.1.2.2"||t.sourceAddress=="10.1.2.3"||t.sourceAddress=="10.1.2.4"||t.sourceAddress=="10.1.2.5")){
				std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
				std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
				std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";

				float thruput =  i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024;
			
				std::cout << "  Throughput: " << thruput  << " Mbps\n"; 
				rate[i->first - 1] = thruput;
				}

			}

			float fi = ((rate[0]+rate[1]+rate[2]+rate[3])*(rate[0]+rate[1]+rate[2]+rate[3])) / (4 * (rate[0]*rate[0]+rate[1]*rate[1]+rate[2]*rate[2]+rate[3]*rate[3]));


			std::cout << "Fairness Index : " << fi <<std::endl;

			fiarray[t] = fi;
			monitor->SerializeToXmlFile("lab-1.flowmon", true, true);
			Simulator::Destroy ();
			t++;
			std::cout << std::endl<<std::endl;


		}


		std::cout<<"*\n*\n*\n*"<<std::endl;
		std::cout<<"average Fairness Index : "<<(fiarray[0]+fiarray[1]+fiarray[2]+fiarray[3]+fiarray[4])/5<<std::endl;




}



