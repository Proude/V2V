/*
 * ListInfo.h
 *
 *  Created on: Aug 2, 2014
 *      Author: tasos
 */

#ifndef LISTINFO_H_
#define LISTINFO_H_

//#include "BaseWaveApplLayer.h"
#include <vector>
#include <list>
#include <string>

using namespace std;

/*
 *  It takes as an input the message from a near vehicle
 *  and adds its contents to the list.
 *
 *  Input: WaveShortMessage wsm
 *
 *  Output: Updates list<infos> data
 */

void updateListInfo(WaveShortMessage* wsm, list<infos>* data)
{
    int eraseIter = 0;
    bool erased = false;
    bool updated = false;
    list<infos>::iterator it;
    vector<speedId>::iterator it2;


    for (it = data->begin(); it != data->end(); ++it)
    {
        eraseIter = 0;
        for (it2 = it->speed_id.begin(); it2 != it->speed_id.end();)
        {
            if (it2->vehId == wsm->getSenderAddress())
            {
                if (it->roadId == wsm->getRoadId())
                {
                    it2->speed = wsm->getVelocity();
                    it2++;
                    updated = true;
                }
                else
                {
                    it2 = it->speed_id.erase(it->speed_id.begin() + eraseIter);
                    erased = true;
                }
            }
            else
            {
                it2++;
            }
            eraseIter++;
        }
    }

    if (erased)
    {
        for (it = data->begin(); it != data->end(); ++it)
        {
            for (it2 = it->speed_id.begin(); it2 != it->speed_id.end(); ++it2)
            {
                if (it->roadId == wsm->getRoadId())
                {
                    struct speedId speedIdTemp;
                    speedIdTemp.speed = wsm->getVelocity();
                    speedIdTemp.vehId = wsm->getSenderAddress();
                    it->speed_id.push_back(speedIdTemp);
                    break;
                }
            }
        }
    }

    if (!updated && !erased)
    {
        for (it = data->begin(); it != data->end(); ++it)
            if (it->roadId == wsm->getRoadId())
                break;
        if (it == data->end())
        {
            struct infos st_temp;
            struct speedId speedIdTemp;
            speedIdTemp.speed = wsm->getVelocity();
            speedIdTemp.vehId = wsm->getSenderAddress();
            st_temp.roadId = wsm->getRoadId();
            st_temp.speed_id.push_back(speedIdTemp);
            data->push_front(st_temp);
        }
        else
        {
            struct speedId speedIdTemp;
            speedIdTemp.speed = wsm->getVelocity();
            speedIdTemp.vehId = wsm->getSenderAddress();
            it->speed_id.push_back(speedIdTemp);
        }
    }

    for (it = data->begin(), eraseIter = 0; it != data->end(); ++eraseIter)
        if (it->speed_id.empty())
            it = data->erase(it);
        else
            it++;

}

/*
 *  Prints list<infos> data
 */

void printList(list<infos>* data)
{
    for (list<infos>::iterator it = data->begin(); it != data->end(); ++it)
    {
        cout << it->roadId << endl;
        for (vector<speedId>::iterator it2 = it->speed_id.begin(); it2 != it->speed_id.end(); ++it2)
            cout << it2->speed << " " << it2->vehId << endl;
    }
}

#endif /* LISTINFO_H_ */
