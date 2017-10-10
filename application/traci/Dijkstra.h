#ifndef DIJKSTRA_H_
#define DIJKSTRA_H_

#include "modules/mobility/traci/TraCIMobility.h"

using namespace std;
using namespace tinyxml2;

struct weights
{
    string i;
    string j;
    string edge;
    double speed;
};

struct distanceList
{
    string v;
    double distNSpeed;
    bool sptSev;
};

typedef std::pair<int, int> Edge;

/*
 *  Initialize graph to be ready for Dijkstra algorithm
 */

string graphInitialize(multimap<string, weights> &weig, double speed, list<infos> data, string edge, string fpath)
{
    XMLDocument xmlDoc;
    string src;
    XMLError eResult = xmlDoc.LoadFile(fpath.c_str());
    if (eResult != XML_SUCCESS) return "";

    XMLElement *pRoot = xmlDoc.FirstChildElement("net");
    if (pRoot)
    {
        XMLElement *pParm = pRoot->FirstChildElement("edge");
        while (pParm)
        {
            if(pParm->Attribute("id") && pParm->Attribute("from") && pParm->Attribute("to")
                    && strcmp(pParm->Attribute("type"), "highway.pedestrian") != 0
                            && strcmp(pParm->Attribute("type"), "highway.footway") != 0
                                    && strcmp(pParm ->Attribute("type"), "highway.cycleway") != 0)
            {
                weights temp;
                temp.i = pParm->Attribute("from");
                temp.j = pParm->Attribute("to");
                temp.edge = pParm->Attribute("id");
                XMLElement *laneParm = pParm->FirstChildElement("lane");
                double laneSpeed = MeanSpeed(temp.edge, 0, data);
                if (laneSpeed == 0.0)
                    temp.speed = atof(laneParm->Attribute("length")) / atof(laneParm->Attribute("speed"));//min(speed, atof(laneParm->Attribute("speed")));
                else if (laneSpeed > 0.0)
                    temp.speed = atof(laneParm->Attribute("length")) / laneSpeed;
                else
                    temp.speed = INT_MAX - 1;
                //weig.push_front(temp);
                //cout << temp.edge << " " << temp.speed << endl;
                weig.insert(pair<string, weights>(temp.i, temp));
                weig.insert(pair<string, weights>(temp.j, temp));
                if (pParm->Attribute("id") == edge)
                    src = pParm->Attribute("to");

            }
            pParm = pParm->NextSiblingElement("edge");
        }
    }
    xmlDoc.Clear();
    return src;
}

int count_edges(string fpath)
{
    XMLDocument xmlDoc;
    int size = 0;
    XMLError eResult = xmlDoc.LoadFile(fpath.c_str());
    if (eResult != XML_SUCCESS) return -1;

    XMLElement *pRoot = xmlDoc.FirstChildElement("net");
    if (pRoot)
    {
        XMLElement *pParm = pRoot->FirstChildElement("edge");
        while (pParm)
        {
            if(pParm->Attribute("id") && pParm->Attribute("from") && pParm->Attribute("to")
                    && strcmp(pParm->Attribute("type"), "highway.pedestrian") != 0
                            && strcmp(pParm->Attribute("type"), "highway.footway") != 0
                                    && strcmp(pParm ->Attribute("type"), "highway.cycleway") != 0)
            {
                size++;
            }
            pParm = pParm->NextSiblingElement("edge");
        }
    }
    xmlDoc.Clear();
    return size;
}

int construct_map(std::map<string, int> *temp, int *count_edges, map<pair<string, string>, string> *all_edges, string fpath)
{
    XMLDocument xmlDoc;
    int size = 0;
    XMLError eResult = xmlDoc.LoadFile(fpath.c_str());
    if (eResult != XML_SUCCESS) return 0;

    XMLElement *pRoot = xmlDoc.FirstChildElement("net");
    if (pRoot)
    {
        XMLElement *pParm = pRoot->FirstChildElement("edge");
        while (pParm)
        {
            if(pParm->Attribute("id") && pParm->Attribute("from") && pParm->Attribute("to")
                    && strcmp(pParm->Attribute("type"), "highway.pedestrian") != 0
                            && strcmp(pParm->Attribute("type"), "highway.footway") != 0
                                    && strcmp(pParm ->Attribute("type"), "highway.cycleway") != 0)
            {
                std::pair<std::map<std::string,int>::iterator,bool> ret;
                ret = temp->insert ( std::pair<std::string,int>(pParm->Attribute("from"), size) );
                if (ret.second==false)
                    size--;
                size++;

                ret = temp->insert ( std::pair<std::string,int>(pParm->Attribute("to"), size) );
                if (ret.second==false)
                    size--;
                size++;
                (*count_edges)++;
                all_edges->insert(pair<pair<string, string>, string>(pair<string, string>(pParm->Attribute("from"), pParm->Attribute("to")), pParm->Attribute("id")));
            }
            pParm = pParm->NextSiblingElement("edge");
        }
    }
    xmlDoc.Clear();
    return size;
}

