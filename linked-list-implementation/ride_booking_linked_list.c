#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

typedef struct Location{
    int x;
    int y;
} Location;

typedef struct driver{
    int d_ID;
    char name[50];
    int vehicle_type;
    Location current_location;
    int status;
    float total_earnings;
    struct driver *next;
} Driver;

typedef struct passenger{
    int p_ID;
    char name[50];
    char mobile_number[10];
    int frequency;
    struct passenger *next;
} Passenger;

typedef struct Booking{
    int booking_id;
    int d_ID;
    int p_ID;
    int vehicle_type;
    float distance_travelled;
    float fare;
    int date;
    int completed;  // Prevents completing the same booking more than once
    struct Booking *next;
} Booking;

void addDriver(Driver **head,int id, char name[],int type,int x,int y){
    Driver *temp = *head;
    while(temp != NULL){
        if(temp->d_ID == id){
            printf("Duplicate driver ID not allowed\n");
            return;
        }
        temp = temp->next;
    }

    if(type != 0 && type != 1){
        printf("Invalid vehicle type\n");
        return;
    }

    Driver *new_driver = (Driver*)malloc(sizeof(Driver));
    if(new_driver == NULL){
        printf("Memory allocation failed\n");
        return;
    }
    new_driver->d_ID = id;
    snprintf(new_driver->name, sizeof(new_driver->name), "%s", name);
    new_driver->vehicle_type = type;
    new_driver->current_location.x = x;
    new_driver->current_location.y = y;
    new_driver->status = 0; // Available
    new_driver->total_earnings = 0.0;
    new_driver->next = NULL;

    if(*head == NULL){
        *head = new_driver;
    }
    else {
        temp = *head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_driver;
    }
}

void addPassenger(Passenger **head,int id, char name[],char mobile[]){
    Passenger *temp = *head;
    while(temp != NULL){
        if(temp->p_ID == id || strcmp(temp->mobile_number,mobile) == 0){
            printf("Duplicate passenger not allowed\n");
            return;
        }
        temp = temp->next;
    }

    Passenger *new_passenger = (Passenger*)malloc(sizeof(Passenger));
    if(new_passenger == NULL){
        printf("Memory allocation failed\n");
        return;
    }
    new_passenger->p_ID = id;
    snprintf(new_passenger->name, sizeof(new_passenger->name), "%s", name);
    snprintf(new_passenger->mobile_number, sizeof(new_passenger->mobile_number), "%s", mobile);
    new_passenger->frequency = 0;
    new_passenger->next = NULL;

    if(*head == NULL){
        *head = new_passenger;
    }
    else {
        temp = *head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_passenger;
    }
}

Driver* findNearestVehicle(Driver *head,int p_x,int p_y,int prefType){
    Driver *temp = head;
    Driver *nearest_driver = NULL;
    float min_distance = 1000000;

    while(temp != NULL){
        if(temp->status == 0 && (temp->vehicle_type == prefType || prefType == -1)){
            float dx = (float)temp->current_location.x - p_x;
            float dy = (float)temp->current_location.y - p_y;
            float distance = sqrt(dx*dx + dy*dy);
            if(distance <= 5 && distance < min_distance){
                min_distance = distance;
                nearest_driver = temp;
            }
        }
        temp = temp->next;
    }
    return nearest_driver;
}

