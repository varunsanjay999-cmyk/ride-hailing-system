#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

typedef struct Location{
    int x;
    int y;
} Location;

typedef struct {
    int d_id;
    int p_id;
    int count;
} Pair;

typedef struct Node{
    int key;  // Can be used for Driver ID, Passenger ID, or Booking ID
    void *data; // Pointer to the actual data (Driver, Passenger, or Booking)
    int height;
    struct Node *left, *right;
} Node;

typedef struct Driver{
    int d_ID;
    char name[50];
    int vehicle_type;
    Location current_location;
    int status;
    float total_earnings;
} Driver;

typedef struct Passenger{
    int p_ID;
    char name[50];
    char mobile_number[15];
    int frequency;
} Passenger;

typedef struct Booking{
    int booking_id;
    int d_ID;
    int p_ID;
    int vehicle_type;
    int pickup_x;
    int pickup_y;
    float distance_travelled;
    float fare;
    int date;
    int completed;   // Added to prevent completing the same ride more than once
} Booking;

Node* searchNode(Node* root, int key) {
    while(root != NULL) {
        if(key < root->key)
            root = root->left;
        else if(key > root->key)
            root = root->right;
        else
            return root; // Found
    }
    return NULL; // Not found
}

int getHeight(Node* node) {
    if(node == NULL)
        return 0;
    return node->height;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

Node* rightRotate(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    // update y
    int leftHeight = getHeight(y->left);
    int rightHeight = getHeight(y->right);
    y->height = 1 + max(leftHeight, rightHeight);

    // update x
    leftHeight = getHeight(x->left);
    rightHeight = getHeight(x->right);
    x->height = 1 + max(leftHeight, rightHeight);
    return x;
}

Node* leftRotate(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    // update x
    int leftHeight = getHeight(x->left);
    int rightHeight = getHeight(x->right);
    x->height = 1 + max(leftHeight, rightHeight);

    // update y
    leftHeight = getHeight(y->left);
    rightHeight = getHeight(y->right);
    y->height = 1 + max(leftHeight, rightHeight);
    return y;
}

Node* insertNode(Node* node, int key, void* data)
{
    if (node == NULL) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        if(newNode == NULL){
            printf("Memory allocation failed\n");
            return NULL;
        }

        newNode->key = key;
        newNode->data = data;
        newNode->height = 1;
        newNode->left = newNode->right = NULL;
        return newNode;
    }

    if (key < node->key)
        node->left = insertNode(node->left, key, data);
    else if (key > node->key)
        node->right = insertNode(node->right, key, data);
    else
        return node; // duplicate

    int leftHeight = getHeight(node->left);
    int rightHeight = getHeight(node->right);

    if(leftHeight > rightHeight)
        node->height = 1 + leftHeight;
    else
        node->height = 1 + rightHeight;

    int balance = leftHeight - rightHeight;

    // LL
    if (balance > 1 && key < node->left->key)
        return rightRotate(node);

    // RR
    if (balance < -1 && key > node->right->key)
        return leftRotate(node);

    // LR
    if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    // RL
    if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

Node* addDriver(Node* root, int id, char name[], int type, int x, int y)
{
    if(searchNode(root, id) != NULL){
        printf("Driver ID already exists\n");
        return root;
    }

    if(type != 0 && type != 1){
        printf("Invalid vehicle type\n");
        return root;
    }

    Driver* d = (Driver*)malloc(sizeof(Driver));
    if(d == NULL){
        printf("Memory allocation failed\n");
        return root;
    }

    d->d_ID = id;
    snprintf(d->name, sizeof(d->name), "%s", name);
    d->vehicle_type = type;
    d->current_location.x = x;
    d->current_location.y = y;
    d->status = 0;
    d->total_earnings = 0;

    Node* newRoot = insertNode(root, id, d);
    if(searchNode(newRoot, id) == NULL){
        free(d);
        return root;
    }
    return newRoot;
}

Node* addPassenger(Node* root, int id, char name[], char mobile[])
{
    if(searchNode(root, id) != NULL){
        printf("Passenger ID already exists\n");
        return root;
    }

    Passenger* p = (Passenger*)malloc(sizeof(Passenger));
    if(p == NULL){
        printf("Memory allocation failed\n");
        return root;
    }

    p->p_ID = id;
    snprintf(p->name, sizeof(p->name), "%s", name);
    snprintf(p->mobile_number, sizeof(p->mobile_number), "%s", mobile);
    p->frequency = 0;

    Node* newRoot = insertNode(root, id, p);
    if(searchNode(newRoot, id) == NULL){
        free(p);
        return root;
    }
    return newRoot;
}

void findNearestDriver(Node* root, int px, int py, int prefType,
                       Driver** best, float* minDist)
{
    if(root == NULL)
        return;

    Driver* d = (Driver*)root->data;

    if(d->status == 0 &&
       (prefType == -1 || d->vehicle_type == prefType))
    {
        float dx = d->current_location.x - px;
        float dy = d->current_location.y - py;
        float dist = sqrt(dx*dx + dy*dy);

        if(dist <= 5 && dist < *minDist)
        {
            *minDist = dist;
            *best = d;
        }
    }

    findNearestDriver(root->left, px, py, prefType, best, minDist);
    findNearestDriver(root->right, px, py, prefType, best, minDist);
}

int generateBookingID()
{
    static int id = 1;
    return id++;
}

Node* requestRide(Node* driverRoot, Node* passengerRoot, Node* bookingRoot,
                  int p_id, int px, int py, int prefType)
{
    Node* pnode = searchNode(passengerRoot, p_id);

    if(pnode == NULL){
        printf("Passenger not found\n");
        return bookingRoot;
    }

    if(prefType != 0 && prefType != 1 && prefType != -1){
        printf("Invalid vehicle type\n");
        return bookingRoot;
    }

    Driver* bestDriver = NULL;
    float minDist = 99999;

    findNearestDriver(driverRoot, px, py, prefType, &bestDriver, &minDist);

    if(bestDriver == NULL){
        printf("No drivers available nearby\n");
        return bookingRoot;
    }

    Booking* b = (Booking*)malloc(sizeof(Booking));
    if(b == NULL){
        printf("Memory allocation failed\n");
        return bookingRoot;
    }

    int booking_id = generateBookingID();

    b->booking_id = booking_id;
    b->d_ID = bestDriver->d_ID;
    b->p_ID = p_id;
    b->vehicle_type = bestDriver->vehicle_type;
    b->pickup_x = px;
    b->pickup_y = py;
    b->distance_travelled = 0;
    b->fare = 0;
    b->date = 0;       // Set to 0 until a date is provided
    b->completed = 0;

    Node* newBookingRoot = insertNode(bookingRoot, booking_id, b);
    if(searchNode(newBookingRoot, booking_id) == NULL){
        free(b);
        return bookingRoot;
    }

    bestDriver->status = 1;

    printf("\nRide Booked Successfully!\n");
    printf("Booking ID: %d\n", booking_id);
    printf("Driver: %s\n", bestDriver->name);
    printf("Vehicle Type: %s\n",
           bestDriver->vehicle_type == 0 ? "Cab" : "Bike");
    printf("Pickup Location: (%d, %d)\n", px, py);
    printf("Distance to driver: %.2f km\n", minDist);

    return newBookingRoot;
}

Node* completeRide(Node* bookingRoot, Node* driverRoot, Node* passengerRoot,
                   int booking_id, int new_x, int new_y)
{
    Node* bnode = searchNode(bookingRoot, booking_id);

    if(bnode == NULL){
        printf("Booking not found\n");
        return bookingRoot;
    }

    Booking* b = (Booking*)bnode->data;

    if(b->completed){
        printf("This ride has already been completed\n");
        return bookingRoot;
    }

    float dx = new_x - b->pickup_x;
    float dy = new_y - b->pickup_y;
    float distance = sqrt(dx*dx + dy*dy);

    b->distance_travelled = distance;

    if(b->vehicle_type == 0)
        b->fare = distance * 10;
    else
        b->fare = distance * 5;

    Node* dnode = searchNode(driverRoot, b->d_ID);

    if(dnode != NULL){
        Driver* d = (Driver*)dnode->data;
        d->total_earnings += b->fare;
        d->status = 0;
        d->current_location.x = new_x;
        d->current_location.y = new_y;
    }

    Node* pnode = searchNode(passengerRoot, b->p_ID);

    if(pnode != NULL){
        Passenger* p = (Passenger*)pnode->data;
        p->frequency++;
    }

    b->completed = 1;

    printf("Ride Completed Successfully\n");
    printf("Booking ID: %d\n", b->booking_id);
    printf("Distance: %.2f\n", b->distance_travelled);
    printf("Fare: %.2f\n", b->fare);

    return bookingRoot;
}

float getDriverEarnings(Node* driverRoot, int d_id){
    Node* dNode = searchNode(driverRoot, d_id);
    if(dNode == NULL){
        printf("Driver not found\n");
        return 0.0;
    }
    Driver* d = (Driver*)dNode->data;
    return d->total_earnings;
}

void displayTopDriversUtil(Node* root,Driver** top1, Driver** top2, Driver** top3)
{
    if(root == NULL)
        return;

    Driver* d = (Driver*)root->data;

    if(*top1 == NULL || d->total_earnings > (*top1)->total_earnings){
        *top3 = *top2;
        *top2 = *top1;
        *top1 = d;
    }
    else if(*top2 == NULL || d->total_earnings > (*top2)->total_earnings){
        *top3 = *top2;
        *top2 = d;
    }
    else if(*top3 == NULL || d->total_earnings > (*top3)->total_earnings){
        *top3 = d;
    }

    displayTopDriversUtil(root->left, top1, top2, top3);
    displayTopDriversUtil(root->right, top1, top2, top3);
}

void displayTopDrivers(Node* driverRoot)
{
    if(driverRoot == NULL){
        printf("No drivers available\n");
        return;
    }

    Driver *top1 = NULL, *top2 = NULL, *top3 = NULL;

    displayTopDriversUtil(driverRoot, &top1, &top2, &top3);

    printf("Top 3 Drivers:\n");

    if(top1)
        printf("1. ID:%d Name:%s Earnings:%.2f\n",
               top1->d_ID, top1->name, top1->total_earnings);

    if(top2)
        printf("2. ID:%d Name:%s Earnings:%.2f\n",
               top2->d_ID, top2->name, top2->total_earnings);

    if(top3)
        printf("3. ID:%d Name:%s Earnings:%.2f\n",
               top3->d_ID, top3->name, top3->total_earnings);
}

void collectPairs(Node* root, Pair pairs[], int *size)
{
    if(root == NULL)
        return;

    Booking* b = (Booking*)root->data;
    int found = 0;

    for(int i = 0; i < *size; i++){
        if(pairs[i].d_id == b->d_ID && pairs[i].p_id == b->p_ID){
            pairs[i].count++;
            found = 1;
            break;
        }
    }

    if(!found){
        if(*size < 1000){
            pairs[*size].d_id = b->d_ID;
            pairs[*size].p_id = b->p_ID;
            pairs[*size].count = 1;
            (*size)++;
        }
    }

    collectPairs(root->left, pairs, size);
    collectPairs(root->right, pairs, size);
}

void displayFrequentPairs(Node* bookingRoot, Node* driverRoot, Node* passengerRoot)
{
    if(bookingRoot == NULL){
        printf("No bookings available\n");
        return;
    }

    Pair pairs[1000];
    int size = 0;

    collectPairs(bookingRoot, pairs, &size);

    if(size == 0){
        printf("No booking pairs available\n");
        return;
    }

    int maxCount = 0;
    for(int i = 0; i < size; i++){
        if(pairs[i].count > maxCount)
            maxCount = pairs[i].count;
    }

    printf("\nMost Frequent Driver-Passenger Pair(s):\n");

    for(int i = 0; i < size; i++){
        if(pairs[i].count == maxCount){
            Node* dnode = searchNode(driverRoot, pairs[i].d_id);
            Node* pnode = searchNode(passengerRoot, pairs[i].p_id);

            if(dnode != NULL && pnode != NULL){
                Driver* d = (Driver*)dnode->data;
                Passenger* p = (Passenger*)pnode->data;

                printf("Driver: %s (ID:%d), Passenger: %s (ID:%d), Rides: %d\n",
                       d->name, d->d_ID,
                       p->name, p->p_ID,
                       pairs[i].count);
            }
        }
    }
}

void displayAvailableVehicles(Node* root)
{
    if(root == NULL)
        return;

    displayAvailableVehicles(root->left);

    Driver* d = (Driver*)root->data;

    if(d->status == 0)
    {
        printf("Vehicle Type: %s\n",
               (d->vehicle_type == 0) ? "Cab" : "Bike");
        printf("Driver: %s (ID:%d)\n", d->name, d->d_ID);
        printf("Location: (%d, %d)\n",
               d->current_location.x,
               d->current_location.y);
        printf("-------------------------\n");
    }

    displayAvailableVehicles(root->right);
}

void updateDriverLocation(Node* root, int d_ID, int new_x, int new_y){
    if(root == NULL){
        printf("No drivers available\n");
        return;
    }

    Node* dNode = searchNode(root, d_ID);
    if(dNode == NULL){
        printf("Driver not found\n");
        return;
    }

    Driver* d = (Driver*)dNode->data;
    if(d->status == 1){
        printf("Cannot update location. Driver is currently on a ride.\n");
        return;
    }

    d->current_location.x = new_x;
    d->current_location.y = new_y;
    printf("Driver location updated successfully\n");
}

int getBalance(Node* node) {
    if(node == NULL)
        return 0;
    return getHeight(node->left) - getHeight(node->right);
}

Node *minValueNode(Node* root) {
    Node* Current = root;
    while(Current->left != NULL)
        Current = Current->left;
    return Current;
}

Node* deleteDriver(Node* root, int d_ID)
{
    if(root == NULL)
        return root;

    if(d_ID < root->key)
        root->left = deleteDriver(root->left, d_ID);
    else if(d_ID > root->key)
        root->right = deleteDriver(root->right, d_ID);
    else
    {
        Driver* d = (Driver*)root->data;
        if(d->status == 1){
            printf("Cannot delete a driver who is currently on a ride\n");
            return root;
        }

        if(root->left == NULL || root->right == NULL)
        {
            Node* temp = root->left ? root->left : root->right;

            if(temp == NULL)
            {
                free(root->data);
                free(root);
                root = NULL;
            }
            else
            {
                Node* oldRoot = root;
                *root = *temp;
                free(oldRoot->data);
                free(oldRoot);
            }
        }
        else
        {
            Node* temp = minValueNode(root->right);

            // Swap the driver data so the recursive deletion removes the
            // original driver node without duplicating the successor data.
            int oldKey = root->key;
            void* oldData = root->data;

            root->key = temp->key;
            root->data = temp->data;
            temp->key = oldKey;
            temp->data = oldData;

            root->right = deleteDriver(root->right, oldKey);
        }
    }

    if(root == NULL)
        return root;

    int leftHeight = getHeight(root->left);
    int rightHeight = getHeight(root->right);
    root->height = 1 + max(leftHeight, rightHeight);

    int balance = getBalance(root);

    if(balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);

    if(balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);

    if(balance > 1 && getBalance(root->left) < 0){
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    if(balance < -1 && getBalance(root->right) > 0){
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

void rangeSearchPassengers(Node* root, int low, int high)
{
    if(root == NULL)
        return;

    if(root->key > low)
        rangeSearchPassengers(root->left, low, high);

    if(root->key >= low && root->key <= high)
    {
        Passenger* p = (Passenger*)root->data;
        printf("Passenger ID: %d\n", p->p_ID);
        printf("Name: %s\n", p->name);
        printf("Mobile: %s\n", p->mobile_number);
        printf("Frequency: %d\n", p->frequency);
        printf("-------------------------\n");
    }

    if(root->key < high)
        rangeSearchPassengers(root->right, low, high);
}

void displayBookingHistory(Node* root)
{
    if(root == NULL){
        printf("No bookings available\n");
        return;
    }

    displayBookingHistory(root->left);

    Booking* b = (Booking*)root->data;
    printf("Booking ID: %d\n", b->booking_id);
    printf("Driver ID: %d\n", b->d_ID);
    printf("Passenger ID: %d\n", b->p_ID);
    printf("Vehicle Type: %s\n",
           (b->vehicle_type == 0) ? "Cab" : "Bike");
    printf("Distance: %.2f km\n", b->distance_travelled);
    printf("Fare: %.2f\n", b->fare);
    printf("Date: %d\n", b->date);
    printf("Status: %s\n", b->completed ? "Completed" : "Booked");
    printf("-----------------------------\n");

    displayBookingHistory(root->right);
}

int main()
{
    Node *driverRoot = NULL;
    Node *passengerRoot = NULL;
    Node *bookingRoot = NULL;

    int choice;

    while(1)
    {
        printf("\n===== RIDE HAILING SYSTEM =====\n");
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
        printf("11. Range Search (Passengers)\n");
        printf("12. Exit\n");
        printf("Enter your choice: ");

        if(scanf("%d", &choice) != 1){
            printf("Invalid input\n");
            return 1;
        }

        switch(choice)
        {
            case 1:
            {
                int id, type, x, y;
                char name[50];

                printf("Enter Driver ID: ");
                if(scanf("%d", &id) != 1) return 1;
                printf("Enter Name: ");
                if(scanf("%49s", name) != 1) return 1;
                printf("Enter Vehicle Type (0=Cab,1=Bike): ");
                if(scanf("%d", &type) != 1) return 1;
                printf("Enter Location (x y): ");
                if(scanf("%d %d", &x, &y) != 2) return 1;

                driverRoot = addDriver(driverRoot, id, name, type, x, y);
                break;
            }

            case 2:
            {
                int id;
                char name[50], mobile[15];

                printf("Enter Passenger ID: ");
                if(scanf("%d", &id) != 1) return 1;
                printf("Enter Name: ");
                if(scanf("%49s", name) != 1) return 1;
                printf("Enter Mobile: ");
                if(scanf("%14s", mobile) != 1) return 1;

                passengerRoot = addPassenger(passengerRoot, id, name, mobile);
                break;
            }

            case 3:
            {
                int p_id, x, y, type;

                printf("Enter Passenger ID: ");
                if(scanf("%d", &p_id) != 1) return 1;
                printf("Enter Pickup Location (x y): ");
                if(scanf("%d %d", &x, &y) != 2) return 1;
                printf("Enter Vehicle Type (0=Cab,1=Bike): ");
                if(scanf("%d", &type) != 1) return 1;

                bookingRoot = requestRide(driverRoot, passengerRoot,
                                          bookingRoot, p_id, x, y, type);
                break;
            }

            case 4:
            {
                int booking_id, new_x, new_y;

                printf("Enter Booking ID: ");
                if(scanf("%d", &booking_id) != 1) return 1;
                printf("Enter Drop Location (x y): ");
                if(scanf("%d %d", &new_x, &new_y) != 2) return 1;

                bookingRoot = completeRide(bookingRoot, driverRoot,
                                           passengerRoot, booking_id,
                                           new_x, new_y);
                break;
            }

            case 5:
                displayTopDrivers(driverRoot);
                break;

            case 6:
                displayFrequentPairs(bookingRoot, driverRoot, passengerRoot);
                break;

            case 7:
                displayAvailableVehicles(driverRoot);
                break;

            case 8:
            {
                int id, x, y;
                printf("Enter Driver ID: ");
                if(scanf("%d", &id) != 1) return 1;
                printf("Enter New Location (x y): ");
                if(scanf("%d %d", &x, &y) != 2) return 1;

                updateDriverLocation(driverRoot, id, x, y);
                break;
            }

            case 9:
            {
                int id;
                printf("Enter Driver ID to delete: ");
                if(scanf("%d", &id) != 1) return 1;

                driverRoot = deleteDriver(driverRoot, id);
                break;
            }

            case 10:
                displayBookingHistory(bookingRoot);
                break;

            case 11:
            {
                int low, high;
                printf("Enter P_ID1 and P_ID2: ");
                if(scanf("%d %d", &low, &high) != 2) return 1;

                if(low > high){
                    int temp = low;
                    low = high;
                    high = temp;
                }

                rangeSearchPassengers(passengerRoot, low, high);
                break;
            }

            case 12:
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid choice\n");
        }
    }
}