string construct_edge_and_weights(Edge *edge_array, double *weights, string *dest, map<std::string, int> temp, string currEdge, string destEdge, list<infos> data, string fpath)
{
    XMLDocument xmlDoc;
    string src;
    int size = 0;
    XMLError eResult = xmlDoc.LoadFile(fpath.c_str());
    if (eResult != XML_SUCCESS) exit(0);

    XMLElement *pRoot = xmlDoc.FirstChildElement("net");
    if (pRoot)
    {
        XMLElement *pParm = pRoot->FirstChildElement("edge");
        while (pParm)
        {
            if(pParm->Attribute("id") && pParm->Attribute("from") && pParm->Attribute("to")
                    && strcmp(pParm->Attribute("type"), "highway.pedestrian") != 0
                            && strcmp(pParm->Attribute("type"), "highway.footway") != 0
                                    && strcmp(pParm ->Attribute("type"), "highway.cycleway") != 0)
            {
                int from = temp[pParm->Attribute("from")];
                int to = temp[pParm->Attribute("to")];
                edge_array[size] = Edge(from, to);
                double speed = 0.0;
//                weights temp;
//                temp.i = pParm->Attribute("from");
//                temp.j = pParm->Attribute("to");
                //temp.edge = pPamr->Attribute("id");
                string edge = pParm->Attribute("id");
                XMLElement *laneParm = pParm->FirstChildElement("lane");
                double laneSpeed = MeanSpeed(edge, 0, data);
                if (laneSpeed == 0.0)
                    speed = atof(laneParm->Attribute("length")) / atof(laneParm->Attribute("speed"));//min(speed, atof(laneParm->Attribute("speed")));
                else if (laneSpeed > 0.0)
                    speed = atof(laneParm->Attribute("length")) / laneSpeed;
                else
                    speed = INT_MAX - 1;
                weights[size] = speed;
                //weig.push_front(temp);
                //cout << temp.edge << " " << temp.speed << endl;
//                weig.insert(pair<string, weights>(temp.i, temp));
//                weig.insert(pair<string, weights>(temp.j, temp));
                if (pParm->Attribute("id") == currEdge)
                    src = pParm->Attribute("to");
                if (pParm->Attribute("id") == destEdge)
                    *dest = pParm->Attribute("to");
                size++;
            }
            pParm = pParm->NextSiblingElement("edge");
        }
    }
    xmlDoc.Clear();
    return src;
}

bool findVertex(list<distanceList> &lst, string attr)
{
    for(list<distanceList>::iterator it = lst.begin(); it != lst.end(); it++)
        if ((*it).v == attr)
            return true;
    return false;
}

void distInitialize(list<distanceList> &distList, string fpath)
{
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(fpath.c_str());
    if (eResult != XML_SUCCESS) return;

    XMLElement *pRoot = xmlDoc.FirstChildElement("net");
    if (pRoot)
    {
        XMLElement *pParm = pRoot->FirstChildElement("edge");
        while (pParm)
        {
            if(pParm->Attribute("id") && pParm->Attribute("from") && pParm->Attribute("to"))
            {
                if(!findVertex(distList, pParm->Attribute("from")))
                {
                    distanceList temp;
                    temp.v = pParm->Attribute("from");
                    temp.distNSpeed = INT_MAX;
                    temp.sptSev = false;
                    distList.push_front(temp);
                }
                if(!findVertex(distList, pParm->Attribute("to")))
                {
                    distanceList temp;
                    temp.v = pParm->Attribute("to");
                    temp.distNSpeed = INT_MAX;
                    temp.sptSev = false;
                    distList.push_front(temp);
                }
            }
            pParm = pParm->NextSiblingElement("edge");
        }
    }
    xmlDoc.Clear();
}

distanceList minDistance(list<distanceList> &distL)
{
    distanceList min_index;
    double min = INT_MAX;

    for (list<distanceList>::iterator v = distL.begin(); v != distL.end(); v++)
      if ((*v).sptSev == false && (*v).distNSpeed <= min)
      {
          min = (*v).distNSpeed;
          min_index = (*v);
      }

    return min_index;
}

