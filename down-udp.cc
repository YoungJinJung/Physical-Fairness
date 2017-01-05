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
#include "ns3/stats-module.h"


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WiFiDistanceExperiment");

void TxCallback (Ptr<CounterCalculator<uint32_t> > datac,std::string path,Ptr<const Packet> packet) {

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
		uint32_t queueSizePerNode = 100000000;
		float delayPerNode = 3.0;
		float simulationTime = 30.0;

		// traffic parameters
  		uint32_t MaxPacketSize = 1024;
  		uint32_t interPacketInterval = 3000;  // in MicroSeconds
  		uint32_t maxPacketCount = 10000000;


		//float fiarray[5] = {0.0,};

		//
		srand((unsigned int)time(NULL));
		ns3::RngSeedManager::SetSeed((unsigned int)time(NULL));



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
			//pointToPoint.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(nNode*queueSizePerNode));
			NetDeviceContainer d0d4 = pointToPoint.Install (n0n4);
			NetDeviceContainer d1d4 = pointToPoint.Install (n1n4);
			NetDeviceContainer d2d4 = pointToPoint.Install (n2n4);
			NetDeviceContainer d3d4 = pointToPoint.Install (n3n4);
		
			// set Retry Limit
			Config::SetDefault("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue(retryLimit));
			Config::SetDefault("ns3::WifiMacQueue::MaxPacketNumber", UintegerValue(nNode*queueSizePerNode));	
			//Config::SetDefault("ns3::WifiMacQueue::DropPolicy", EnumValue(ns3::WifiMacQueue::DROP_OLDEST));
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

			UdpClientHelper * sources[nNode] ;
			//OnOffHelper * sources[nNode] ;
			ApplicationContainer sourceApps[nNode] ;
		
		
			for(int k = 0 ; k < nNode ; k++){
			//for(int k = nNode-1 ; k >= 0 ; k--){
		
  			sources[k] = new UdpClientHelper (wifiip.GetAddress(k+1), port);
			(*sources[k]).SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
			(*sources[k]).SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
			(*sources[k]).SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
			sourceApps[k] = (*sources[k]).Install (nodes.Get (k));
			}


			// in MicroSeconds
			double start_time = 10.0 * 1000000 + static_cast<double> (rand()%100) ;
			double time[nNode];

			//unduplicated time setting of 4 nodes 
			//for(int k = nNode-1 ; k >= 0 ; k--){
			for(int k = 0 ; k < nNode ; k++){
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
			PacketSinkHelper sinkA ("ns3::UdpSocketFactory", sinkAaddr);
			ApplicationContainer sinkAppA = sinkA.Install (wifiNodes);
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
			filename += "-down-udp.xml";
		
		//counting recived frame in Mac Layer
		 if(t <4){//each single node excution 
			Ptr<CounterCalculator<uint32_t> > totalTx1 = CreateObject<CounterCalculator<uint32_t> >();
			totalTx1->SetKey ("p2p-Tx-frames");
			nodename = "node[";	
			ss2<<t;
			nodename += ss2.str();
			nodename += "]";
			totalTx1->SetContext (nodename);
			string path = "/NodeList/";
			path += ss2.str();
			path += "/DeviceList/*/$ns3::PointToPointNetDevice/MacTx";	
			Config::Connect (path,MakeBoundCallback (&TxCallback,totalTx1));
			data.AddDataCalculator (totalTx1);

			Ptr<PacketCounterCalculator> totalRx = CreateObject<PacketCounterCalculator>();
			totalRx->SetKey ("wifi-Rx-frames");
			nodename = "node[";	
			ss3<<t+5;
			nodename += ss3.str();
			nodename += "]" ;
			totalRx->SetContext(nodename);
			path = "/NodeList/";
			path += ss3.str();
			path += "/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx";
			Config::Connect (path,MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx));
			data.AddDataCalculator (totalRx);

		}else{//all nodes excution

			Ptr<PacketCounterCalculator> totalRx1 = CreateObject<PacketCounterCalculator>();
			Ptr<PacketCounterCalculator> totalRx2 = CreateObject<PacketCounterCalculator>();
			Ptr<PacketCounterCalculator> totalRx3 = CreateObject<PacketCounterCalculator>();
			Ptr<PacketCounterCalculator> totalRx4 = CreateObject<PacketCounterCalculator>();
	 		totalRx1->SetKey ("wifi-Rx-frames");
 			totalRx2->SetKey ("wifi-Rx-frames");
			totalRx3->SetKey ("wifi-Rx-frames");
			totalRx4->SetKey ("wifi-Rx-frames");
		

			Ptr<CounterCalculator<uint32_t> > totalTx1 = CreateObject<CounterCalculator<uint32_t> >();
			Ptr<CounterCalculator<uint32_t> > totalTx2 = CreateObject<CounterCalculator<uint32_t> >();
			Ptr<CounterCalculator<uint32_t> > totalTx3 = CreateObject<CounterCalculator<uint32_t> >();
			Ptr<CounterCalculator<uint32_t> > totalTx4 = CreateObject<CounterCalculator<uint32_t> >();
			totalTx1->SetKey ("p2p-Tx-frames");
			totalTx2->SetKey ("p2p-Tx-frames");
			totalTx3->SetKey ("p2p-Tx-frames");
			totalTx4->SetKey ("p2p-Tx-frames");


			Ptr<PacketCounterCalculator> totalRx5 = CreateObject<PacketCounterCalculator>();
			Config::Connect ("/NodeList/4/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx5));
	
			Ptr<CounterCalculator<uint32_t> > totalTx5 = CreateObject<CounterCalculator<uint32_t> >();
			Config::Connect ("/NodeList/4/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",MakeBoundCallback (&TxCallback,totalTx5));
			totalTx5->SetContext ("node[4]");
			totalRx5->SetContext ("node[4]");
			
			totalRx5->SetKey("p2p-Rx-frames");
			totalTx5->SetKey("Ap-Tx-frames");
			
			
			totalTx1->SetContext ("node[0]");
			Config::Connect ("/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx1));
			
			
			totalTx2->SetContext ("node[1]");
			Config::Connect ("/NodeList/1/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx2));
			
			
			totalTx3->SetContext ("node[2]");
			Config::Connect ("/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx3));

			
			totalTx4->SetContext ("node[3]");
			Config::Connect ("/NodeList/3/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",MakeBoundCallback (&TxCallback,totalTx4));
		

			totalRx1->SetContext("node[5]");
			Config::Connect ("/NodeList/5/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx1));
			
			totalRx2->SetContext("node[6]");
 			Config::Connect ("/NodeList/6/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx2));

			totalRx3->SetContext("node[7]");
 			Config::Connect ("/NodeList/7/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx3));

			totalRx4->SetContext("node[8]");
			Config::Connect ("/NodeList/8/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",MakeCallback (&PacketCounterCalculator::PacketUpdate,totalRx4));

				
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
			filename += "-down-udp";
			filename += "-r7.tr";
			Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (filename);
  			wifiPhy.EnableAsciiAll (stream);
 			pointToPoint.EnableAsciiAll (stream);
 			internet.EnableAsciiIpv4All (stream);
			

			// Run the simulation
			Simulator::Stop (Seconds (60.0));
			Simulator::Run ();

			Ptr<PacketSink> sinks[nNode];
			double throughput[nNode];
		
			for(int k= 0 ; k < nNode ; k++){
				sinks[k] = StaticCast<PacketSink> (sinkAppA.Get(k));
				std::cout << (k+1)<<"node's UDP Rx Bytes : " << sinks[k]->GetTotalRx()  << "\n";							
								
			}
			for(int k= 0 ; k < nNode ; k++){
				throughput[k]  = ((sinks[k]->GetTotalRx()*8)/(1e6 * simulationTime));
				std::cout << (k+1)<<"node's Throughput : " << throughput[k] << "Mbps\n";					
								
			}
			
			
			/*
			//Create flowmonitor file
			FlowMonitorHelper flowmon;
			Ptr<FlowMonitor> monitor = flowmon.InstallAll();
			monitor->CheckForLostPackets ();
			Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
			std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
			float rate[4]={0.0,};
			for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
			{

				Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
					
				std::cout << "Flow r" << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
				std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
				std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
				std::cout << "  Tx Frames:    " << (i->second.txBytes)/1052 << "\n";
				std::cout << "  Rx Frames:    " << (i->second.rxBytes)/1052 << "\n";

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



