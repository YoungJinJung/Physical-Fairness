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
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/stats-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WiFiDistanceExperiment");

void RxCallback (Ptr<CounterCalculator<uint32_t> > datac,std::string path,Ptr<const Packet> packet) {

  NS_LOG_INFO ("Sent frame counted in " << datac->GetKey ());
  datac->Update ();
	
  // end TxCallback
}

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
		float simulationTime = 30.0;

			// traffic parameters
  		uint32_t interPacketInterval = 3000;  // in MicroSeconds
  	
		string tcpver = "TcpNewReno";
	

		//float fiarray[5] = {0.0,};

		//
		srand((unsigned int)time(NULL));
		ns3::RngSeedManager::SetSeed((unsigned int)time(NULL));
		
		CommandLine cmd;
		cmd.AddValue("TcpVersion","Tcp version of each experiment",tcpver);
		cmd.Parse(argc,argv);
		
		if(tcpver.compare("TcpVegas")==0){
			Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpVegas"));
		}
		else if(tcpver.compare("TcpWestwood")==0){
			Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpWestwood"));
		}
		else if(tcpver.compare("TcpNewReno")==0){
			Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpNewReno"));
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
			nodes.Create(5);
			NodeContainer wifiNodes;
			wifiNodes.Create(4);
 			NodeContainer n0n4 = NodeContainer (nodes.Get (0), nodes.Get (4));
 			NodeContainer n1n4 = NodeContainer (nodes.Get (1), nodes.Get (4));
			NodeContainer n2n4 = NodeContainer (nodes.Get (2), nodes.Get (4));
  			NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));			
			
			// Create the point-to-point link required by the topology(node0 ------ node1)
			PointToPointHelper pointToPoint;
			pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (linkRate));
			pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
			NetDeviceContainer d0d4 = pointToPoint.Install (n0n4);
			NetDeviceContainer d1d4 = pointToPoint.Install (n1n4);
			NetDeviceContainer d2d4 = pointToPoint.Install (n2n4);
			NetDeviceContainer d3d4 = pointToPoint.Install (n3n4);

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
			apDevices = wifi.Install(wifiPhy,mac,nodes.Get(4));

			//Set Position of each wireless nodes
			MobilityHelper mobility;

			Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
			positionAlloc->Add(Vector(0.0,4.0,0.0));
			positionAlloc->Add(Vector(0.0,4.0,0.0));
			positionAlloc->Add(Vector(0.0,4.0,0.0));
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
			Ipv4InterfaceContainer ip[nNode];
			ipv4.SetBase ("10.1.1.0", "255.255.255.0");
			ip[0] = ipv4.Assign (d0d4);
			ipv4.SetBase ("10.1.2.0", "255.255.255.0");
			ip[1] = ipv4.Assign(d1d4);
			ipv4.SetBase ("10.1.3.0", "255.255.255.0");
			ip[2] = ipv4.Assign(d2d4);
			ipv4.SetBase ("10.1.4.0", "255.255.255.0");
			ip[3] = ipv4.Assign(d3d4);
			ipv4.SetBase("10.1.5.0","255.255.255.0");
			Ipv4InterfaceContainer wifiip = ipv4.Assign(apDevices);
			wifiip.Add(ipv4.Assign(staDevices));

			Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

			uint16_t port = 1010;

			BulkSendHelper * sources[nNode] ;
			//OnOffHelper * sources[nNode] ;
			ApplicationContainer sourceApps[nNode] ;
		
			for(int k = 0 ; k < nNode ; k++){
			//for(int k = nNode-1 ; k >= 0 ; k--){
			sources[k] = new BulkSendHelper("ns3::TcpSocketFactory",InetSocketAddress(ip[k].GetAddress(0), port));
			(*sources[k]).SetAttribute ("MaxBytes", UintegerValue (0));
			(*sources[k]).SetAttribute ("SendSize", UintegerValue (1024));
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
				cout<<"Node " << k << " starts at " << time[k]-10.0 * 1000000 <<endl;
			   } else {
				sourceApps[k].Stop (MicroSeconds(1));
				cout<<"Node " << k << " stops at 0" <<endl;
			   }
			}
					
			//
			LogComponentEnable("TcpSocket", LOG_LEVEL_INFO);


			// Create a PacketSinkApplication and install it on wifinode
			Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), port));
			PacketSinkHelper sinkA ("ns3::TcpSocketFactory", sinkAaddr);
			ApplicationContainer sinkAppA = sinkA.Install (nodes);
			sinkAppA.Start (Seconds (0.0));
			sinkAppA.Stop (Seconds (60.0));
			
			

			string filename;
			string nodename;	
			stringstream ss1;
			stringstream ss2;
			stringstream ss3;
			ss1<<t+1;
			filename = ss1.str();
			DataCollector data;
			data.DescribeRun(filename,filename,filename,filename);
			filename += "-up-tcp.xml";
		
		//counting recived frame in Mac Layer
		 if(t <4){//each single node excution 
			Ptr<CounterCalculator<uint32_t> > totalRx1 = CreateObject<CounterCalculator<uint32_t> >();
			totalRx1->SetKey ("p2p-Rx-frames");
			nodename = "node[";	
			ss2<<t;
			nodename += ss2.str();
			nodename += "]";
			totalRx1->SetContext (nodename);
			string path = "/NodeList/";
			path += ss2.str();
			path += "/DeviceList/*/$ns3::PointToPointNetDevice/MacRx";	
			Config::Connect (path,MakeBoundCallback (&RxCallback,totalRx1));
			data.AddDataCalculator (totalRx1);

			Ptr<PacketCounterCalculator> totalTx = CreateObject<PacketCounterCalculator>();
			totalTx->SetKey ("wifi-Tx-frames");
			nodename = "node[";	
			ss3<<t+5;
			nodename += ss3.str();
			nodename += "]" ;
			totalTx->SetContext(nodename);
			path = "/NodeList/";
			path += ss3.str();
			path += "/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx";
			Config::Connect (path,MakeCallback (&PacketCounterCalculator::PacketUpdate,totalTx));
			data.AddDataCalculator (totalTx);

		}else{//all nodes excution

			Ptr<PacketCounterCalculator> totalTx1 = CreateObject<PacketCounterCalculator>();
			Ptr<PacketCounterCalculator> totalTx2 = CreateObject<PacketCounterCalculator>();
			Ptr<PacketCounterCalculator> totalTx3 = CreateObject<PacketCounterCalculator>();
			Ptr<PacketCounterCalculator> totalTx4 = CreateObject<PacketCounterCalculator>();
	 		totalTx1->SetKey ("wifi-Tx-frames");
 			totalTx2->SetKey ("wifi-Tx-frames");
			totalTx3->SetKey ("wifi-Tx-frames");
			totalTx4->SetKey ("wifi-Tx-frames");
		

			Ptr<CounterCalculator<uint32_t> > totalRx1 = CreateObject<CounterCalculator<uint32_t> >();
			Ptr<CounterCalculator<uint32_t> > totalRx2 = CreateObject<CounterCalculator<uint32_t> >();
			Ptr<CounterCalculator<uint32_t> > totalRx3 = CreateObject<CounterCalculator<uint32_t> >();
			Ptr<CounterCalculator<uint32_t> > totalRx4 = CreateObject<CounterCalculator<uint32_t> >();
			totalRx1->SetKey ("p2p-Rx-frames");
			totalRx2->SetKey ("p2p-Rx-frames");
			totalRx3->SetKey ("p2p-Rx-frames");
			totalRx4->SetKey ("p2p-Rx-frames");


			Ptr<PacketCounterCalculator> totalRx5 = CreateObject<PacketCounterCalculator>();
			Config::Connect ("/NodeList/4/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx5));
	
			Ptr<CounterCalculator<uint32_t> > totalTx5 = CreateObject<CounterCalculator<uint32_t> >();
			Config::Connect ("/NodeList/4/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeBoundCallback (&RxCallback,totalTx5));
			totalTx5->SetContext ("node[4]");
			totalRx5->SetContext ("node[4]");
			
			totalTx5->SetKey("p2p-Tx-frames");
			totalRx5->SetKey("Ap-Rx-frames");


			totalRx1->SetContext ("node[0]");
			Config::Connect ("/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",MakeBoundCallback (&RxCallback,totalRx1));
			
			
			totalRx2->SetContext ("node[1]");
			Config::Connect ("/NodeList/1/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",MakeBoundCallback (&RxCallback,totalRx2));
			
			
			totalRx3->SetContext ("node[2]");
			Config::Connect ("/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",MakeBoundCallback (&RxCallback,totalRx3));

			
			totalRx4->SetContext ("node[3]");
			Config::Connect ("/NodeList/3/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",MakeBoundCallback (&RxCallback,totalRx4));
		

			totalTx1->SetContext("node[5]");
			Config::Connect ("/NodeList/5/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalTx1));
			
			totalTx2->SetContext("node[6]");
 			Config::Connect ("/NodeList/6/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalTx2));

			totalTx3->SetContext("node[7]");
 			Config::Connect ("/NodeList/7/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalTx3));

			totalTx4->SetContext("node[8]");
			Config::Connect ("/NodeList/8/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalTx4));

			data.AddDataCalculator (totalTx1);
			data.AddDataCalculator (totalTx2);
			data.AddDataCalculator (totalTx3);
			data.AddDataCalculator (totalTx4);
			data.AddDataCalculator (totalRx5);

			data.AddDataCalculator (totalTx5);
			data.AddDataCalculator (totalRx1);
			data.AddDataCalculator (totalRx2);
			data.AddDataCalculator (totalRx3);
			data.AddDataCalculator (totalRx4);

				
			

			
		}
			AsciiTraceHelper ascii;
			filename = ss1.str();
			filename += "-up-tcp";
			filename += "-r7.tr";
			Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (filename);
  			wifiPhy.EnableAsciiAll (stream);
 			pointToPoint.EnableAsciiAll (stream);
 			internet.EnableAsciiIpv4All (stream);

			Ptr<PacketSink> sinks[nNode];
			

			// Run the simulation
			Simulator::Stop (Seconds (50.0));
			Simulator::Run ();

		
			double throughput[nNode];
		
			for(int k= 0 ; k < nNode ; k++){
				sinks[k] = StaticCast<PacketSink> (sinkAppA.Get(k));
				std::cout << (k+1)<<"node's TCP Rx Bytes : " << sinks[k]->GetTotalRx()  << "\n";							
								
			}
			for(int k= 0 ; k < nNode ; k++){
				throughput[k]  = ((sinks[k]->GetTotalRx()*8)/(1e6 * simulationTime));
				std::cout << (k+1)<<"node's Throughput : " << throughput[k] << "Mbps\n";					
								
			}


			/*FlowMonitorHelper flowmon;
			Ptr<FlowMonitor> monitor = flowmon.InstallAll();
			monitor->CheckForLostPackets ();

			Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
			std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
			float rate[4]={0.0,};


			for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
			{

				Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
					
				std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
				std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
				std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
				std::cout << "  Tx Frames:    " << (i->second.txBytes)/MaxPacketSize << "\n";
				std::cout << "  Rx Frames:    " << (i->second.rxBytes)/MaxPacketSize << "\n";

				float thruput =  i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024;
				std::cout<<"timeLastRxPacket: "<<i->second.timeLastRxPacket.GetSeconds() <<std::endl;
				std::cout<< "timeFirstTxPacket: "<<i->second.timeFirstTxPacket.GetSeconds()<<std::endl;
				std::cout<<(i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())<<std::endl;

				std::cout << "  Throughput: " << thruput  << " Mbps\n"; 
				rate[i->first - 1] = thruput;

			}
			std::cout << "Total Throughput: " <<rate[0]+rate[1]+rate[2]+rate[3] << "\n";
				float fi = ((rate[0]+rate[1]+rate[2]+rate[3])*(rate[0]+rate[1]+rate[2]+rate[3])) / (4 * (rate[0]*rate[0]+rate[1]*rate[1]+rate[2]*rate[2]+rate[3]*rate[3]));
			std::cout << "Fairness Index : " << fi <<std::endl;

			fiarray[t] = fi;
			monitor->SerializeToXmlFile(filename, true, true);*/
			Ptr<DataOutputInterface> output = 0;
			output = CreateObject<SqliteDataOutput>();
			  if (output != 0)
			    output->Output (data);

			Simulator::Destroy ();
			t++;
			std::cout << std::endl<<std::endl;


		}


		//std::cout<<"*\n*\n*\n*"<<std::endl;
		//std::cout<<"average Fairness Index : "<<(fiarray[0]+fiarray[1]+fiarray[2]+fiarray[3]+fiarray[4])/5<<std::endl;




}