double getGraphValue(list<weights> &weigList, string vertexI, string vertexJ)
{
    for(list<weights>::iterator i = weigList.begin(); i != weigList.end(); i++)
        if ((*i).i == vertexI && (*i).j == vertexJ)
            return (*i).speed;
    return 0.0;
}

void printSolution(list<distanceList> &dist, int n)
{
   printf("Vertex   Distance from Source\n");
   for (list<distanceList>::iterator i = dist.begin(); i != dist.end(); i++)
      cout << (*i).v << " \t\t " << (*i).distNSpeed << endl;
   getchar();
}

list<string> graphDijkstra(multimap<string, weights> weig, string src, string destin, string fpath)
{
    //int aSize = weig.size() / 2;
    list<distanceList> dist;

    distInitialize(dist, fpath);
    for(list<distanceList>::iterator i = dist.begin(); i != dist.end(); i++)
        if ((*i).v == src)
        {
            (*i).distNSpeed = 0;
            break;
        }
    //string parent[aSize];
    map<string, string> parent;

    for(unsigned int i = 0; i < dist.size() - 1; i++)
    {
        distanceList u = minDistance(dist);
        if (u.v == destin)
            break;
        for(list<distanceList>::iterator j = dist.begin(); j != dist.end(); j++)
            if ((*j).v == u.v)
            {
                (*j).sptSev = true;
                break;
            }

        for(list<distanceList>::iterator j = dist.begin(); j != dist.end(); j++)
        {
            //double graphValue = getGraphValue(weig, u.v, (*j).v); //kapws na eksafanistei
            double graphValue = 0.0;
            pair <multimap<string, weights>::iterator, multimap<string, weights>::iterator> ret;
            ret = weig.equal_range(u.v);
            for(multimap<string, weights>::iterator m = ret.first; m != ret.second; m++)
                if (m->second.i == u.v && m->second.j == (*j).v)
                    graphValue = m->second.speed;
            if(!(*j).sptSev && graphValue && u.distNSpeed != INT_MAX && u.distNSpeed + graphValue < (*j).distNSpeed)
            {
                parent[(*j).v] = u.v; //parent.insert(pair<string, string>((*j).v, u.v));
                (*j).distNSpeed = u.distNSpeed + graphValue;
            }
        }
    }
//    cout << endl;
//    for (list<string>::iterator k = parent.begin(); k != parent.end(); k++)
//        cout << (*k) << " ";
//    cout << endl;

    list<string> tempEdges = list<string>();
    string temp = destin;
    pair<multimap<string, weights>::iterator, multimap<string, weights>::iterator> ret;
    ret = weig.equal_range(temp);
    for(multimap<string, weights>::iterator m = ret.first; m != ret.second; m++)
        if (m->second.i == parent[temp] && m->second.j == temp)
            tempEdges.push_front(m->second.edge);
    while (temp != src)
    {
        temp = parent[temp];
        pair<multimap<string, weights>::iterator, multimap<string, weights>::iterator> ret;
        ret = weig.equal_range(temp);
        for(multimap<string, weights>::iterator m = ret.first; m != ret.second; m++)
            if (m->second.i == parent[temp] && m->second.j == temp)
                tempEdges.push_front(m->second.edge);
    }
//    for (list<string>::iterator i = parent.begin(); i != parent.end(); i++)
//    {
//        list<string>::iterator l = i;
//        l++;
//        for (list<string>::iterator j = l; j != parent.end(); j++)
//        {
//            pair <multimap<string, weights>::iterator, multimap<string, weights>::iterator> ret;
//            ret = weig.equal_range((*i));
//
//            for(multimap<string, weights>::iterator k = ret.first; k != ret.second; k++)
//            {
//                if (k->second.i == (*i) && k->second.j == (*j))
//                {
//                    tempEdges.push_back(k->second.edge);
//                    i = j;
//                    j = i;
//                }
//                else if (k->second.i == (*i) && k->second.j == destin)
//                {
//                    tempEdges.push_back(k->second.edge);
//                    goto exitLoops;
//                }
//            }
//        }
//    }
//exitLoops:
//    for(list<string>::iterator it = tempEdges.begin(); it != tempEdges.end(); it++)
//        cout << (*it) << " ";
//    cout << endl;

    return tempEdges;
    //printSolution(dist, aSize);
}

/*
 *  Get vertice destination
 */

string getDestination(string edge, string fpath)
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
                    string temp = pParm->Attribute("to");
                    xmlDoc.Clear();
                    return temp;
                }
            }
            pParm = pParm->NextSiblingElement("edge");
        }
    }
    return "";
}

#endif //DIJKSTRA_H_
