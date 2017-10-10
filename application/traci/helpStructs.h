#include <string>
#include <vector>

using namespace std;

struct speedId {
    double speed;
    int vehId;
};

struct infos {
    string roadId;
    vector<speedId> speed_id;
};

struct EdgeSpeed {
    string roadId;
    double speed;
};
