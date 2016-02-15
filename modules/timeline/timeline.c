#include <stdio.h>
#include <stdlib.h>
#include "timeline.h"
#include "../map/map.h"
#include <math.h>

#define TRUE 1
#define FALSE 0

// Here's how we plan on solving the order timeline
// First, you obviously have to enter the order timeline
// If the latest order is a regular order, we'll try to deliver it ASAP
// If our prediction function says we'll be late for the delivery, we'll consider dropping this delivery
// and moving on to the next one
// If there's time left to the next delivery (a.k.a. free time), we'll consider doing one of 3 things:
// 1) Find more Pizzas
// 2) Pick up preorder Pizzas
// 3) Pick up previously canceled / delayed orders 
// After every delivery is done, we want to consider using it as free time. If we can squeeze some
// extra work in, we might as well
// However, this could backfire in the sense that you *could* get away with taking some free time for the
// next 2 orders, but over time, it'll cost you because you'll be delayed for all orders after that
// Somewhere in the middle of all of this, we also need to consider instances when we will *definitely*
// need 2 arms, like when 1 house orders 2 Pizzas. In this situation, we don't want our second arm to be
// filled already.
// So the best thing to do might be to look for these *definite* needs first and "blocking off" that period
// of time. These blocked off periods will be considered in our "free time" considerations, and any overlapping
// permutations will be dismissed.

Timeline *our_timeline;

// A time period when we will *require* 2 arms
// Since it's end period is set dynamically, every time that we stop requiring it
// we can check the future orders again for another "definite need"
TimeBlock *next_required_period;

// We only have 10 pizza pick up points, and since the rules state that no pizzas will be added
// or removed *during* a run, we can assume that the max orders we'll get is 10
const int MAX_ORDERS = 10;
const int DELIVERY_PERIOD = 30;

void CreateOrder(
        Timeline *timeline,
        char colour,
        char size,
        int order_time,
        char order_type,
        char *delivery_house_name
    ) {

    Order *new_order;
    int i = 0;

    new_order = malloc(sizeof(Order));
    new_order->colour = colour;
    new_order->size = size;
    new_order->order_time = order_time;
    new_order->order_type = order_type;
    new_order->delivery_house = GetNodeByName(delivery_house_name);
    new_order->status = 'n';
    new_order->pickup_point = NULL;
    new_order->block = malloc(sizeof(TimeBlock));

    if (order_type == 'p') {
        new_order->block->start = order_time - DELIVERY_PERIOD;
        new_order->block->end = order_time + DELIVERY_PERIOD;
    }
    else {
        new_order->block->start = order_time;
        new_order->block->end = order_time + DELIVERY_PERIOD;
    }

    i = timeline->len;
    // printf("%d: %s\n", i, delivery_house_name);
    // Here, we want to insert the new_order into the array and keep the
    // array sorted by order_time in ascending order
    // To do so, we shift the array's elements to the right until we find
    // the new_order's spot. Then we insert the new_order at that spot
    while (
        i > 0 &&
        timeline->orders[i - 1] != NULL &&
        new_order->order_time < timeline->orders[i - 1]->order_time
    ) {
        timeline->orders[i] = timeline->orders[i - 1];
        i--;
    }
    timeline->orders[i] = new_order;
    timeline->len++;
}

void InitTimeline() {
    our_timeline = malloc(sizeof(Timeline));
    our_timeline->orders = malloc(MAX_ORDERS * sizeof(Order));
    our_timeline->len = 0;

    next_required_period = malloc(sizeof(TimeBlock));
    next_required_period->start = next_required_period->end = 0;

    CreateOrder(our_timeline, 'r', 'l', 30, 'r', "H12");
    CreateOrder(our_timeline, 'g', 'l', 50, 'p', "H2");
    CreateOrder(our_timeline, 'r', 'l', 60, 'r', "H4");
    CreateOrder(our_timeline, 'b', 'l', 89, 'r', "H4");
    CreateOrder(our_timeline, 'b', 'l', 0, 'r', "H6");
    CreateOrder(our_timeline, 'r', 'l', 30, 'p', "H6");
}

