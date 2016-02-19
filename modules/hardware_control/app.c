#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../map/map.h"
#include "../dijkstra/dijkstra.h"
#include "../pos_encoder/pos_encoder.h"
#include "../bot_memory/bot_memory.h"
#include "../lcd/lcd.h"
#include "../hardware_control/hardware_control.h"

int main() {
    InitBotInfo();
    InitGraph();
    // LcdInitDevices();
    // LcdSet4Bit();
    // LcdInit();
    // MoveBotInitDevices();

    MoveBotToNode(GetNodeByName("r3"));

    return 0;
}