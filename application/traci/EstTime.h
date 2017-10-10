/*
 * EstTime.h
 *
 *  Created on: Nov 11, 2015
 *      Author: tasos
 */

#ifndef ESTTIME_H_
#define ESTTIME_H_

#include "application/traci/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

double MeanSpeed(string edge, int id, list<infos> data)
{
    for (list<infos>::iterator it = data.begin(); it != data.end(); ++it)
    {
        if (it->roadId.find(edge) == 0 && it->roadId.find(edge) != it->roadId.npos)
        {
            double temp_sum = 0.0;
            int module_sum = 0;
            for (vector<speedId>::iterator it2 = it->speed_id.begin(); it2 != it->speed_id.end(); ++it2)
            {
                if (it2->speed <= 2.0)
                    return -1.0;
                temp_sum += it2->speed;
                module_sum++;
            }
            temp_sum = (double)temp_sum / module_sum;
            if (temp_sum > 0.0)
            {
                return temp_sum;
            }
            else if (temp_sum == 0.0)
            {
                return -1.0;
            }
        }
    }
    return 0.0;
}

double min(double a, double b)
{
    return (a < b) ? a : b;
}

// Not used

/*double ArrivalTime(list<string> edges, int id, double speed, string currEdge)
{
    double time = 0.0;
    list<string>::iterator it;
    for(it = edges.begin(); it != edges.end() && *it != currEdge; it++);
    if (it != edges.end())
        it++;
    for (; it != edges.end(); it++)
    {
        double laneLength = 0.0;
        double laneSpeed = 0.0;
        XMLDocument xmlDoc;
        XMLError eResult = xmlDoc.LoadFile("/home/tasos/Desktop/workspace/veins/examples/veins/traffic_map1/traffic_map1.net.xml");
        if (eResult != XML_SUCCESS) return -1.0;

        XMLElement *pRoot = xmlDoc.FirstChildElement("net");
        if (pRoot)
        {
            XMLElement *pParm = pRoot->FirstChildElement("edge");
            while (pParm)
            {
                if(!strcmp((*it).c_str(), pParm->Attribute("id")))
                {
                    XMLElement *pLane = pParm->FirstChildElement("lane");
                    laneLength = atof(pLane->Attribute("length"));
                    laneSpeed = MeanSpeed(*it, id);
                    if (laneSpeed == 0.0)
                        laneSpeed = min(speed, atof(pLane->Attribute("speed")));
                    break;
                }
                pParm = pParm->NextSiblingElement("edge");
            }
        }
        xmlDoc.Clear();
        time += laneLength / laneSpeed;
    }
    return time;
}*/

#endif /* ESTTIME_H_ */
