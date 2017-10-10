//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "application/traci/TraCIDemo11p.h"
#include "application/traci/ListInfo.h"
#include "mobility/traci/TraCICoord.h"
#include "application/traci/FindJunction.h"
#include "application/traci/EstTime.h"
#include "application/traci/Dijkstra.h"

#include <boost/config.hpp>
#include <iostream>
#include <fstream>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

using namespace std;
using namespace boost;
using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;
using Veins::TraCIScenarioManagerAccess;
using Veins::TraCICommandInterface;
using Veins::TraCIConnection;

const simsignalwrap_t TraCIDemo11p::parkingStateChangedSignal = simsignalwrap_t(TRACI_SIGNAL_PARKING_CHANGE_NAME);

Define_Module(TraCIDemo11p);

void TraCIDemo11p::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	if (stage == 0) {
		traci = TraCIMobilityAccess().get(getParentModule());
		annotations = AnnotationManagerAccess().getIfExists();
		ASSERT(annotations);

		sentMessage = false;
		lastDroveAt = simTime();
		findHost()->subscribe(parkingStateChangedSignal, this);
		isParking = false;
		sendWhileParking = par("sendWhileParking").boolValue();
	}
}

void TraCIDemo11p::onBeacon(WaveShortMessage* wsm) {
    typedef adjacency_list<listS, vecS, directedS, no_property, property <edge_weight_t, double> > graph_t;
    typedef graph_traits <graph_t>::vertex_descriptor vertex_descriptor;
    typedef std::pair<int, int> Edge;

    string filepath = "/home/tasos/Desktop/workspace/veins/examples/veins/Volos_200_70/volos.net.xml";
    TraCICommandInterface* commandIfc = TraCIScenarioManagerAccess().get()->getCommandInterface();
    Coord curr_pos;
    Veins::TraCICoord junct_pos; //TraCi coordination points
    double junctionDist = 30.0; // distance until reaching junction
    string currEdge;

    updateListInfo(wsm, &traci->otherVehiclesData); // data
    //printList(&traci->otherVehiclesData);  // print data

    curr_pos = traci->getCurrentPosition();
    currEdge = commandIfc->getEdgeId(traci->getExternalId().c_str());

    int checkIfInsideJunction = 1;
    string nextJunction = findNextJunction(currEdge, filepath); // search next junction
    if (nextJunction != "") // before junction
    {
        junct_pos = commandIfc->getJunctionPosition(nextJunction);
        junctionDist = commandIfc->distanceRequest(TraCIScenarioManagerAccess().get()->omnet2traci(curr_pos), junct_pos, false);
    }
    else // inside junction
    {
        junctionDist = 10.0;
        checkIfInsideJunction = 0;
    }
    list<string> routeNodes; // List that contains the best route

    if (junctionDist < 100.0 && checkIfInsideJunction == 1) // we are really close to junction but not inside it
    {
        map<string, int> temp;
        map<pair<string, string>, string> all_edges;
        int num_edges = 0;
        int num_nodes = construct_map(&temp, &num_edges, &all_edges, filepath);
        Edge edge_array[num_edges];
        double weights[num_edges];
        string destination;
        string src = construct_edge_and_weights(edge_array, weights, &destination, temp, currEdge, commandIfc->getPlannedEdgeIds(traci->getExternalId()).back(), traci->otherVehiclesData, filepath);
        int num_arcs = sizeof(edge_array) / sizeof(Edge);
        graph_t g(edge_array, edge_array + num_arcs, weights, num_nodes);
        property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
        std::vector<vertex_descriptor> p(num_vertices(g));
        std::vector<double> d(num_vertices(g));
        vertex_descriptor s = vertex(temp[src], g);
        int e = vertex(temp[destination], g);
        dijkstra_shortest_paths(g, s,
                         predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
                         distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));
        std::vector< graph_traits< graph_t >::vertex_descriptor > path;
        graph_traits< graph_t >::vertex_descriptor current = e;
        while(current != s)
        {
            for (map<string, int>::iterator it = temp.begin(); it != temp.end(); ++it)
                if (it->second == current)
                    routeNodes.push_front(it->first);
            current=p[current];
        }
        for (map<string, int>::iterator it = temp.begin(); it != temp.end(); ++it)
            if (it->second == s)
                routeNodes.push_front(it->first);
        list<string> routeEdges;
        list<string>::iterator it = routeNodes.begin();
        list<string>::iterator jt = routeNodes.begin();
        jt++;
        for(; jt != routeNodes.end(); it++, jt++)
            routeEdges.push_back(all_edges[pair<string, string>((*it), (*jt))]);
        routeEdges.push_front(currEdge);
        commandIfc->changeVehicleRoute(traci->getExternalId(), routeEdges);
//        multimap<string, weights> graph;
//        //cout << traci->getExternalId() << ":" << endl;
//        clock_t begin = clock();
//        string src = graphInitialize(graph, traci->getSpeed(), traci->otherVehiclesData, currEdge, filepath);
//        clock_t end = clock();
//        string dest = getDestination(commandIfc->getPlannedEdgeIds(traci->getExternalId()).back(), filepath);
//        routeEdges = graphDijkstra(graph, src, dest, filepath);
//        cout << (double)(end - begin) / CLOCKS_PER_SEC << endl;
//        routeEdges.push_front(currEdge);
//        commandIfc->changeVehicleRoute(traci->getExternalId(), routeEdges);
    }
}

void TraCIDemo11p::onData(WaveShortMessage* wsm) {
//	findHost()->getDisplayString().updateWith("r=16,green");
//	annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), traci->getPositionAt(simTime()), "blue"));
//
//	if (traci->getRoadId()[0] != ':') traci->commandChangeRoute(wsm->getWsmData(), 9999);
//	if (!sentMessage) sendMessage(wsm->getWsmData());
}

void TraCIDemo11p::sendMessage(std::string blockedRoadId) {
	sentMessage = true;

	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
	wsm->setWsmData(blockedRoadId.c_str());
	sendWSM(wsm);
}
void TraCIDemo11p::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj) {
	Enter_Method_Silent();
	if (signalID == mobilityStateChangedSignal) {
		handlePositionUpdate(obj);
	}
	else if (signalID == parkingStateChangedSignal) {
		handleParkingUpdate(obj);
	}
}
void TraCIDemo11p::handleParkingUpdate(cObject* obj) {
	isParking = traci->getParkingState();
	if (sendWhileParking == false) {
		if (isParking == true) {
			(FindModule<BaseConnectionManager*>::findGlobalModule())->unregisterNic(this->getParentModule()->getSubmodule("nic"));
		}
		else {
			Coord pos = traci->getCurrentPosition();
			(FindModule<BaseConnectionManager*>::findGlobalModule())->registerNic(this->getParentModule()->getSubmodule("nic"), (ChannelAccess*) this->getParentModule()->getSubmodule("nic")->getSubmodule("phy80211p"), &pos);
		}
	}
}
void TraCIDemo11p::handlePositionUpdate(cObject* obj) {
	BaseWaveApplLayer::handlePositionUpdate(obj);

	// stopped for for at least 10s?
	if (traci->getSpeed() < 1) {
		if (simTime() - lastDroveAt >= 10) {
			findHost()->getDisplayString().updateWith("r=16,red");
			if (!sentMessage) sendMessage(traci->getRoadId());
		}
	}
	else {
		lastDroveAt = simTime();
	}
}
void TraCIDemo11p::sendWSM(WaveShortMessage* wsm) {
	if (isParking && !sendWhileParking) return;
	sendDelayedDown(wsm,individualOffset);
}