Passenger* findPassenger(Passenger *head,int p_ID){
    Passenger *temp = head;
    while(temp != NULL){
        if(temp->p_ID == p_ID){
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

int requestRide(Driver *dHead,Passenger *pHead,Booking **bHead,int p_ID,int p_x,int p_y,int prefType){
    Passenger *passenger = findPassenger(pHead, p_ID);
    if(passenger == NULL){
        printf("Passenger not found\n");
        return -1;
    }

    if(prefType != 0 && prefType != 1 && prefType != -1){
        printf("Invalid vehicle type\n");
        return -1;
    }

    Driver *driver = findNearestVehicle(dHead, p_x, p_y, prefType);
    if(driver == NULL){
        printf("No available drivers nearby\n");
        return -1;
    }

    Booking *new_booking = (Booking*)malloc(sizeof(Booking));
    if(new_booking == NULL){
        printf("Memory allocation failed\n");
        return -1;
    }

    static int booking_counter = 1;
    new_booking->booking_id = booking_counter++;
    new_booking->d_ID = driver->d_ID;
    new_booking->p_ID = passenger->p_ID;
    new_booking->vehicle_type = driver->vehicle_type;
    new_booking->distance_travelled = 0.0;
    new_booking->fare = 0.0;
    new_booking->date = 0; // No date input is provided by this program
    new_booking->completed = 0;
    new_booking->next = NULL;

    if(*bHead == NULL){
        *bHead = new_booking;
    }
    else {
        Booking *temp = *bHead;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_booking;
    }

    driver->status = 1;

    printf("Ride requested successfully. Driver ID: %d, Booking ID: %d\n", driver->d_ID, new_booking->booking_id);
    return new_booking->booking_id;
}

void completeRide(Driver *dHead,Passenger *pHead,Booking *bHead,int booking_id,float distance){
    if(distance < 0){
        printf("Distance cannot be negative\n");
        return;
    }

    Booking *temp = bHead;
    while(temp != NULL){
        if(temp->booking_id == booking_id){
            if(temp->completed){
                printf("This ride has already been completed\n");
                return;
            }

            // Update the booking details
            temp->distance_travelled = distance;
            if(temp->vehicle_type == 0)
                temp->fare = distance * 10;
            else
                temp->fare = distance * 5;
            temp->date = 0; // No date input is provided by this program

            // Update driver earnings and status
            Driver *driver = dHead;
            while(driver != NULL){
                if(driver->d_ID == temp->d_ID){
                    driver->total_earnings += temp->fare;
                    driver->status = 0; // Set driver to available
                    break;
                }
                driver = driver->next;
            }

            if(driver == NULL){
                printf("Driver not found\n");
                return;
            }

            // Update passenger frequency
            Passenger *passenger = pHead;
            while(passenger != NULL){
                if(passenger->p_ID == temp->p_ID){
                    passenger->frequency += 1;
                    break;
                }
                passenger = passenger->next;
            }

            temp->completed = 1;
            printf("Ride completed successfully. Fare: %.2f\n", temp->fare);
            return;
        }
        temp = temp->next;
    }
    printf("Booking not found\n");
}

float calculateDriverEarnings(Driver *dHead,int d_id){
    if(dHead == NULL){
        printf("No drivers available\n");
        return -1.0;
    }
    Driver *temp = dHead;
    while(temp != NULL){
        if(temp->d_ID == d_id){
            return temp->total_earnings;
        }
        temp = temp->next;
    }
    printf("Driver not found\n");
    return -1.0;
}

void displayTopDrivers(Driver *dHead){
    if(dHead == NULL){
        printf("No drivers available\n");
        return;
    }
    Driver *top1 = NULL, *top2 = NULL, *top3 = NULL;
    Driver *temp = dHead;
    while(temp != NULL){
        if(top1 == NULL || temp->total_earnings > top1->total_earnings){
            top3 = top2;
            top2 = top1;
            top1 = temp;
        }
        else if(top2 == NULL || temp->total_earnings > top2->total_earnings){
            top3 = top2;
            top2 = temp;
        }
        else if(top3 == NULL || temp->total_earnings > top3->total_earnings){
            top3 = temp;
        }
        temp = temp->next;
    }
    printf("Top 3 Drivers:\n");
    if(top1 != NULL)
        printf("1. Driver ID: %d, Name: %s, Total Earnings: %.2f\n", top1->d_ID, top1->name, top1->total_earnings);
    if(top2 != NULL)
        printf("2. Driver ID: %d, Name: %s, Total Earnings: %.2f\n", top2->d_ID, top2->name, top2->total_earnings);
    if(top3 != NULL)
        printf("3. Driver ID: %d, Name: %s, Total Earnings: %.2f\n", top3->d_ID, top3->name, top3->total_earnings);
}

void displayFrequentPairs(Driver *dHead,Passenger *pHead,Booking *bHead){
    if(dHead == NULL || pHead == NULL || bHead == NULL){
        printf("Insufficient data to display frequent pairs\n");
        return;
    }
    typedef struct Pair{
        int d_ID;
        int p_ID;
        int count;
    } Pair;

    Pair pairs[100];
    int pair_count = 0;

    Booking *temp = bHead;
    while(temp != NULL){
        int found = 0;
        for(int i = 0; i < pair_count; i++){
            if(pairs[i].d_ID == temp->d_ID && pairs[i].p_ID == temp->p_ID){
                pairs[i].count++;
                found = 1;
                break;
            }
        }
        if(!found){
            if(pair_count < 100){
                pairs[pair_count].d_ID = temp->d_ID;
                pairs[pair_count].p_ID = temp->p_ID;
                pairs[pair_count].count = 1;
                pair_count++;
            }
            else{
                printf("Pair storage limit reached; remaining new pairs are not counted\n");
            }
        }
        temp = temp->next;
    }

    if(pair_count == 0){
        printf("No booking pairs available\n");
        return;
    }

    int max_count = 0;
    for(int i = 0; i < pair_count; i++){
        if(pairs[i].count > max_count)
            max_count = pairs[i].count;
    }

    printf("Frequent Driver-Passenger Pairs:\n");
    for(int i = 0; i < pair_count; i++){
        if(pairs[i].count == max_count){
            Driver *driver = dHead;
            while(driver != NULL && driver->d_ID != pairs[i].d_ID)
                driver = driver->next;

            Passenger *passenger = pHead;
            while(passenger != NULL && passenger->p_ID != pairs[i].p_ID)
                passenger = passenger->next;

            if(driver != NULL && passenger != NULL){
                printf("Driver ID: %d, Driver Name: %s, Passenger ID: %d, Passenger Name: %s, Rides Together: %d\n",
                    driver->d_ID, driver->name, passenger->p_ID, passenger->name, pairs[i].count);
            }
        }
    }
}

void displayAvailableVehicles(Driver *dHead){
    if(dHead == NULL){
        printf("No drivers available\n");
        return;
    }
    printf("Available Vehicles:\n");
    Driver *temp = dHead;
    int found = 0;
    while(temp != NULL){
        if(temp->status == 0){
            printf("Driver ID: %d, Name: %s, Vehicle Type: %s, Location: (%d, %d)\n",
                temp->d_ID, temp->name, temp->vehicle_type == 0 ? "Cab" : "Bike",
                temp->current_location.x, temp->current_location.y);
            found = 1;
        }
        temp = temp->next;
    }
    if(!found)
        printf("No available vehicles\n");
}

void updateDriverLocation(Driver *dHead,int d_ID,int new_x,int new_y){
    if(dHead == NULL){
        printf("No drivers available\n");
        return;
    }
    Driver *temp = dHead;
    while(temp != NULL){
        if(temp->d_ID == d_ID){
            if(temp->status == 1){
                printf("Cannot update location. Driver is currently on a ride.\n");
                return;
            }
            temp->current_location.x = new_x;
            temp->current_location.y = new_y;
            printf("Driver location updated successfully\n");
            return;
        }
        temp = temp->next;
    }
    printf("Driver not found\n");
}

void deleteDriver(Driver **dHead,int d_ID){
    if(*dHead == NULL){
        printf("No drivers available\n");
        return;
    }
    Driver *temp = *dHead, *prev = NULL;
    while(temp != NULL){
        if(temp->d_ID == d_ID){
            if(temp->status == 1){
                printf("Cannot delete driver. Driver is currently on a ride.\n");
                return;
            }
            if(prev == NULL)
                *dHead = temp->next;
            else
                prev->next = temp->next;

            free(temp);
            printf("Driver deleted successfully\n");
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    printf("Driver not found\n");
}

void displayBookingHistory(Booking *bHead){
    if(bHead == NULL){
        printf("No bookings available\n");
        return;
    }
    printf("Booking History:\n");
    Booking *temp = bHead;
    while(temp != NULL){
        printf("Booking ID: %d, Driver ID: %d, Passenger ID: %d, Vehicle Type: %s, Distance Travelled: %.2f, Fare: %.2f, Date: %d, Status: %s\n",
            temp->booking_id, temp->d_ID, temp->p_ID,
            temp->vehicle_type == 0 ? "Cab" : "Bike",
            temp->distance_travelled, temp->fare, temp->date,
            temp->completed ? "Completed" : "Booked");
        temp = temp->next;
    }
}

int main()
{
    Driver *dHead = NULL;
    Passenger *pHead = NULL;
    Booking *bHead = NULL;

    int choice;

    while(1)
    {
        printf("\n RIDE BOOKING SYSTEM \n");
        printf("1. Add Driver\n");
        printf("2. Add Passenger\n");
        printf("3. Request Ride\n");
        printf("4. Complete Ride\n");
        printf("5. Display Top Drivers\n");
        printf("6. Display Frequent Pairs\n");
        printf("7. Display Available Vehicles\n");
        printf("8. Update Driver Location\n");
        printf("9. Delete Driver\n");
        printf("10. Display Booking History\n");
        printf("0. Exit\n");
        printf("Enter choice: ");

        if(scanf("%d", &choice) != 1){
            printf("Invalid input\n");
            break;
        }

        if(choice == 0){
            printf("Exiting...\n");
            break;
        }

        int id, type, x, y, p_id, d_id, booking_id;
        char name[50], mobile[15];
        float distance;

        switch(choice)
        {
            case 1:
                printf("Enter Driver ID, Name, Vehicle Type(0=Cab,1=Bike), Location(x y): ");
                if(scanf("%d %49s %d %d %d", &id, name, &type, &x, &y) != 5){
                    printf("Invalid input\n");
                    return 1;
                }
                addDriver(&dHead, id, name, type, x, y);
                break;

            case 2:
                printf("Enter Passenger ID, Name, Mobile: ");
                if(scanf("%d %49s %14s", &id, name, mobile) != 3){
                    printf("Invalid input\n");
                    return 1;
                }
                addPassenger(&pHead, id, name, mobile);
                break;

            case 3:
                printf("Enter Passenger ID, Location(x y), Preferred Type(0=Cab,1=Bike): ");
                if(scanf("%d %d %d %d", &p_id, &x, &y, &type) != 4){
                    printf("Invalid input\n");
                    return 1;
                }
                {
                    int bid = requestRide(dHead, pHead, &bHead, p_id, x, y, type);
                    if(bid != -1)
                        printf("Booking ID: %d\n", bid);
                }
                break;

            case 4:
                printf("Enter Booking ID and Distance: ");
                if(scanf("%d %f", &booking_id, &distance) != 2){
                    printf("Invalid input\n");
                    return 1;
                }
                completeRide(dHead, pHead, bHead, booking_id, distance);
                break;

            case 5:
                displayTopDrivers(dHead);
                break;

            case 6:
                displayFrequentPairs(dHead, pHead, bHead);
                break;

            case 7:
                displayAvailableVehicles(dHead);
                break;

            case 8:
                printf("Enter Driver ID and new location(x y): ");
                if(scanf("%d %d %d", &d_id, &x, &y) != 3){
                    printf("Invalid input\n");
                    return 1;
                }
                updateDriverLocation(dHead, d_id, x, y);
                break;

            case 9:
                printf("Enter Driver ID to delete: ");
                if(scanf("%d", &d_id) != 1){
                    printf("Invalid input\n");
                    return 1;
                }
                deleteDriver(&dHead, d_id);
                break;

            case 10:
                displayBookingHistory(bHead);
                break;

            default:
                printf("Invalid choice\n");
        }
    }

    // Free all allocated linked-list nodes before exiting
    while(dHead != NULL){
        Driver *temp = dHead;
        dHead = dHead->next;
        free(temp);
    }
    while(pHead != NULL){
        Passenger *temp = pHead;
        pHead = pHead->next;
        free(temp);
    }
    while(bHead != NULL){
        Booking *temp = bHead;
        bHead = bHead->next;
        free(temp);
    }

    return 0;
}
