/*
 * FindJunction.h
 *
 *  Created on: Sep 15, 2015
 *      Author: tasos
 */

#ifndef FINDJUNCTION_H_
#define FINDJUNCTION_H_

#include <iostream>
#include <list>
#include "application/traci/tinyxml2.h"
using namespace std;
using namespace tinyxml2;

/*
 *  Finds the next vetrice that the vehicle approachs
 *
 *  Input: current edge, filepath to extract data
 *
 *  Output: next vertice
 */

string findNextJunction(string edge, string fpath)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(fpath.c_str());
    if (eResult != XML_SUCCESS) return "";

    XMLElement *pRoot = xmlDoc.FirstChildElement("net");
    if (pRoot)
    {
        XMLElement *pParm = pRoot->FirstChildElement("edge");
        while (pParm)
        {
            if (pParm->Attribute("id") && pParm->Attribute("from") && pParm->Attribute("to"))
            {
                if (pParm->Attribute("id") == edge)
                {
                    //cout << pParm->Attribute("id") << " " << pParm->Attribute("to") << endl;
                    string rValue = pParm->Attribute("to");
                    xmlDoc.Clear();
                    return rValue;
                }
            }
            pParm = pParm->NextSiblingElement("edge");
        }
    }
    xmlDoc.Clear();
    return "";
}

/*list<string> findConnections(string Edge)
{
    list<string> connections;
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile("/home/tasos/Desktop/workspace/veins/examples/veins/traffic_map1/traffic_map1.net.xml");
    if (eResult != XML_SUCCESS) return {};

    XMLElement *pRoot = xmlDoc.FirstChildElement("net");
    if (pRoot)
    {
        XMLElement *pParm = pRoot->FirstChildElement("connection");
        while(pParm)
        {
            if (pParm->Attribute("from") == Edge)
                connections.push_front(pParm->Attribute("to"));
            pParm = pParm->NextSiblingElement("connection");
        }
    }
    xmlDoc.Clear();
    return connections;
}*/

#endif /* FINDJUNCTION_H_ */