Timeline *GetTimeline() {
    return our_timeline;
}

// TODO: Consider adding a threshold. An overlap of 1s means very little
int CheckOverlap(TimeBlock *a, TimeBlock *b){
    // An overlap occurs if A starts before B ends *and* B starts before A ends.
    if (
        a->start < b->end &&
        b->start < a->end
    ) {
        return TRUE;
    }
    return FALSE;
}

TimeBlock *GetCurrentTimeBlock() {
    TimeBlock *current;
    current = malloc(sizeof(TimeBlock));
    // TODO: USE TIMER TO GET CURRENT TIME
    // current->start = TimerGetTime();
    current->start = 0;
    current->end = current->start + 1;
}

// Find the time block where we'll definitely need 2 arms
// Note that this function *does not* set the end time of the block time
// Since our algorithm doesn't look ahead into the future for delivery permutations
// we leave it to the arm lower function to set the end time
void FindNextDefiniteNeed(Timeline *timeline) {
    int i, j;
    Order *order1, *order2;
    float start_time = INFINITY; // The potential start time of the next_required_period
    float lowest_start_time = INFINITY; // The lowest required period we've found so far
    TimeBlock *next_potential; // Potentially the next_required_period

    // If there's a required_period going on right now, there's no point to checking for the next one
    // until this one ends
    if (CheckOverlap(GetCurrentTimeBlock(), next_required_period)) {
        return;
    }

    next_potential = malloc(sizeof(TimeBlock));
    next_potential->start = INFINITY;
    next_potential->end = INFINITY;

    // We'll look at every combination of the orders we've got and see if any of them have an overlapping period
    // and are going to the same house.
    // This is the only situation we see where we *definitely* need to use our 2 arms
    // If the orders go to different houses, we might spend more time and might get delayed for
    // our other orders, so this isn't a *definite* thing, it's something to consider, which
    // will be done by our algorithm in its "free time" state.
    // Note, though, that we can only consider finding our next definite need if our current one
    // is over
    // This function needs to find the *next* required period, not *a* required period.
    // So we need the one with the timeframe being as soon as possible compared to right now.
    for (i = 0; i < timeline->len; i++) {
        for (j = 0; j < timeline->len; j++) {
            if (i == j) {
                continue;
            }
            order1 = timeline->orders[i];
            order2 = timeline->orders[j];

            // The start time is set to when we pick up the order from second block,
            // since that's when we start using 2 arms
            // The end time will be changed dynamically as soon as we deliver the order,
            // since that's when we stop using 2 arms
            if (order1->block->start > order2->block->start) {
                start_time = order1->block->start;
            }
            else {
                start_time = order2->block->start;
            }

            // If it's the lowest one we've found
            // and going to the same house
            // and their timings overlap
            if (
                start_time < lowest_start_time &&
                order1->delivery_house == order2->delivery_house &&
                CheckOverlap(order1->block, order2->block) == TRUE
            ) {
                lowest_start_time = start_time;
                next_potential->start = start_time;
                printf("Overlap between %s, %f and %f\n", order1->delivery_house->name, order1->block->start, order2->block->start);
            }
            
        }
    }
    printf("Next period: %f to %f\n", next_potential->start, next_potential->end);
    // Out of all the required periods available, the one with the lowest time
    // is set to next_potential, so we set that to the next_required_period
    next_required_period = next_potential;
}

// This is called when we've got some free time, and want to consider doing the following:
// Finding more pizzas
// Picking an extra pizza up with the second arm
// Picking an old, canceled pizza up
void FreeTimeDecision() {

}

void Display(Order *current_order) {
    printf("colour: %c, ", current_order->colour);
    printf("size: %c, ", current_order->size);
    printf("order time: %d, ", current_order->order_time);
    printf("order type: %c, ", current_order->order_type);
    printf("house: %s\n", current_order->delivery_house->name);
    // printf("pickup point: %s\n", current_order->pickup_point);
}
